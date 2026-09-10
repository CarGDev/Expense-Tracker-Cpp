#include "layout.h"

#include <string>

Layout::~Layout() {
  destroyWindows();
}

bool Layout::needsPlaceholder(int rows, int cols) const {
  return rows < 24 || cols < 80;
}

Regions Layout::compute(int rows, int cols) const {
  Regions r;
  if (needsPlaceholder(rows, cols)) {
    return r;
  }
  // Design: menuH=6 fixed, tableH=rows-titleH-menuH-1; at 80x24 => 6 vs ~14.
  const int titleH = 3;
  const int menuH = 6;
  int tableH = rows - titleH - menuH - 1;
  if (tableH < 1) tableH = 1;
  int menuW = cols / 4; // 20 at 80 cols
  int expensesW = cols / 4;
  int filterW = cols - menuW - expensesW;
  if (filterW < 20) {
    filterW = 20;
    menuW = (cols - filterW) / 2;
    expensesW = cols - menuW - filterW;
  }

  r.title = {0, 0, titleH, cols};
  r.menu = {titleH, 0, menuH, menuW};
  r.filter = {titleH, menuW, menuH, filterW};
  r.expenses = {titleH, menuW + filterW, menuH, expensesW};
  r.table = {titleH + menuH, 0, tableH, cols};
  return r;
}

void Layout::createWindows(const Regions& regs) {
  destroyWindows();
  lastRegs_ = regs;
  if (regs.title.w == 0 && regs.menu.w == 0) {
    return;
  }
  auto makeWin = [this](const Rect& rc) {
    if (rc.h <= 0 || rc.w <= 0) {
      return;
    }
    WINDOW* raw = newwin(rc.h, rc.w, rc.y, rc.x);
    if (!raw) {
      return;
    }
    // Enable keypad for consistent input handling
    keypad(raw, TRUE);
    box(raw, 0, 0);
    wrefresh(raw);
    windows_.emplace_back(raw);
  };

  makeWin(regs.title);
  makeWin(regs.menu);
  makeWin(regs.filter);
  makeWin(regs.expenses);
  makeWin(regs.table);
}

void Layout::destroyWindows() {
  windows_.clear();
  lastRegs_ = Regions{};
}

void Layout::touchAndRefresh() {
  for (auto& w : windows_) {
    if (w) {
      touchwin(w.get());
      wrefresh(w.get());
    }
  }
}

void Layout::drawPlaceholder(int rows, int cols) const {
  // Use stdscr for minimal placeholder; app loop will have called erase().
  std::string msg = "Terminal too small: need 80x24";
  std::string hint = std::to_string(cols) + "x" + std::to_string(rows) + " - resize or enlarge";
  int y = rows / 2;
  int x1 = (cols - static_cast<int>(msg.size())) / 2;
  int x2 = (cols - static_cast<int>(hint.size())) / 2;
  if (x1 < 0) x1 = 0;
  if (x2 < 0) x2 = 0;
  mvprintw(y, x1, "%s", msg.c_str());
  mvprintw(y + 1, x2, "%s", hint.c_str());
  refresh();
}

const Regions& Layout::lastRegions() const {
  return lastRegs_;
}

size_t Layout::windowCount() const {
  return windows_.size();
}

WINDOW* Layout::windowAt(size_t idx) const {
  if (idx >= windows_.size()) return nullptr;
  return windows_[idx].get();
}

WINDOW* Layout::titleWin() const {
  return windowAt(0);
}
WINDOW* Layout::menuWin() const {
  return windowAt(1);
}
WINDOW* Layout::filterWin() const {
  return windowAt(2);
}
WINDOW* Layout::expensesWin() const {
  return windowAt(3);
}
WINDOW* Layout::tableWin() const {
  return windowAt(4);
}
