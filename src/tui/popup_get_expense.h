#ifndef TUI_POPUP_GET_EXPENSE_H
#define TUI_POPUP_GET_EXPENSE_H

#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "../memory/core.h"

namespace tui {

ftxui::Component MakeGetExpenseModal(
    ExpenseMemory* memory,
    std::string* from_str,
    std::string* to_str,
    std::string* hint,
    std::vector<ExpenseRecord>* out_filtered,
    bool* did_submit,
    std::function<void()> on_close);

}  // namespace tui

#endif
