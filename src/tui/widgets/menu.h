#ifndef TUI_WIDGETS_MENU_H
#define TUI_WIDGETS_MENU_H

#include <string>
#include <vector>
#include <ncurses.h>

#include "../layout.h"

// Dropdown/menu widget composable for Category/Subcategory and generic lists.
// Holds non-owning reference to vector<string> so dependent filtering
// (category -> subcategory via getSubcategories) can swap the source
// without copying. Highlight index drives rendering with A_REVERSE
// and optional x-offset.
class Menu {
public:
  explicit Menu(const std::vector<std::string>& items);

  void setItems(const std::vector<std::string>& items);
  const std::vector<std::string>& items() const;

  int selectedIndex() const;
  void setSelectedIndex(int idx);
  bool hasSelection() const;
  const std::string& selected() const;
  static const std::string& emptySelection();

  void moveUp();
  void moveDown();

  // Handles j/k and arrows for highlight movement.
  // Returns true if key was navigation; Enter/Esc are left to caller
  // (popup will interpret Enter as select, Esc as cancel).
  // When handled, highlight is updated.
  bool handleKey(int ch);

  // xOffset shifts rendering right (for indented menus).
  void draw(WINDOW* win, const Rect& rect, bool focused, int xOffset = 0) const;

  // For testing: allow direct set
  void setHighlight(int idx);

private:
  const std::vector<std::string>* items_;
  int highlight_;
};

#endif
