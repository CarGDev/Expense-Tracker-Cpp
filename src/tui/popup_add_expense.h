#ifndef TUI_POPUP_ADD_EXPENSE_H
#define TUI_POPUP_ADD_EXPENSE_H

#include <memory>
#include <string>
#include <vector>

#include <ncurses.h>

#include "focus.h"
#include "layout.h"
#include "widgets/input.h"
#include "widgets/menu.h"
#include "../memory/core.h"

// Centered modal for adding an expense.
// RAII owns its WINDOW* via WindowPtr; destroys on close/dtor.
// Fields: Category Menu -> Subcategory Menu (dependent via getSubcategories),
// Total Input (Amount kind), Date Input (Date kind), Submit button.
// Focus sub-manager handles Tab-only inside popup, Category->Subcategory wiring,
// x-offset rendering, centered via terminal size, box/border, A_REVERSE highlight.
// Submit parses Amount (stod) and Date (parseYYYYMMDD) -> addExpense.
class PopupAddExpense {
 public:
  PopupAddExpense();
  ~PopupAddExpense();

  bool isOpen() const;
  void open(int termRows, int termCols);
  void close();

  // Handles a key while popup is open.
  // Esc/q closes, Enter on Submit stubs submit, Category change resets Subcategory.
  // Returns true if key was consumed.
  bool handleKey(int ch);

  void draw();

  Menu& categoryMenu();
  Menu& subcategoryMenu();
  Input& totalInput();
  Input& dateInput();
  WidgetId popupFocus() const;
  bool didSubmit() const;
  Rect popupRect() const;
  Mode popupMode() const;
  const std::string& hint() const;
  void clearHint();
  void setMemory(ExpenseMemory& mem);

  // Sync subcategory list to current category selection.
  void syncSubcategory();

 private:
  bool open_;
  Rect rect_;
  WindowPtr win_;
  Menu categoryMenu_;
  Menu subcategoryMenu_;
  Input totalInput_;
  Input dateInput_;
  WidgetId cur_;
  Mode mode_;
  bool didSubmit_;
  std::vector<WidgetId> order_;
  ExpenseMemory* memory_ = nullptr;
  std::string hint_;

  int indexOf(WidgetId id) const;
  WidgetId moveNext();
  WidgetId movePrev();
  WidgetId translate(int ch);
};

#endif
