#ifndef TUI_FOCUS_H
#define TUI_FOCUS_H

#include <vector>

// Focus management for TUI shell.
// Tab-only navigation: Tab moves next, BTAB moves previous with wrap.
// Edit mode: Tab/BTAB exits edit and moves, Esc exits edit.

enum class WidgetId {
  MenuAdd,
  MenuGet,
  MenuRemove,
  FilterFrom,
  FilterTo,
  Import,
  Category,
  Subcategory,
  Expenses,
  TableRow,
  // Popup fields (for future PR3, included here to keep enum stable)
  PopupCategory,
  PopupSubcategory,
  PopupTotal,
  PopupDate,
  PopupSubmit
};

enum class Mode {
  Navigation,
  Edit
};

class FocusManager {
public:
  FocusManager();

  WidgetId current() const;
  Mode mode() const;
  bool isEdit() const;

  void setCurrent(WidgetId id);
  void enterEdit();
  void exitEdit();

  // Translate a keypress into new focus — Tab/BTAB only.
  // Navigation: Tab→next, BTAB→prev, wrap. hjkl/arrows are inert.
  // Edit: Tab/BTAB exits edit (Navigation) and moves; Esc exits edit.
  WidgetId translate(int ch);

  // Explicit stepping used by translate and by tests.
  WidgetId moveNext();
  WidgetId movePrev();

private:
  WidgetId cur_;
  Mode mode_;
  std::vector<WidgetId> order_;

  int indexOf(WidgetId id) const;
};

#endif
