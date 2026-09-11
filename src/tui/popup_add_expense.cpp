#include "popup_add_expense.h"

#include <chrono>
#include <string>
#include <vector>

#include "../categories/categories.h"
#include "../memory/core.h"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace tui {

Component
MakeAddExpenseModal(ExpenseMemory *memory, std::string *category_selected,
                    std::string *subcategory_selected, std::string *amount_str,
                    std::string *date_str, std::vector<std::string> *categories,
                    std::vector<std::string> *subcategories, std::string *hint,
                    bool *did_submit, std::function<void()> on_close,
                    std::function<void()> on_subcategory_sync) {
  // Category menu: use selected index mapping
  int cat_selected = 0;
  int sub_selected = 0;
  // Note: This is a lightweight builder; main.cpp owns the actual state.
  // We provide a simple vertical container as placeholder TBH.
  // Real wiring is done in main.cpp; this demonstrates FTXUI usage.
  auto cat_menu = Menu(categories, &cat_selected);
  auto sub_menu = Menu(subcategories, &sub_selected);
  auto amount_input = Input(amount_str, "x.xx");
  auto date_input = Input(date_str, "YYYY-MM-DD");
  auto submit = Button("Submit", [&] {
    if (!memory || !hint || !did_submit)
      return;
    hint->clear();
    if (amount_str->empty()) {
      *hint = "Amount required";
      return;
    }
    double amt = 0;
    try {
      size_t pos = 0;
      amt = std::stod(*amount_str, &pos);
      if (pos != amount_str->size())
        throw std::invalid_argument("trailing");
    } catch (...) {
      *hint = "Invalid amount";
      return;
    }
    auto dt = std::chrono::system_clock::now();
    if (!date_str->empty()) {
      auto parsed = parseYYYYMMDD(*date_str);
      if (!parsed) {
        *hint = "Invalid date YYYY-MM-DD";
        return;
      }
      dt = *parsed;
    }
    ExpenseRecord rec;
    rec.id = 0;
    rec.amount = amt;
    rec.category = *category_selected;
    rec.sub_category = *subcategory_selected;
    rec.datetime = dt;
    memory->addExpense(rec);
    *did_submit = true;
    amount_str->clear();
    date_str->clear();
    hint->clear();
    if (on_close)
      on_close();
  });
  auto cancel = Button("Cancel", on_close);
  return Container::Vertical(
      {cat_menu, sub_menu, amount_input, date_input, submit, cancel});
}

} // namespace tui
