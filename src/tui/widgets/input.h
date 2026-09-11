#ifndef TUI_WIDGETS_INPUT_H
#define TUI_WIDGETS_INPUT_H

#include <string>

// InputKind validation preserved for FTXUI.
// FTXUI Input + CatchEvent uses these helpers to filter characters.

enum class InputKind { Text, Amount, Date };

// Validate full amount string: digits, at most one '.', at most 2 decimals.
bool ValidateAmountString(const std::string &s);

// Check whether inserting c at position cursor in buffer would keep amount
// valid. Used inside CatchEvent to decide filtering.
bool IsValidAmountInsertion(const std::string &buffer, size_t cursor, char c);

// Helpers for CatchEvent filtering:
inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }

// Max length constants (mirrors old Input maxLen behavior)
constexpr size_t kDateMaxLen = 10;
constexpr size_t kAmountMaxLen = 32;
constexpr size_t kTextMaxLen = 32;

// Placeholder strings
inline const std::string &AmountPlaceholder() {
  static const std::string p = "x.xx";
  return p;
}
inline const std::string &DatePlaceholder() {
  static const std::string p = "YYYY-MM-DD";
  return p;
}

#endif
