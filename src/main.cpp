#include <ncurses.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <algorithm>

#include "categories/categories.h"
#include "memory/core.h"
#include "tui/focus.h"
#include "tui/layout.h"
#include "tui/popup_add_expense.h"
#include "tui/popup_get_expense.h"
#include "tui/widgets/input.h"
#include "tui/widgets/menu.h"
#include "tui/widgets/table.h"

namespace {

void drawTitlePane(WINDOW* win) {
  if (!win) return;
  werase(win);
  box(win, 0, 0);
  int h, w;
  getmaxyx(win, h, w);
  std::string t1 = "===========Expense Tracker=========";
  std::string t2 = "Welcome to expense tracker";
  int x1 = (w - static_cast<int>(t1.size())) / 2;
  int x2 = (w - static_cast<int>(t2.size())) / 2;
  if (x1 < 1) x1 = 1;
  if (x2 < 1) x2 = 1;
  if (h > 1) mvwprintw(win, 1, x1, "%s", t1.c_str());
  if (h > 2) mvwprintw(win, 2, x2, "%s", t2.c_str());
  wrefresh(win);
}

void drawMenuPane(WINDOW* win, Menu& mainMenu, FocusManager& focus) {
  if (!win) return;
  werase(win);
  box(win, 0, 0);
  int h, w;
  getmaxyx(win, h, w);
  mvwprintw(win, 0, 2, " Menu ");
  // Sync highlight to focus
  WidgetId cur = focus.current();
  bool isMenuFocused = (cur == WidgetId::MenuAdd || cur == WidgetId::MenuGet || cur == WidgetId::MenuRemove);
  if (cur == WidgetId::MenuAdd) mainMenu.setSelectedIndex(0);
  else if (cur == WidgetId::MenuGet) mainMenu.setSelectedIndex(1);
  else if (cur == WidgetId::MenuRemove) mainMenu.setSelectedIndex(2);
  // Interior rect
  Rect rc{1, 2, h - 2, w - 4};
  mainMenu.draw(win, rc, isMenuFocused, 0);
  wrefresh(win);
}

void drawFilterPane(WINDOW* win, Input& filterFrom, Input& filterTo, Input& importInput,
                    Menu& categoryMenu, Menu& subcategoryMenu, FocusManager& focus) {
  if (!win) return;
  werase(win);
  box(win, 0, 0);
  int h, w;
  getmaxyx(win, h, w);
  mvwprintw(win, 0, 2, " Filter ");
  // Row 1: Filter from ___ to ___
  if (h > 2) {
    mvwprintw(win, 1, 2, "Filter");
    mvwprintw(win, 1, 10, "from");
    bool fFrom = (focus.current() == WidgetId::FilterFrom);
    Rect rcFrom{1, 15, 1, 10};
    if (rcFrom.x + rcFrom.w < w - 1) filterFrom.draw(win, rcFrom, fFrom);
    mvwprintw(win, 1, 26, "to");
    bool fTo = (focus.current() == WidgetId::FilterTo);
    Rect rcTo{1, 29, 1, 10};
    if (rcTo.x + rcTo.w < w - 1) filterTo.draw(win, rcTo, fTo);
  }
  // Row 2: import: input
  if (h > 3) {
    mvwprintw(win, 2, 2, "import:");
    bool fImp = (focus.current() == WidgetId::Import);
    int impX = 12;
    int impW = w - impX - 2;
    if (impW < 6) impW = 6;
    Rect rc{2, impX, 1, impW};
    importInput.draw(win, rc, fImp);
  }
  // Row 3: Category menu
  if (h > 4) {
    mvwprintw(win, 3, 2, "Category:");
    bool fCat = (focus.current() == WidgetId::Category);
    int mx = 13;
    int mw = w - mx - 2;
    if (mw < 8) mw = 8;
    Rect rc{3, mx, 1, mw};
    categoryMenu.draw(win, rc, fCat, 0);
  }
  // Row 4: Subcategory menu
  if (h > 5) {
    mvwprintw(win, 4, 2, "Subcategory:");
    bool fSub = (focus.current() == WidgetId::Subcategory);
    int mx = 14;
    int mw = w - mx - 2;
    if (mw < 8) mw = 8;
    Rect rc{4, mx, 1, mw};
    subcategoryMenu.draw(win, rc, fSub, 0);
  }
  wrefresh(win);
}

void drawExpensesPane(WINDOW* win, FocusManager& focus,
                       const ExpenseMemory& memory) {
  if (!win) return;
  werase(win);
  box(win, 0, 0);
  int h, w;
  getmaxyx(win, h, w);
  bool focused = (focus.current() == WidgetId::Expenses);
  if (focused) wattron(win, A_REVERSE);
  mvwprintw(win, 0, 2, " Expenses ");
  if (focused) wattroff(win, A_REVERSE);
  size_t count = memory.getExpenseCount();
  double total = memory.getTotalAmount();
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << total;
  std::string totalStr = oss.str();
  if (h > 1) mvwprintw(win, 1, 2, "Item: %zu", count);
  if (h > 2) mvwprintw(win, 2, 2, "Total: $ %s", totalStr.c_str());
  wrefresh(win);
}

void drawTablePane(WINDOW* win, Table& table, FocusManager& focus) {
  if (!win) return;
  werase(win);
  box(win, 0, 0);
  int h, w;
  getmaxyx(win, h, w);
  Rect inner{1, 1, h - 2, w - 2};
  bool focused = (focus.current() == WidgetId::TableRow);
  table.draw(win, inner, focused);
  wrefresh(win);
}

}  // namespace

