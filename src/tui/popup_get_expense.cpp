#include "popup_get_expense.h"

#include <chrono>
#include <string>

#include "../memory/core.h"

PopupGetExpense::PopupGetExpense()
    : open_(false)
    , rect_{}
    , fromInput_(InputKind::Date)
    , toInput_(InputKind::Date)
    , cur_(0)
    , mode_(Mode::Navigation)
    , didSubmit_(false) {
  fromInput_.setPlaceholder("YYYY-MM-DD");
  toInput_.setPlaceholder("YYYY-MM-DD");
}

PopupGetExpense::~PopupGetExpense() {
  close();
}

bool PopupGetExpense::isOpen() const {
  return open_;
}

Rect PopupGetExpense::popupRect() const {
  return rect_;
}

Mode PopupGetExpense::popupMode() const {
  return mode_;
}

Input& PopupGetExpense::fromInput() {
  return fromInput_;
}

Input& PopupGetExpense::toInput() {
  return toInput_;
}

bool PopupGetExpense::didSubmit() const {
  return didSubmit_;
}

const std::string& PopupGetExpense::hint() const {
  return hint_;
}

void PopupGetExpense::clearHint() {
  hint_.clear();
}

void PopupGetExpense::setMemory(ExpenseMemory& mem) {
  memory_ = &mem;
}

const std::vector<ExpenseRecord>& PopupGetExpense::filtered() const {
  return filtered_;
}

int PopupGetExpense::moveNext() {
  cur_ = (cur_ + 1) % 4;
  return cur_;
}

int PopupGetExpense::movePrev() {
  cur_ = (cur_ - 1 + 4) % 4;
  return cur_;
}

int PopupGetExpense::translate(int ch) {
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
    return cur_;
  }
  if (ch == '\t') return moveNext();
  if (ch == KEY_BTAB) return movePrev();
  return cur_;
}

void PopupGetExpense::open(int termRows, int termCols) {
  close();
  const int h = 14;
  const int w = 58;
  int y = (termRows - h) / 2;
  int x = (termCols - w) / 2;
  if (y < 0) y = 0;
  if (x < 0) x = 0;
  rect_ = {y, x, h, w};
  WINDOW* raw = newwin(h, w, y, x);
  if (!raw) return;
  keypad(raw, TRUE);
  box(raw, 0, 0);
  win_.reset(raw);
  open_ = true;
  cur_ = 0;
  mode_ = Mode::Navigation;
  didSubmit_ = false;
  hint_.clear();
  filtered_.clear();
  wrefresh(win_.get());
}

void PopupGetExpense::close() {
  if (win_) win_.reset();
  open_ = false;
  rect_ = Rect{};
  mode_ = Mode::Navigation;
}

bool PopupGetExpense::handleKey(int ch) {
  if (!open_) return false;

  if (mode_ == Mode::Edit && (cur_ == 0 || cur_ == 1)) {
    if (ch == 27) {
      mode_ = Mode::Navigation;
      return true;
    }
    Input* inp = (cur_ == 0) ? &fromInput_ : &toInput_;
    if (inp->handleKey(ch, true)) return true;
    if (ch == '\t' || ch == KEY_BTAB) {
      int prev = cur_;
      int nxt = translate(ch);
      if (nxt != prev) return true;
    }
    return false;
  }

  if (ch == 27) {
    didSubmit_ = false;
    hint_.clear();
    close();
    return true;
  }
  if (mode_ == Mode::Navigation && (ch == 'q' || ch == 'Q')) {
    didSubmit_ = false;
    hint_.clear();
    close();
    return true;
  }

  if (cur_ == 0 || cur_ == 1) {
    if (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n') {
      mode_ = Mode::Edit;
      return true;
    }
    if (ch >= 32 && ch <= 126) {
      mode_ = Mode::Edit;
      Input* inp = (cur_ == 0) ? &fromInput_ : &toInput_;
      inp->handleKey(ch, true);
      return true;
    }
  }

  if (cur_ == 2 && (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n' || ch == ' ')) {
    hint_.clear();
    std::string f = fromInput_.value();
    std::string t = toInput_.value();
    if (f.empty() || t.empty()) {
      hint_ = "Both dates required YYYY-MM-DD";
      return true;
    }
    auto pf = parseYYYYMMDD(f);
    auto pt = parseYYYYMMDD(t);
    if (!pf) {
      hint_ = "Invalid date_from YYYY-MM-DD";
      return true;
    }
    if (!pt) {
      hint_ = "Invalid date_to YYYY-MM-DD";
      return true;
    }
    if (!memory_) {
      hint_ = "No memory";
      return true;
    }
    if (*pf > *pt) {
      hint_ = "date_from must be <= date_to";
      return true;
    }
    filtered_ = memory_->getExpensesByDateTime(*pf, *pt);
    didSubmit_ = true;
    hint_.clear();
    close();
    return true;
  }

  if (cur_ == 3 && (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n' || ch == ' ')) {
    didSubmit_ = false;
    hint_.clear();
    close();
    return true;
  }

  int prev = cur_;
  int nxt = translate(ch);
  if (nxt != prev) {
    hint_.clear();
    return true;
  }

  return false;
}

void PopupGetExpense::draw() {
  if (!open_ || !win_) return;
  WINDOW* win = win_.get();
  werase(win);
  box(win, 0, 0);
  int h = rect_.h;
  int w = rect_.w;
  std::string title = "Get Expenses";
  int tx = (w - static_cast<int>(title.size())) / 2;
  if (tx < 1) tx = 1;
  mvwprintw(win, 0, tx, "%s", title.c_str());

  int fieldX = 18;
  int fieldW = w - fieldX - 3;
  if (fieldW < 10) fieldW = 10;

  int rowFrom = 3;
  int rowTo = 5;
  int rowBtn = 8;

  mvwprintw(win, rowFrom, 2, "Date from");
  mvwprintw(win, rowTo, 2, "Date to");

  {
    bool focused = (cur_ == 0);
    Rect rc{rowFrom, fieldX, 1, fieldW};
    fromInput_.draw(win, rc, focused);
  }
  {
    bool focused = (cur_ == 1);
    Rect rc{rowTo, fieldX, 1, fieldW};
    toInput_.draw(win, rc, focused);
  }
  {
    std::string label = "[ Submit ]";
    int sx = (w / 2) - static_cast<int>(label.size()) - 2;
    if (sx < 2) sx = 2;
    bool focused = (cur_ == 2);
    if (focused) wattron(win, A_REVERSE);
    mvwprintw(win, rowBtn, sx, "%s", label.c_str());
    if (focused) wattroff(win, A_REVERSE);
  }
  {
    std::string label = "[ Cancel ]";
    int sx = (w / 2) + 2;
    bool focused = (cur_ == 3);
    if (focused) wattron(win, A_REVERSE);
    mvwprintw(win, rowBtn, sx, "%s", label.c_str());
    if (focused) wattroff(win, A_REVERSE);
  }

  std::string hintLine = hint_.empty() ? "Tab navigate  Enter select  Esc cancel" : hint_;
  int hx = (w - static_cast<int>(hintLine.size())) / 2;
  if (hx < 1) hx = 1;
  if (!hint_.empty()) wattron(win, A_BOLD);
  mvwprintw(win, h - 2, hx, "%s", hintLine.c_str());
  if (!hint_.empty()) wattroff(win, A_BOLD);

  wrefresh(win);
}
