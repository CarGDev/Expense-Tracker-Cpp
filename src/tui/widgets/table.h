#ifndef TUI_WIDGETS_TABLE_H
#define TUI_WIDGETS_TABLE_H

#include <chrono>
#include <string>
#include <vector>

#include "../../memory/core.h"

// Table data model for FTXUI Renderer.
// Preserves semantics of previous Table: header, format helpers, checkbox logic.

class Table {
 public:
  Table();

  void setRows(const std::vector<ExpenseRecord>& rows);
  const std::vector<ExpenseRecord>& rows() const;

  void setStubRows(size_t count);

  int selectedIndex() const;
  void setSelectedIndex(int idx);
  bool hasSelection() const;

  void moveUp();
  void moveDown();

  int scrollOffset() const;

  // Checkbox API — parallel vector<char> keeps single highlight select generic
  void toggleChecked(int idx);
  std::vector<size_t> checkedIndices() const;
  void clearChecked();
  bool isChecked(int idx) const;
  size_t checkedCount() const;

  // Helpers for testing/layout
  static std::string header();
  static std::string formatAmount(double amount);
  static std::string formatDate(const std::chrono::system_clock::time_point& tp);

 private:
  std::vector<ExpenseRecord> rows_;
  std::vector<char> checked_;
  int selected_;
  int scrollOffset_;

  void clampSelection();
  void updateScroll(int visibleRows);
};

#endif
