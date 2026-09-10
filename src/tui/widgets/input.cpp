#include "input.h"

#include <string>

bool ValidateAmountString(const std::string& s) {
  if (s.empty()) return true;
  size_t dots = 0;
  for (char c : s) {
    if (c == '.') {
      dots++;
    } else if (c < '0' || c > '9') {
      return false;
    }
  }
  if (dots > 1) return false;
  auto pos = s.find('.');
  if (pos != std::string::npos) {
    size_t decimals = s.size() - pos - 1;
    if (decimals > 2) return false;
  }
  return true;
}

bool IsValidAmountInsertion(const std::string& buffer, size_t cursor, char c) {
  if (c != '.' && (c < '0' || c > '9')) return false;
  std::string tmp = buffer;
  if (cursor > tmp.size()) cursor = tmp.size();
  tmp.insert(tmp.begin() + static_cast<long>(cursor), c);
  if (tmp.size() > kAmountMaxLen) return false;
  return ValidateAmountString(tmp);
}
