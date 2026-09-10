#ifndef CATEGORIES_EXPENSE_RECORDER_H
#define CATEGORIES_EXPENSE_RECORDER_H

#include <string>

#include "../memory/core.h"

class CategoryExpenseRecorder {
public:
  void recordExpense(const std::string& category, const ExpenseRecord& expense);
};

#endif
