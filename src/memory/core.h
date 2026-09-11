#ifndef EXPENSE_MEMORY_CORE_H
#define EXPENSE_MEMORY_CORE_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct ExpenseRecord {
  uint32_t id = 0;
  double amount = 0;
  std::string category;
  std::string sub_category;
  std::chrono::system_clock::time_point datetime{};
};

struct ExpenseFilterCriteria {
  std::optional<std::chrono::system_clock::time_point> date_from;
  std::optional<std::chrono::system_clock::time_point> date_to;
  std::optional<std::string> category;
  std::optional<std::string> sub_category;
};

class ExpenseMemory {
public:
  void addExpense(const ExpenseRecord &expense);
  void deleteExpense(std::size_t idx);
  std::vector<ExpenseRecord> viewAllExpenses() const;
  ExpenseRecord viewExpense(std::size_t idx) const;
  std::vector<ExpenseRecord>
  filterExpenses(const ExpenseFilterCriteria &criteria) const;
  std::vector<ExpenseRecord> getExpensesByDateTime(
      std::chrono::system_clock::time_point datetime_from,
      std::chrono::system_clock::time_point datetime_to) const;
  std::size_t getExpenseCount() const;
  std::size_t getExpenseCount(const std::vector<ExpenseRecord> &records) const;
  double getTotalAmount() const;
  double getTotalAmount(const std::vector<ExpenseRecord> &records) const;
  void clear();
  void reset();
  void release();
};

// Parse YYYY-MM-DD → time_point at local midnight. Returns nullopt on invalid.
std::optional<std::chrono::system_clock::time_point>
parseYYYYMMDD(const std::string &s);

#endif
