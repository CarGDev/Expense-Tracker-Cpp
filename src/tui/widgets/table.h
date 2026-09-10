#ifndef TUI_WIDGETS_TABLE_H
#define TUI_WIDGETS_TABLE_H

#include <string>
#include <vector>
#include <ncurses.h>

#include "../layout.h"
#include "../../memory/core.h"

// Table widget for bottom pane: header Id|[ ]|Category|Subcategory|Total|Date
// Rows are provided externally as vector<ExpenseRecord>. Supports row selection
// highlight, scroll offset, empty state, checkbox column [ ]/[x] via parallel
// vector<char> checked_, and responsive layout via Rect. Checkbox handling is
// generic (toggle/clear) — Space/Enter wiring is done in main loop (PR3).
class Table {
public:
  Table();

  void setRows(const std::vector<ExpenseRecord>& rows);
  const std::vector<ExpenseRecord>& rows() const;

  // For visual shell without data: populate with stub rows
  void setStubRows(size_t count);

  int selectedIndex() const;
  void setSelectedIndex(int idx);
  bool hasSelection() const;

  void moveUp();
  void moveDown();

  // Handles j/k and arrows for row navigation
  bool handleKey(int ch);

  int scrollOffset() const;

  void draw(WINDOW* win, const Rect& rect, bool focused) const;

  // Helpers for testing/layout
  static std::string header();
  static std::string formatAmount(double amount);
  static std::string formatDate(const std::chrono::system_clock::time_point& tp);

  // Checkbox API — parallel vector<char> keeps single highlight select generic
  void toggleChecked(int idx);
  std::vector<size_t> checkedIndices() const;
  void clearChecked();
  bool isChecked(int idx) const;
  size_t checkedCount() const;

private:
  std::vector<ExpenseRecord> rows_;
  std::vector<char> checked_;
  int selected_;
  int scrollOffset_;

  void clampSelection();
  void updateScroll(int visibleRows);
};

#endif
