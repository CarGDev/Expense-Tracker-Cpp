#ifndef TUI_LAYOUT_H
#define TUI_LAYOUT_H

#include <memory>
#include <vector>

#include <ncurses.h>

// Geometry helpers for TUI shell.
// RAII choice: Layout owns WINDOW* via unique_ptr<WINDOW, decltype(&delwin)>
// so that KEY_RESIZE recompute cannot leak windows. Manual delwin was
// rejected as leak-prone. Alternative panel was rejected (extra dep).
// Regions: titleH=3 fixed, menuH=6 fixed, tableH=rows-titleH-menuH-1.
// At 80x24 this yields 3+6+14=23 (plus 1 border gap) — menu (6) < table (~14).
// Responsive: menu/filter/expenses share menuH row; table fills remainder.

struct Rect {
  int y = 0;
  int x = 0;
  int h = 0;
  int w = 0;
};

struct Regions {
  Rect title;
  Rect menu;
  Rect filter;
  Rect expenses;
  Rect table;
};

// Window deleter for unique_ptr
struct WindowDeleter {
  void operator()(WINDOW* w) const {
    if (w) {
      delwin(w);
    }
  }
};

using WindowPtr = std::unique_ptr<WINDOW, WindowDeleter>;

class Layout {
public:
  Layout() = default;
  ~Layout();

  // Returns true if terminal too small for shell (<80x24)
  bool needsPlaceholder(int rows, int cols) const;

  // Compute region geometry from current getmaxyx. Pure, no ncurses calls
  // except dimensions passed in, so it is unit-testable.
  Regions compute(int rows, int cols) const;

  // Create windows for given regions. Destroys any existing windows first.
  // Each window is boxed and refreshed. No-op if needsPlaceholder.
  void createWindows(const Regions& regs);

  // Destroy all owned windows (RAII also does this on dtor).
  void destroyWindows();

  // For resize: touch underlying windows and refresh.
  // Caller should call destroyWindows + compute + createWindows then this.
  void touchAndRefresh();

  // Draw placeholder for <80x24 terminals. Uses stdscr directly.
  void drawPlaceholder(int rows, int cols) const;

  // Accessors for testing
  const Regions& lastRegions() const;
  size_t windowCount() const;

  // Window accessors for shell rendering (order: title, menu, filter, expenses, table).
  WINDOW* windowAt(size_t idx) const;
  WINDOW* titleWin() const;
  WINDOW* menuWin() const;
  WINDOW* filterWin() const;
  WINDOW* expensesWin() const;
  WINDOW* tableWin() const;

 private:
  Regions lastRegs_{};
  std::vector<WindowPtr> windows_;
};

#endif
