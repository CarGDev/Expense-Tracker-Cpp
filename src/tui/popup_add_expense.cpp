#include "popup_add_expense.h"

#include <algorithm>
#include <chrono>
#include <string>

#include "../categories/categories.h"
#include "../memory/core.h"

namespace {
const std::vector<std::string> kEmptySubs;
}  // namespace

PopupAddExpense::PopupAddExpense()
    : open_(false)
    , rect_{}
    , categoryMenu_(categories)
    , subcategoryMenu_(kEmptySubs)
    , totalInput_(InputKind::Amount)
    , dateInput_(InputKind::Date)
    , cur_(WidgetId::PopupCategory)
    , mode_(Mode::Navigation)
    , didSubmit_(false)
    , order_({WidgetId::PopupCategory, WidgetId::PopupSubcategory,
              WidgetId::PopupTotal, WidgetId::PopupDate,
              WidgetId::PopupSubmit}) {
  // Initialize subcategory from first category if available.
  if (!categories.empty()) {
    const auto& subs = getSubcategories(categories[0]);
    subcategoryMenu_.setItems(subs);
  }
  totalInput_.setPlaceholder("x.xx");
  dateInput_.setPlaceholder("YYYY-MM-DD");
}

PopupAddExpense::~PopupAddExpense() {
  close();
}

bool PopupAddExpense::isOpen() const {
  return open_;
}

Rect PopupAddExpense::popupRect() const {
  return rect_;
}

WidgetId PopupAddExpense::popupFocus() const {
  return cur_;
}

Mode PopupAddExpense::popupMode() const {
  return mode_;
}

Menu& PopupAddExpense::categoryMenu() {
  return categoryMenu_;
}

Menu& PopupAddExpense::subcategoryMenu() {
  return subcategoryMenu_;
}

Input& PopupAddExpense::totalInput() {
  return totalInput_;
}

Input& PopupAddExpense::dateInput() {
  return dateInput_;
}

bool PopupAddExpense::didSubmit() const {
  return didSubmit_;
}

const std::string& PopupAddExpense::hint() const {
  return hint_;
}

void PopupAddExpense::clearHint() {
  hint_.clear();
}

void PopupAddExpense::setMemory(ExpenseMemory& mem) {
  memory_ = &mem;
}

void PopupAddExpense::syncSubcategory() {
  const std::string& cat = categoryMenu_.selected();
  const auto& subs = getSubcategories(cat);
  subcategoryMenu_.setItems(subs);
}

int PopupAddExpense::indexOf(WidgetId id) const {
  for (size_t i = 0; i < order_.size(); ++i) {
    if (order_[i] == id) return static_cast<int>(i);
  }
  return 0;
}

WidgetId PopupAddExpense::moveNext() {
  int idx = indexOf(cur_);
  idx = (idx + 1) % static_cast<int>(order_.size());
  cur_ = order_[idx];
  return cur_;
}

WidgetId PopupAddExpense::movePrev() {
  int idx = indexOf(cur_);
  idx = (idx - 1 + static_cast<int>(order_.size())) % static_cast<int>(order_.size());
  cur_ = order_[idx];
  return cur_;
}

WidgetId PopupAddExpense::translate(int ch) {
  if (mode_ == Mode::Edit) {
    if (ch == 27) {
      mode_ = Mode::Navigation;
      return cur_;
    }
    if (ch == '\t') {
      mode_ = Mode::Navigation;
      return moveNext();
    }
    if (ch == KEY_BTAB) {
      mode_ = Mode::Navigation;
      return movePrev();
    }
    // hjkl/arrows inert in Edit (except handled via Input handleKey)
    return cur_;
  }
  if (ch == '\t') return moveNext();
  if (ch == KEY_BTAB) return movePrev();
  return cur_;
}

void PopupAddExpense::open(int termRows, int termCols) {
  close();
  const int h = 14;
  const int w = 58;
  int y = (termRows - h) / 2;
  int x = (termCols - w) / 2;
  if (y < 0) y = 0;
  if (x < 0) x = 0;
  rect_ = {y, x, h, w};
  WINDOW* raw = newwin(h, w, y, x);
  if (!raw) {
    return;
  }
  keypad(raw, TRUE);
  box(raw, 0, 0);
  win_.reset(raw);
  open_ = true;
  cur_ = WidgetId::PopupCategory;
  mode_ = Mode::Navigation;
  didSubmit_ = false;
  hint_.clear();
  // Ensure subcategory synced to current category
  syncSubcategory();
  wrefresh(win_.get());
}

void PopupAddExpense::close() {
  if (win_) {
    win_.reset();
  }
  open_ = false;
  rect_ = Rect{};
  mode_ = Mode::Navigation;
}