int main() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  Layout layout;
  FocusManager focus;
  ExpenseMemory memory;

  std::vector<std::string> mainItems = {"add_expense", "get_expenses",
                                        "remove_expenses"};
  Menu mainMenu(mainItems);

  Input filterFrom(InputKind::Date);
  Input filterTo(InputKind::Date);
  Input importInput(InputKind::Text, 32, "input");

  Menu categoryMenu(categories);
  const std::string& initCat = categories.empty() ? std::string() : categories[0];
  const auto& initSubs = getSubcategories(initCat);
  Menu subcategoryMenu(initSubs);

  Table table;
  table.setStubRows(0);

  PopupAddExpense popup;
  PopupGetExpense popupGet;
  popup.setMemory(memory);
  popupGet.setMemory(memory);

  auto syncSubMain = [&]() {
    const std::string& cat = categoryMenu.selected();
    const auto& subs = getSubcategories(cat);
    subcategoryMenu.setItems(subs);
  };

  int rows, cols;
  getmaxyx(stdscr, rows, cols);
  if (!layout.needsPlaceholder(rows, cols)) {
    Regions regs = layout.compute(rows, cols);
    layout.createWindows(regs);
  }

  bool running = true;

  while (running) {
    getmaxyx(stdscr, rows, cols);
    bool placeholder = layout.needsPlaceholder(rows, cols);

    if (placeholder) {
      layout.destroyWindows();
      erase();
      layout.drawPlaceholder(rows, cols);
      int ch = getch();
      if (ch == 'q' || ch == 'Q') {
        running = false;
        break;
      }
      if (ch == KEY_RESIZE) {
        // will recompute on next loop iteration
        continue;
      }
      continue;
    }

    if (layout.windowCount() == 0) {
      Regions regs = layout.compute(rows, cols);
      layout.createWindows(regs);
      clear();
      refresh();
      layout.touchAndRefresh();
    }

    // Draw all panes
    drawTitlePane(layout.titleWin());
    drawMenuPane(layout.menuWin(), mainMenu, focus);
    drawFilterPane(layout.filterWin(), filterFrom, filterTo, importInput, categoryMenu, subcategoryMenu, focus);
    drawExpensesPane(layout.expensesWin(), focus, memory);
    drawTablePane(layout.tableWin(), table, focus);

    if (popup.isOpen()) {
      popup.draw();
    }
    if (popupGet.isOpen()) {
      popupGet.draw();
    }

    // Position cursor for edit mode inputs
    if (focus.isEdit()) {
      curs_set(1);
    } else if (popup.isOpen() && popup.popupMode() == Mode::Edit) {
      curs_set(1);
    } else if (popupGet.isOpen() && popupGet.popupMode() == Mode::Edit) {
      curs_set(1);
    } else {
      curs_set(0);
    }

    int ch = getch();

    if (ch == KEY_RESIZE) {
      getmaxyx(stdscr, rows, cols);
      layout.destroyWindows();
      if (layout.needsPlaceholder(rows, cols)) {
        erase();
        layout.drawPlaceholder(rows, cols);
      } else {
        Regions regs = layout.compute(rows, cols);
        layout.createWindows(regs);
        // touch and refresh underlying after recompute
        layout.touchAndRefresh();
        clear();
        refresh();
        if (popup.isOpen()) {
          popup.close();
          getmaxyx(stdscr, rows, cols);
          popup.open(rows, cols);
        }
        if (popupGet.isOpen()) {
          popupGet.close();
          getmaxyx(stdscr, rows, cols);
          popupGet.open(rows, cols);
        }
      }
      continue;
    }

    if (popup.isOpen()) {
      bool wasOpen = popup.isOpen();
      popup.handleKey(ch);
      if (!popup.isOpen() && wasOpen) {
        clear();
        refresh();
        layout.touchAndRefresh();
        if (popup.didSubmit()) {
          table.setRows(memory.viewAllExpenses());
          table.clearChecked();
        }
      }
      continue;
    }
    if (popupGet.isOpen()) {
      bool wasOpen = popupGet.isOpen();
      popupGet.handleKey(ch);
      if (!popupGet.isOpen() && wasOpen) {
        clear();
        refresh();
        layout.touchAndRefresh();
        if (popupGet.didSubmit()) {
          table.setRows(popupGet.filtered());
          focus.setCurrent(WidgetId::TableRow);
          table.clearChecked();
        }
      }
      continue;
    }

    WidgetId cur = focus.current();
    bool isEdit = focus.isEdit();

    // Global quit when not editing
    if (!isEdit && (ch == 'q' || ch == 'Q')) {
      running = false;
      break;
    }

    if (ch == 27) {  // Esc
      if (isEdit) {
        focus.exitEdit();
        curs_set(0);
        continue;
      }
      // In navigation, Esc does nothing (q quits)
      continue;
    }

    // Edit mode delegation for inputs
    if (isEdit) {
      Input* curInput = nullptr;
      if (cur == WidgetId::FilterFrom) curInput = &filterFrom;
      else if (cur == WidgetId::FilterTo) curInput = &filterTo;
      else if (cur == WidgetId::Import) curInput = &importInput;
      if (curInput) {
        if (curInput->handleKey(ch, true)) {
          continue;
        }
        // Not handled by input (e.g., Tab) -> try focus move
        WidgetId prev = focus.current();
        WidgetId nxt = focus.translate(ch);
        if (nxt != prev) {
          curs_set(0);
        }
        continue;
      } else {
        focus.exitEdit();
        curs_set(0);
      }
    }

    // Widget-internal j/k for Category/Subcategory/Table
    if (cur == WidgetId::Category) {
      if (ch == 'j' || ch == 'k' || ch == KEY_DOWN || ch == KEY_UP) {
        categoryMenu.handleKey(ch);
        syncSubMain();
        continue;
      }
    } else if (cur == WidgetId::Subcategory) {
      if (ch == 'j' || ch == 'k' || ch == KEY_DOWN || ch == KEY_UP) {
        subcategoryMenu.handleKey(ch);
        continue;
      }
    } else if (cur == WidgetId::TableRow) {
      if (ch == 'j' || ch == 'k' || ch == KEY_DOWN || ch == KEY_UP) {
        table.handleKey(ch);
        continue;
      }
      if (ch == ' ') {
        int sel = table.selectedIndex();
        if (sel >= 0) table.toggleChecked(sel);
        continue;
      }
      if (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n') {
        auto indices = table.checkedIndices();
        if (!indices.empty()) {
          // Collect ids for descending delete to avoid shift
          std::vector<std::pair<size_t, uint32_t>> toDelete;
          auto rows = table.rows();
          for (size_t idx : indices) {
            if (idx < rows.size()) toDelete.emplace_back(idx, rows[idx].id);
          }
          std::sort(toDelete.begin(), toDelete.end(),
                    [](auto& a, auto& b) { return a.second > b.second; });
          // Also sort by index descending as fallback if id duplicates
          for (auto& p : toDelete) {
            memory.deleteExpense(p.second);
          }
          table.setRows(memory.viewAllExpenses());
          table.clearChecked();
        }
        continue;
      }
    }

    // Enter handling for menu and inputs
    if (ch == 10 || ch == 13 || ch == KEY_ENTER || ch == '\n') {
      if (cur == WidgetId::MenuAdd) {
        getmaxyx(stdscr, rows, cols);
        popup.open(rows, cols);
        continue;
      }
      if (cur == WidgetId::MenuGet) {
        getmaxyx(stdscr, rows, cols);
        popupGet.open(rows, cols);
        continue;
      }
      if (cur == WidgetId::MenuRemove) {
        table.setRows(memory.viewAllExpenses());
        table.clearChecked();
        focus.setCurrent(WidgetId::TableRow);
        continue;
      }
      if (cur == WidgetId::FilterFrom || cur == WidgetId::FilterTo || cur == WidgetId::Import) {
        focus.enterEdit();
        curs_set(1);
        continue;
      }
    }

    // Printable auto-enter edit for inputs (navigation mode)
    if ((cur == WidgetId::FilterFrom || cur == WidgetId::FilterTo || cur == WidgetId::Import) &&
        ch >= 32 && ch <= 126) {
      focus.enterEdit();
      curs_set(1);
      Input* curInput = nullptr;
      if (cur == WidgetId::FilterFrom) curInput = &filterFrom;
      else if (cur == WidgetId::FilterTo) curInput = &filterTo;
      else curInput = &importInput;
      curInput->handleKey(ch, true);
      continue;
    }

    // Generic focus navigation via hjkl/Tab
    focus.translate(ch);
  }

  layout.destroyWindows();
  endwin();
  return 0;
}
