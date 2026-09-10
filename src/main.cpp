#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "categories/categories.h"
#include "memory/core.h"
#include "tui/widgets/input.h"
#include "tui/widgets/table.h"

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

int main() {
  ExpenseMemory memory;
  Table table;
  table.setStubRows(0);

  // State
  std::string filterFromStr;
  std::string filterToStr;
  std::string importStr;

  std::string addAmountStr;
  std::string addDateStr;
  std::string addHint;

  std::string getFromStr;
  std::string getToStr;
  std::string getHint;

  bool showAdd = false;
  bool showGet = false;

  // Main menu
  std::vector<std::string> mainItems = {"add_expense", "get_expenses", "remove_expenses"};
  int mainSelected = 0;

  // Filter category/subcategory
  int filterCategorySelected = 0;
  int filterSubSelected = 0;
  std::vector<std::string> filterSubs;
  if (!categories.empty()) {
    filterSubs = getSubcategories(categories[0]);
  }

  // Add modal category/subcategory
  int addCategorySelected = 0;
  int addSubSelected = 0;
  std::vector<std::string> addSubs;
  if (!categories.empty()) {
    addSubs = getSubcategories(categories[0]);
  }

  auto screen = ScreenInteractive::Fullscreen();

  // ----- Menu -----
  MenuOption mainOpt;
  mainOpt.on_enter = [&] {
    if (mainSelected == 0) {
      showAdd = true;
    } else if (mainSelected == 1) {
      showGet = true;
    } else if (mainSelected == 2) {
      table.setRows(memory.viewAllExpenses());
      table.clearChecked();
      if (!table.rows().empty()) table.setSelectedIndex(0);
    }
  };
  auto mainMenu = Menu(&mainItems, &mainSelected, mainOpt);

  // ----- Filter inputs -----
  auto filterFromInput = Input(&filterFromStr, "YYYY-MM-DD");
  auto filterToInput = Input(&filterToStr, "YYYY-MM-DD");
  auto importInput = Input(&importStr, "input");

  // Date validation: max 10 chars
  filterFromInput |= CatchEvent([&](Event e) {
    if (e.is_character() && filterFromStr.size() >= 10) return true;
    return false;
  });
  filterToInput |= CatchEvent([&](Event e) {
    if (e.is_character() && filterToStr.size() >= 10) return true;
    return false;
  });

  // ----- Filter category menus -----
  MenuOption filterCatOpt;
  filterCatOpt.on_change = [&] {
    if (!categories.empty() && filterCategorySelected >= 0 &&
        filterCategorySelected < (int)categories.size()) {
      const auto& subs = getSubcategories(categories[filterCategorySelected]);
      filterSubs = subs;
      filterSubSelected = 0;
    } else {
      filterSubs.clear();
      filterSubSelected = 0;
    }
  };
  auto filterCategoryMenu = Menu(&categories, &filterCategorySelected, filterCatOpt);
  auto filterSubMenu = Menu(&filterSubs, &filterSubSelected);

  // ----- Table -----
  auto tableInner = Renderer([&](bool focused) {
    Elements els;
    els.push_back(text(Table::header()) | bold);
    els.push_back(separator());
    if (table.rows().empty()) {
      els.push_back(text("No expenses — table shell (visual only)") | dim | hcenter);
    } else {
      for (size_t i = 0; i < table.rows().size(); ++i) {
        const auto& r = table.rows()[i];
        std::string chk = table.isChecked((int)i) ? "[x]" : "[ ]";
        std::string line = std::to_string(r.id) + " | " + chk + " | " + r.category + " | " +
                           r.sub_category + " | " + Table::formatAmount(r.amount) + " | " +
                           Table::formatDate(r.datetime);
        Element row = text(line);
        if ((int)i == table.selectedIndex() && focused) {
          row = row | inverted;
        } else if ((int)i == table.selectedIndex()) {
          row = row | bold;
        }
        if (table.isChecked((int)i)) {
          row = row | color(Color::Green);
        }
        els.push_back(row);
      }
    }
    return vbox(els) | flex;
  });

  Component tableComponent = tableInner;
  tableComponent |= CatchEvent([&](Event e) {
    if (table.rows().empty()) return false;
    if (e == Event::ArrowUp || e == Event::Character('k')) {
      table.moveUp();
      return true;
    }
    if (e == Event::ArrowDown || e == Event::Character('j')) {
      table.moveDown();
      return true;
    }
    if (e == Event::Character(' ')) {
      int sel = table.selectedIndex();
      if (sel >= 0) table.toggleChecked(sel);
      return true;
    }
    if (e == Event::Return) {
      auto indices = table.checkedIndices();
      if (!indices.empty()) {
        std::vector<std::pair<size_t, uint32_t>> toDelete;
        auto rows = table.rows();
        for (size_t idx : indices) {
          if (idx < rows.size()) toDelete.emplace_back(idx, rows[idx].id);
        }
        std::sort(toDelete.begin(), toDelete.end(),
                  [](auto& a, auto& b) { return a.second > b.second; });
        for (auto& p : toDelete) {
          memory.deleteExpense(p.second);
        }
        table.setRows(memory.viewAllExpenses());
        table.clearChecked();
      }
      return true;
    }
    return false;
  });

  // ----- Add modal -----
  MenuOption addCatOpt;
  addCatOpt.on_change = [&] {
    if (!categories.empty() && addCategorySelected >= 0 &&
        addCategorySelected < (int)categories.size()) {
      const auto& subs = getSubcategories(categories[addCategorySelected]);
      addSubs = subs;
      addSubSelected = 0;
    } else {
      addSubs.clear();
      addSubSelected = 0;
    }
  };
  auto addCategoryMenu = Menu(&categories, &addCategorySelected, addCatOpt);
  auto addSubMenu = Menu(&addSubs, &addSubSelected);

  auto addAmountInput = Input(&addAmountStr, "x.xx");
  auto addDateInput = Input(&addDateStr, "YYYY-MM-DD");

  addAmountInput |= CatchEvent([&](Event e) {
    if (!e.is_character()) return false;
    char c = e.character()[0];
    if (c != '.' && !std::isdigit(static_cast<unsigned char>(c))) return true;
    if (!IsValidAmountInsertion(addAmountStr, addAmountStr.size(), c)) return true;
    if (addAmountStr.size() >= 32) return true;
    return false;
  });
  addDateInput |= CatchEvent([&](Event e) {
    if (e.is_character() && addDateStr.size() >= 10) return true;
    return false;
  });

  auto addSubmit = Button("Submit", [&] {
    addHint.clear();
    if (addAmountStr.empty()) {
      addHint = "Amount required";
      return;
    }
    double amt = 0;
    try {
      size_t pos = 0;
      amt = std::stod(addAmountStr, &pos);
      if (pos != addAmountStr.size()) throw std::invalid_argument("trailing");
    } catch (...) {
      addHint = "Invalid amount";
      return;
    }
    auto dt = std::chrono::system_clock::now();
    if (!addDateStr.empty()) {
      auto parsed = parseYYYYMMDD(addDateStr);
      if (!parsed) {
        addHint = "Invalid date YYYY-MM-DD";
        return;
      }
      dt = *parsed;
    }
    std::string cat;
    if (!categories.empty() && addCategorySelected >= 0 &&
        addCategorySelected < (int)categories.size()) {
      cat = categories[addCategorySelected];
    }
    std::string sub;
    if (!addSubs.empty() && addSubSelected >= 0 && addSubSelected < (int)addSubs.size()) {
      sub = addSubs[addSubSelected];
    }
    ExpenseRecord rec;
    rec.id = 0;
    rec.amount = amt;
    rec.category = cat;
    rec.sub_category = sub;
    rec.datetime = dt;
    memory.addExpense(rec);
    table.setRows(memory.viewAllExpenses());
    table.clearChecked();
    if (!table.rows().empty()) table.setSelectedIndex(0);
    addAmountStr.clear();
    addDateStr.clear();
    addHint.clear();
    showAdd = false;
  });
  auto addCancel = Button("Cancel", [&] {
    showAdd = false;
    addHint.clear();
  });

  auto getFromInput = Input(&getFromStr, "YYYY-MM-DD");
  auto getToInput = Input(&getToStr, "YYYY-MM-DD");
  getFromInput |= CatchEvent([&](Event e) {
    if (e.is_character() && getFromStr.size() >= 10) return true;
    return false;
  });
  getToInput |= CatchEvent([&](Event e) {
    if (e.is_character() && getToStr.size() >= 10) return true;
    return false;
  });

  auto getSubmit = Button("Submit", [&] {
    getHint.clear();
    if (getFromStr.empty() || getToStr.empty()) {
      getHint = "Both dates required YYYY-MM-DD";
      return;
    }
    auto pf = parseYYYYMMDD(getFromStr);
    auto pt = parseYYYYMMDD(getToStr);
    if (!pf) {
      getHint = "Invalid date_from YYYY-MM-DD";
      return;
    }
    if (!pt) {
      getHint = "Invalid date_to YYYY-MM-DD";
      return;
    }
    if (*pf > *pt) {
      getHint = "date_from must be <= date_to";
      return;
    }
    auto filtered = memory.getExpensesByDateTime(*pf, *pt);
    table.setRows(filtered);
    table.clearChecked();
    if (!filtered.empty()) table.setSelectedIndex(0);
    getHint.clear();
    showGet = false;
  });
  auto getCancel = Button("Cancel", [&] {
    showGet = false;
    getHint.clear();
  });

  // Containers
  auto filterContainer = Container::Vertical({
      filterFromInput,
      filterToInput,
      importInput,
      filterCategoryMenu,
      filterSubMenu,
  });

  auto addModalContainer = Container::Vertical({
      addCategoryMenu,
      addSubMenu,
      addAmountInput,
      addDateInput,
      addSubmit,
      addCancel,
  });

  auto getModalContainer = Container::Vertical({
      getFromInput,
      getToInput,
      getSubmit,
      getCancel,
  });

  // Main focus container: Menu, filter, table
  auto mainContainer = Container::Vertical({
      mainMenu,
      filterContainer,
      tableComponent,
  });

  // Renderer for main UI
  auto mainRenderer = Renderer(mainContainer, [&] {
    Element title = vbox({
                            text("===========Expense Tracker=========") | bold | hcenter,
                            text("Welcome to expense tracker") | hcenter,
                        }) |
                    border;

    Element menuPane = vbox({
                            text("Menu") | bold | hcenter,
                            mainMenu->Render(),
                        }) |
                       border | size(WIDTH, EQUAL, 20);

    Element filterPane = vbox({
                              hbox({text("Filter from: "), filterFromInput->Render() | flex}),
                              hbox({text(" to: "), filterToInput->Render() | flex}),
                              hbox({text("import: "), importInput->Render() | flex}),
                              hbox({text("Category: "), filterCategoryMenu->Render() | flex}),
                              hbox({text("Subcat: "), filterSubMenu->Render() | flex}),
                          }) |
                         border | flex;

    Element expensesPane = vbox({
                               text("Expenses") | bold | hcenter,
                               text("Item: " + std::to_string(memory.getExpenseCount())),
                               text("Total: $ " + Table::formatAmount(memory.getTotalAmount())),
                           }) |
                           border | size(WIDTH, EQUAL, 20);

    Element middle = hbox({menuPane, filterPane, expensesPane}) | size(HEIGHT, EQUAL, 8);

    Element tableElem = tableComponent->Render() | border | flex;

    Element root = vbox({title, middle, tableElem}) | flex;

    // Modal overlay via dbox
    if (showAdd) {
      Element modalContent = vbox({
                                     text("Add Expense") | bold | hcenter,
                                     separator(),
                                     hbox({text("Category: "), addCategoryMenu->Render() | flex}),
                                     hbox({text("Subcategory: "), addSubMenu->Render() | flex}),
                                     hbox({text("Total: "), addAmountInput->Render() | flex}),
                                     hbox({text("Date: "), addDateInput->Render() | flex}),
                                     hbox({addSubmit->Render() | flex, addCancel->Render() | flex}) | hcenter,
                                     text(addHint.empty() ? "Tab navigate  Enter select  Esc/q close" : addHint) |
                                         hcenter |
                                         (addHint.empty() ? dim : color(Color::Red)),
                                 }) |
                             border | size(WIDTH, GREATER_THAN, 58) | size(HEIGHT, GREATER_THAN, 14);
      Element modal = window(text("Add Expense"), modalContent) | center;
      root = dbox({root, modal | clear_under | center});
    } else if (showGet) {
      Element modalContent = vbox({
                                     text("Get Expenses") | bold | hcenter,
                                     separator(),
                                     hbox({text("Date from: "), getFromInput->Render() | flex}),
                                     hbox({text("Date to: "), getToInput->Render() | flex}),
                                     hbox({getSubmit->Render() | flex, getCancel->Render() | flex}) | hcenter,
                                     text(getHint.empty() ? "Tab navigate  Enter select  Esc cancel" : getHint) |
                                         hcenter |
                                         (getHint.empty() ? dim : color(Color::Red)),
                                 }) |
                             border | size(WIDTH, GREATER_THAN, 58) | size(HEIGHT, GREATER_THAN, 10);
      Element modal = window(text("Get Expenses"), modalContent) | center;
      root = dbox({root, modal | clear_under | center});
    }

    return root;
  });

  // Global + modal event handling
  auto top = mainRenderer | CatchEvent([&](Event e) {
    if (showAdd) {
      if (e == Event::Escape || e == Event::Character('q') || e == Event::Character('Q')) {
        showAdd = false;
        addHint.clear();
        return true;
      }
      // Forward to modal container
      return addModalContainer->OnEvent(e);
    }
    if (showGet) {
      if (e == Event::Escape || e == Event::Character('q') || e == Event::Character('Q')) {
        showGet = false;
        getHint.clear();
        return true;
      }
      return getModalContainer->OnEvent(e);
    }
    if (e == Event::Character('q') || e == Event::Character('Q')) {
      screen.ExitLoopClosure()();
      return true;
    }
    return false;
  });

  screen.Loop(top);
  return 0;
}
