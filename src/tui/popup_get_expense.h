#ifndef TUI_POPUP_GET_EXPENSE_H
#define TUI_POPUP_GET_EXPENSE_H

#include <memory>
#include <string>
#include <vector>

#include <ncurses.h>

#include "focus.h"
#include "layout.h"
#include "widgets/input.h"
#include "../memory/core.h"

// Centered modal for filtering expenses by date range.
// 14x58 centered, two Date inputs (from/to) + Submit/Cancel.
// Tab-only navigation, Submit validates both dates -> memory.getExpensesByDateTime -> setRows.
class PopupGetExpense {
 public:
  PopupGetExpense();
  ~PopupGetExpense();

  bool isOpen() const;
  void open(int termRows, int termCols);
  void close();

  bool handleKey(int ch);
  void draw();

  Input& fromInput();
  Input& toInput();
  bool didSubmit() const;
  const std::string& hint() const;
  void clearHint();
  void setMemory(ExpenseMemory& mem);
  const std::vector<ExpenseRecord>& filtered() const;
  Rect popupRect() const;
  Mode popupMode() const;

 private:
  bool open_;
  Rect rect_;
  WindowPtr win_;
  Input fromInput_;
  Input toInput_;
  int cur_; // 0:from 1:to 2:submit 3:cancel
  Mode mode_;
  bool didSubmit_;
  ExpenseMemory* memory_ = nullptr;
  std::string hint_;
  std::vector<ExpenseRecord> filtered_;

  int indexOf(int id) const;
  int moveNext();
  int movePrev();
  int translate(int ch);
};

#endif
