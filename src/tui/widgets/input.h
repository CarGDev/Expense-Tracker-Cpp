#ifndef TUI_WIDGETS_INPUT_H
#define TUI_WIDGETS_INPUT_H

#include <string>
#include <ncurses.h>

#include "../layout.h"

// Text field widget composable for PR3 popup and filter bars.
// Handles cursor, insertion/deletion, max length, placeholder,
// and visual-only variants for amount (x.xx) and date (YYYY-MM-DD).
// Nav vs edit: when isEdit is true, 'h'/'l' insert via tryInsert;
// arrows move cursor. Caller (FocusManager) decides mode.
enum class InputKind {
  Text,
  Amount,
  Date
};

class Input {
public:
  explicit Input(InputKind kind = InputKind::Text,
                 size_t maxLen = 32,
                 const std::string& placeholder = "");

  // Try to insert char at cursor; enforces kind rules.
  // Returns false if rejected (maxLen, x.xx, date 10-char, etc.).
  bool tryInsert(char c);

  void eraseBackspace(); // KEY_BACKSPACE / 127 / 8
  void eraseDel();       // KEY_DC

  void moveLeft();
  void moveRight();
  void moveHome();
  void moveEnd();

  // Handles edit-mode keys: backspace, del, arrows, home/end, printable.
  // When isEdit=false, returns false (no input handling).
  // When isEdit=true, 'h'/'l' fall through to tryInsert, arrows move.
  bool handleKey(int ch, bool isEdit);

  const std::string& value() const;
  void setValue(const std::string& v);
  size_t cursor() const;
  void setCursor(size_t pos);

  const std::string& placeholder() const;
  void setPlaceholder(const std::string& p);
  size_t maxLen() const;
  void setMaxLen(size_t n);
  InputKind kind() const;

  // Display string: value if non-empty, otherwise placeholder (if any).
  std::string display() const;
  bool isEmpty() const;

  void clear();

  // Draw into win at rect. If focused, cursor is positioned and
  // field is highlighted with A_REVERSE. Placeholder is dimmed.
  // rect.y/x are window-relative offsets, rect.w is available width.
  void draw(WINDOW* win, const Rect& rect, bool focused) const;

private:
  InputKind kind_;
  size_t maxLen_;
  std::string placeholder_;
  std::string buf_;
  size_t cur_;

  bool validateAmountInsert(const std::string& tmp) const;
};

#endif
