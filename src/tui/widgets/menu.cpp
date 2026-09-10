#include "menu.h"

#include <algorithm>

Menu::Menu(const std::vector<std::string>& items)
  : items_(&items)
  , highlight_(items.empty() ? -1 : 0) {
}

void Menu::setItems(const std::vector<std::string>& items) {
  items_ = &items;
  if (items_->empty()) {
    highlight_ = -1;
  } else {
    highlight_ = 0;
  }
}

const std::vector<std::string>& Menu::items() const {
  return *items_;
}

int Menu::selectedIndex() const {
  return highlight_;
}

void Menu::setSelectedIndex(int idx) {
  if (!items_ || items_->empty()) {
    highlight_ = -1;
    return;
  }
  if (idx < 0) idx = 0;
  if (idx >= static_cast<int>(items_->size())) idx = static_cast<int>(items_->size()) - 1;
  highlight_ = idx;
}

bool Menu::hasSelection() const {
  return highlight_ >= 0 && items_ && !items_->empty() && highlight_ < static_cast<int>(items_->size());
}

const std::string& Menu::selected() const {
  if (hasSelection()) return (*items_)[static_cast<size_t>(highlight_)];
  return emptySelection();
}

const std::string& Menu::emptySelection() {
  static const std::string empty;
  return empty;
}

void Menu::moveUp() {
  if (!items_ || items_->empty()) return;
  highlight_--;
  if (highlight_ < 0) highlight_ = static_cast<int>(items_->size()) - 1;
}

void Menu::moveDown() {
  if (!items_ || items_->empty()) return;
  highlight_++;
  if (highlight_ >= static_cast<int>(items_->size())) highlight_ = 0;
}

bool Menu::handleKey(int ch) {
  if (!items_ || items_->empty()) return false;
  if (ch == 'k' || ch == KEY_UP) {
    moveUp();
    return true;
  }
  if (ch == 'j' || ch == KEY_DOWN) {
    moveDown();
    return true;
  }
  return false;
}

void Menu::setHighlight(int idx) {
  setSelectedIndex(idx);
}

void Menu::draw(WINDOW* win, const Rect& rect, bool focused, int xOffset) const {
  if (!win) return;
  if (!items_) return;
  int maxRows = rect.h;
  int maxCols = rect.w - xOffset;
  if (maxRows <= 0 || maxCols <= 0) return;

  // Clear area
  for (int r = 0; r < maxRows; ++r) {
    for (int c = 0; c < rect.w; ++c) {
      mvwaddch(win, rect.y + r, rect.x + c, ' ');
    }
  }

  if (items_->empty()) {
    std::string empty = "(empty)";
    mvwprintw(win, rect.y, rect.x + xOffset, "%s", empty.c_str());
    return;
  }

  // For simple dropdown, show visible slice starting at 0.
  // If highlight beyond maxRows, we could scroll, but keeper generic:
  // compute start so highlight is visible.
  int start = 0;
  if (highlight_ >= maxRows) {
    start = highlight_ - maxRows + 1;
  }

  for (int i = 0; i < maxRows; ++i) {
    int idx = start + i;
    if (idx >= static_cast<int>(items_->size())) break;
    const std::string& label = (*items_)[static_cast<size_t>(idx)];
    std::string clipped = label.substr(0, static_cast<size_t>(maxCols));
    bool isHighlight = (idx == highlight_) && focused;
    if (isHighlight) wattron(win, A_REVERSE);
    mvwprintw(win, rect.y + i, rect.x + xOffset, "%s", clipped.c_str());
    if (isHighlight) wattroff(win, A_REVERSE);
  }
}