bool PopupAddExpense::handleKey(int ch) {
  if (!open_) return false;

  // Edit mode handling for Total/Date inputs
  if (mode_ == Mode::Edit && (cur_ == WidgetId::PopupTotal || cur_ == WidgetId::PopupDate)) {
    if (ch == 27) {
      mode_ = Mode::Navigation;
      return true;
    }
    Input* inp = (cur_ == WidgetId::PopupTotal) ? &totalInput_ : &dateInput_;
    if (inp->handleKey(ch, true)) {
      return true;
    }
    if (ch == '\t' || ch == KEY_BTAB) {
      WidgetId prev = cur_;
      WidgetId nxt = translate(ch);
      if (nxt != prev) {
        return true;
      }
    }
    return false;
  }

  if (ch == 27 || ch == 'q' || ch == 'Q') {
    didSubmit_ = false;
    hint_.clear();
    close();
    return true;
  }

  if (cur_ == WidgetId::PopupCategory) {
    if (ch == 'j' || ch == 'k' || ch == KEY_DOWN || ch == KEY_UP) {
      bool handled = categoryMenu_.handleKey(ch);
      if (handled) {
        syncSubcategory();
        return true;
      }
    }
  } else if (cur_ == WidgetId::PopupSubcategory) {
    if (ch == 'j' || ch == 'k' || ch == KEY_DOWN || ch == KEY_UP) {
      subcategoryMenu_.handleKey(ch);
      return true;
    }
  } else if (cur_ == WidgetId::PopupTotal || cur_ == WidgetId::PopupDate) {
    if (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n') {
      mode_ = Mode::Edit;
      return true;
    }
    if (ch >= 32 && ch <= 126) {
      mode_ = Mode::Edit;
      Input* inp = (cur_ == WidgetId::PopupTotal) ? &totalInput_ : &dateInput_;
      inp->handleKey(ch, true);
      return true;
    }
  }

  if (cur_ == WidgetId::PopupSubmit && (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n' || ch == ' ')) {
    hint_.clear();
    std::string amtStr = totalInput_.value();
    if (amtStr.empty()) {
      hint_ = "Amount required";
      return true;
    }
    double amt = 0;
    try {
      size_t pos = 0;
      amt = std::stod(amtStr, &pos);
      if (pos != amtStr.size()) throw std::invalid_argument("trailing");
    } catch (...) {
      hint_ = "Invalid amount";
      return true;
    }
    std::string dateStr = dateInput_.value();
    std::chrono::system_clock::time_point dt = std::chrono::system_clock::now();
    if (!dateStr.empty()) {
      auto parsed = parseYYYYMMDD(dateStr);
      if (!parsed) {
        hint_ = "Invalid date YYYY-MM-DD";
        return true;
      }
      dt = *parsed;
    }
    if (!memory_) {
      hint_ = "No memory";
      return true;
    }
    ExpenseRecord rec;
    rec.id = 0;
    rec.amount = amt;
    rec.category = categoryMenu_.selected();
    rec.sub_category = subcategoryMenu_.selected();
    rec.datetime = dt;
    memory_->addExpense(rec);
    didSubmit_ = true;
    totalInput_.clear();
    dateInput_.clear();
    hint_.clear();
    close();
    return true;
  }

  WidgetId prev = cur_;
  WidgetId nxt = translate(ch);
  if (nxt != prev) {
    hint_.clear();
    return true;
  }

  return false;
}

void PopupAddExpense::draw() {
  if (!open_ || !win_) return;
  WINDOW* win = win_.get();
  werase(win);
  box(win, 0, 0);
  int h = rect_.h;
  int w = rect_.w;
  // Title centered on border
  std::string title = "Add Expense";
  int tx = (w - static_cast<int>(title.size())) / 2;
  if (tx < 1) tx = 1;
  mvwprintw(win, 0, tx, "%s", title.c_str());

  // Layout: labels at x=2, fields at x=18 with width = w-20
  int fieldX = 18;
  int fieldW = w - fieldX - 3;
  if (fieldW < 10) fieldW = 10;

  // Row positions inside window (1-indexed for border)
  int rowCat = 2;
  int rowSub = 5;
  int rowTot = 8;
  int rowDat = 9;
  int rowSubm = 11;

  // Labels
  mvwprintw(win, rowCat, 2, "Category");
  mvwprintw(win, rowSub, 2, "Subcategory");
  mvwprintw(win, rowTot, 2, "Total");
  mvwprintw(win, rowDat, 2, "Date");

  // Category menu: draw with x-offset rendering
  {
    bool focused = (cur_ == WidgetId::PopupCategory);
    // Show at rowCat, but allow menu to occupy 2 rows height so scrolling visible
    Rect rc{rowCat, fieldX, 2, fieldW};
    // x-offset for indented look: use 1
    categoryMenu_.draw(win, rc, focused, 0);
  }
  {
    bool focused = (cur_ == WidgetId::PopupSubcategory);
    Rect rc{rowSub, fieldX, 2, fieldW};
    subcategoryMenu_.draw(win, rc, focused, 0);
  }
  {
    bool focused = (cur_ == WidgetId::PopupTotal);
    // Also respect edit mode highlight
    if (mode_ == Mode::Edit && focused) {
      // Input draw will use A_REVERSE when focused; keep as is
    }
    Rect rc{rowTot, fieldX, 1, fieldW};
    totalInput_.draw(win, rc, focused);
  }
  {
    bool focused = (cur_ == WidgetId::PopupDate);
    Rect rc{rowDat, fieldX, 1, fieldW};
    dateInput_.draw(win, rc, focused);
  }
  // Submit button centered
  {
    std::string label = "[ Submit ]";
    int sx = (w - static_cast<int>(label.size())) / 2;
    bool focused = (cur_ == WidgetId::PopupSubmit);
    if (focused) wattron(win, A_REVERSE);
    mvwprintw(win, rowSubm, sx, "%s", label.c_str());
    if (focused) wattroff(win, A_REVERSE);
  }

  // Hint / error line
  std::string hintLine = hint_.empty() ? "Tab navigate  Enter select  Esc/q close" : hint_;
  int hx = (w - static_cast<int>(hintLine.size())) / 2;
  if (hx < 1) hx = 1;
  if (!hint_.empty()) wattron(win, A_BOLD);
  mvwprintw(win, h - 2, hx, "%s", hintLine.c_str());
  if (!hint_.empty()) wattroff(win, A_BOLD);

  wrefresh(win);
}
