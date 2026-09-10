#include "focus.h"

#include <ncurses.h>

FocusManager::FocusManager()
  : cur_(WidgetId::MenuAdd)
  , mode_(Mode::Navigation)
  , order_({
      WidgetId::MenuAdd,
      WidgetId::MenuGet,
      WidgetId::MenuRemove,
      WidgetId::FilterFrom,
      WidgetId::FilterTo,
      WidgetId::Import,
      WidgetId::Category,
      WidgetId::Subcategory,
      WidgetId::Expenses,
      WidgetId::TableRow
    }) {
}

WidgetId FocusManager::current() const {
  return cur_;
}

Mode FocusManager::mode() const {
  return mode_;
}

bool FocusManager::isEdit() const {
  return mode_ == Mode::Edit;
}

void FocusManager::setCurrent(WidgetId id) {
  cur_ = id;
}

void FocusManager::enterEdit() {
  mode_ = Mode::Edit;
}

void FocusManager::exitEdit() {
  mode_ = Mode::Navigation;
}

int FocusManager::indexOf(WidgetId id) const {
  for (size_t i = 0; i < order_.size(); ++i) {
    if (order_[i] == id) {
      return static_cast<int>(i);
    }
  }
  return 0;
}

WidgetId FocusManager::moveNext() {
  int idx = indexOf(cur_);
  idx = (idx + 1) % static_cast<int>(order_.size());
  cur_ = order_[idx];
  return cur_;
}

WidgetId FocusManager::movePrev() {
  int idx = indexOf(cur_);
  idx = (idx - 1 + static_cast<int>(order_.size())) % static_cast<int>(order_.size());
  cur_ = order_[idx];
  return cur_;
}

WidgetId FocusManager::translate(int ch) {
  if (mode_ == Mode::Edit) {
    if (ch == 27) { // Esc
      exitEdit();
      return cur_;
    }
    if (ch == '\t') {
      exitEdit();
      return moveNext();
    }
    if (ch == KEY_BTAB) {
      exitEdit();
      return movePrev();
    }
    return cur_;
  }

  if (ch == '\t') {
    return moveNext();
  }
  if (ch == KEY_BTAB) {
    return movePrev();
  }
  return cur_;
}
