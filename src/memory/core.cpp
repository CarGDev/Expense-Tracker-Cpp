#include "core.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

namespace {
std::vector<ExpenseRecord> expense_store;

std::size_t get_size_of_expenses_records() { return expense_store.size(); }

void freeMemory() { expense_store.clear(); }
} // namespace

void ExpenseMemory::addExpense(const ExpenseRecord &expense) {
  ExpenseRecord rec = expense;
  if (rec.id == 0) {
    rec.id = static_cast<uint32_t>(get_size_of_expenses_records() + 1);
  }
  expense_store.push_back(rec);
}

void ExpenseMemory::deleteExpense(std::size_t idx) {
  auto it = std::find_if(expense_store.begin(), expense_store.end(),
                         [idx](const ExpenseRecord &e) {
                           return e.id == static_cast<uint32_t>(idx);
                         });
  if (it != expense_store.end()) {
    expense_store.erase(it);
    return;
  }
  if (idx < expense_store.size()) {
    expense_store.erase(expense_store.begin() +
                        static_cast<std::ptrdiff_t>(idx));
  }
}

std::vector<ExpenseRecord> ExpenseMemory::viewAllExpenses() const {
  return expense_store;
}

ExpenseRecord ExpenseMemory::viewExpense(std::size_t idx) const {
  if (expense_store.empty()) {
    return {};
  }
  for (const auto &rec : expense_store) {
    if (rec.id == static_cast<uint32_t>(idx)) {
      return rec;
    }
  }
  if (idx < expense_store.size()) {
    return expense_store[idx];
  }
  return {};
}

std::vector<ExpenseRecord> ExpenseMemory::getExpensesByDateTime(
    std::chrono::system_clock::time_point datetime_from,
    std::chrono::system_clock::time_point datetime_to) const {
  std::vector<ExpenseRecord> all_expenses;
  all_expenses.reserve(expense_store.size());
  for (const auto &rec : expense_store) {
    if (rec.datetime >= datetime_from && rec.datetime <= datetime_to) {
      all_expenses.push_back(rec);
    }
  }
  return all_expenses;
}

std::size_t ExpenseMemory::getExpenseCount() const {
  return expense_store.size();
}

double ExpenseMemory::getTotalAmount() const {
  double total = 0.0;
  for (const auto &rec : expense_store) {
    total += rec.amount;
  }
  return total;
}

std::optional<std::chrono::system_clock::time_point> parseYYYYMMDD(
    const std::string &s) {
  if (s.size() != 10) return std::nullopt;
  if (s[4] != '-' || s[7] != '-') return std::nullopt;
  int y = 0, m = 0, d = 0;
  try {
    y = std::stoi(s.substr(0, 4));
    m = std::stoi(s.substr(5, 2));
    d = std::stoi(s.substr(8, 2));
  } catch (...) {
    return std::nullopt;
  }
  if (m < 1 || m > 12) return std::nullopt;
  if (d < 1 || d > 31) return std::nullopt;
  std::tm tm{};
  tm.tm_year = y - 1900;
  tm.tm_mon = m - 1;
  tm.tm_mday = d;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  // Save original for validation after normalization
  std::tm orig = tm;
  std::time_t t = std::mktime(&tm);
  if (t == static_cast<std::time_t>(-1)) return std::nullopt;
  // mktime normalizes out-of-range dates; reject if changed
  if (tm.tm_year != orig.tm_year || tm.tm_mon != orig.tm_mon ||
      tm.tm_mday != orig.tm_mday) {
    return std::nullopt;
  }
  return std::chrono::system_clock::from_time_t(t);
}
