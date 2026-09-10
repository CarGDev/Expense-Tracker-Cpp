#include "popup_get_expense.h"

#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "../memory/core.h"

using namespace ftxui;

namespace tui {

Component MakeGetExpenseModal(
    ExpenseMemory* memory,
    std::string* from_str,
    std::string* to_str,
    std::string* hint,
    std::vector<ExpenseRecord>* out_filtered,
    bool* did_submit,
    std::function<void()> on_close) {
  auto from_input = Input(from_str, "YYYY-MM-DD");
  auto to_input = Input(to_str, "YYYY-MM-DD");
  auto submit = Button("Submit", [=] {
    if (!memory || !hint || !did_submit || !out_filtered) return;
    hint->clear();
    if (from_str->empty() || to_str->empty()) {
      *hint = "Both dates required YYYY-MM-DD";
      return;
    }
    auto pf = parseYYYYMMDD(*from_str);
    auto pt = parseYYYYMMDD(*to_str);
    if (!pf) {
      *hint = "Invalid date_from YYYY-MM-DD";
      return;
    }
    if (!pt) {
      *hint = "Invalid date_to YYYY-MM-DD";
      return;
    }
    if (*pf > *pt) {
      *hint = "date_from must be <= date_to";
      return;
    }
    *out_filtered = memory->getExpensesByDateTime(*pf, *pt);
    *did_submit = true;
    if (on_close) on_close();
  });
  auto cancel = Button("Cancel", on_close);
  return Container::Vertical({from_input, to_input, submit, cancel});
}

}  // namespace tui
