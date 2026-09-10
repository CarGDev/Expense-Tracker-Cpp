#include "input.h"

#include <algorithm>

Input::Input(InputKind kind, size_t maxLen, const std::string& placeholder)
  : kind_(kind)
  , maxLen_(maxLen)
  , placeholder_(placeholder)
  , cur_(0) {
  if (kind_ == InputKind::Date) {
    maxLen_ = 10;
    if (placeholder_.empty()) {
      placeholder_ = "YYYY-MM-DD";
    }
  } else if (kind_ == InputKind::Amount) {
    if (placeholder_.empty()) {
      placeholder_ = "x.xx";
    }
  }
}

bool Input::validateAmountInsert(const std::string& tmp) const {
  size_t dots = 0;
  for (char c : tmp) {
    if (c == '.') dots++;
  }
  if (dots > 1) return false;
  auto pos = tmp.find('.');
  if (pos != std::string::npos) {
    size_t decimals = tmp.size() - pos - 1;
    if (decimals > 2) return false;
  }
  return true;
}

bool Input::tryInsert(char c) {
  if (buf_.size() >= maxLen_) return false;
  if (c < 32 || c > 126) return false;
  if (kind_ == InputKind::Amount) {
    if (c != '.' && (c < '0' || c > '9')) return false;
    std::string tmp = buf_;
    tmp.insert(tmp.begin() + static_cast<long>(cur_), c);
    if (!validateAmountInsert(tmp)) return false;
  } else if (kind_ == InputKind::Date) {
    // 10-char text-only, accept any printable up to maxLen
    // no chrono validation per spec
  }
  // Text: any printable
  buf_.insert(buf_.begin() + static_cast<long>(cur_), c);
  cur_++;
  return true;
}

void Input::eraseBackspace() {
  if (cur_ == 0 || buf_.empty()) return;
  buf_.erase(buf_.begin() + static_cast<long>(cur_ - 1));
  cur_--;
}

void Input::eraseDel() {
  if (cur_ >= buf_.size()) return;
  buf_.erase(buf_.begin() + static_cast<long>(cur_));
}

void Input::moveLeft() {
  if (cur_ > 0) cur_--;
}

void Input::moveRight() {
  if (cur_ < buf_.size()) cur_++;
}

void Input::moveHome() {
  cur_ = 0;
}

void Input::moveEnd() {
  cur_ = buf_.size();
}

bool Input::handleKey(int ch, bool isEdit) {
  if (!isEdit) return false;
  if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
    eraseBackspace();
    return true;
  }
  if (ch == KEY_DC) {
    eraseDel();
    return true;
  }
  if (ch == KEY_LEFT) {
    moveLeft();
    return true;
  }
  if (ch == KEY_RIGHT) {
    moveRight();
    return true;
  }
  if (ch == KEY_HOME) {
    moveHome();
    return true;
  }
  if (ch == KEY_END) {
    moveEnd();
    return true;
  }
  if (ch >= 32 && ch <= 126) {
    return tryInsert(static_cast<char>(ch));
  }
  return false;
}

const std::string& Input::value() const {
  return buf_;
}

void Input::setValue(const std::string& v) {
  // Enforce maxLen and kind rules when setting directly (truncate if needed)
  std::string truncated = v.substr(0, maxLen_);
  if (kind_ == InputKind::Amount) {
    // filter to valid prefix: keep valid prefix, drop invalid tail for visual shell
    std::string filtered;
    for (char c : truncated) {
      std::string tmp = filtered;
      tmp.insert(tmp.begin() + static_cast<long>(filtered.size()), c);
      if (c != '.' && (c < '0' || c > '9')) continue;
      if (!validateAmountInsert(tmp)) continue;
      filtered = tmp;
    }
    buf_ = filtered;
  } else {
    buf_ = truncated;
  }
  cur_ = buf_.size();
}

size_t Input::cursor() const {
  return cur_;
}

void Input::setCursor(size_t pos) {
  cur_ = std::min(pos, buf_.size());
}

const std::string& Input::placeholder() const {
  return placeholder_;
}

void Input::setPlaceholder(const std::string& p) {
  placeholder_ = p;
}

size_t Input::maxLen() const {
  return maxLen_;
}

void Input::setMaxLen(size_t n) {
  if (kind_ == InputKind::Date) {
    maxLen_ = 10;
    return;
  }
  maxLen_ = n;
  if (buf_.size() > maxLen_) {
    buf_.resize(maxLen_);
    if (cur_ > buf_.size()) cur_ = buf_.size();
  }
}

InputKind Input::kind() const {
  return kind_;
}

std::string Input::display() const {
  if (!buf_.empty()) return buf_;
  return placeholder_;
}

bool Input::isEmpty() const {
  return buf_.empty();
}

void Input::clear() {
  buf_.clear();
  cur_ = 0;
}

void Input::draw(WINDOW* win, const Rect& rect, bool focused) const {
  if (!win) return;
  bool showingPlaceholder = buf_.empty() && !placeholder_.empty();
  std::string text = showingPlaceholder ? placeholder_ : buf_;
  // Clip to available width
  int avail = rect.w;
  if (avail <= 0) return;
  if (static_cast<int>(text.size()) > avail) {
    text = text.substr(0, avail);
  }
  // Clear field area
  for (int i = 0; i < avail; ++i) {
    mvwaddch(win, rect.y, rect.x + i, ' ');
  }
  if (showingPlaceholder) {
    wattron(win, A_DIM);
    mvwprintw(win, rect.y, rect.x, "%s", text.c_str());
    wattroff(win, A_DIM);
  } else {
    if (focused) wattron(win, A_REVERSE);
    mvwprintw(win, rect.y, rect.x, "%s", text.c_str());
    if (focused) wattroff(win, A_REVERSE);
  }
  // Place cursor if focused and not showing placeholder
  if (focused && !showingPlaceholder) {
    int cx = rect.x + static_cast<int>(cur_);
    if (cx >= rect.x + avail) cx = rect.x + avail - 1;
    if (cx < rect.x) cx = rect.x;
    wmove(win, rect.y, cx);
  } else if (focused && showingPlaceholder) {
    wmove(win, rect.y, rect.x);
  }
}
