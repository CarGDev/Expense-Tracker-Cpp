#ifndef TUI_POPUP_ADD_EXPENSE_H
#define TUI_POPUP_ADD_EXPENSE_H

#include <string>
#include <vector>

#include "../memory/core.h"
#include "ftxui/component/component.hpp"

// FTXUI modal for adding an expense.
// Implemented as FTXUI Components (Menu + Input + Button) rendered via
// dbox overlay in src/main.cpp. This header provides helper to build the
// modal component tree; main.cpp wires the actual UI.

namespace tui {

ftxui::Component
MakeAddExpenseModal(ExpenseMemory *memory, std::string *category_selected,
                    std::string *subcategory_selected, std::string *amount_str,
                    std::string *date_str, std::vector<std::string> *categories,
                    std::vector<std::string> *subcategories, std::string *hint,
                    bool *did_submit, std::function<void()> on_close,
                    std::function<void()> on_subcategory_sync);

} // namespace tui

#endif
