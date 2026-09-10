#include "table.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Table::Table() : selected_(-1), scrollOffset_(0) {}

void Table::setRows(const std::vector<ExpenseRecord>& rows) {
  rows_ = rows;
  checked_.assign(rows_.size(), 0);
  if (rows_.empty()) {
    selected_ = -1;
    scrollOffset_ = 0;
  } else {
    if (selected_ < 0) selected_ = 0;
    clampSelection();
    updateScroll(1000);
  }
}

const std::vector<ExpenseRecord>& Table::rows() const {
  return rows_;
}

void Table::setStubRows(size_t count) {
  std::vector<ExpenseRecord> stub;
  stub.reserve(count);
  auto now = std::chrono::system_clock::now();
  for (size_t i = 0; i < count; ++i) {
    ExpenseRecord r;
    r.id = static_cast<uint32_t>(i + 1);
    r.amount = 10.0 + static_cast<double>(i) * 1.5;
    r.category = (i % 2 == 0) ? "auto" : "home";
    r.sub_category = (i % 2 == 0) ? "Gas" : "Rent";
    r.datetime = now;
    stub.push_back(r);
  }
  setRows(stub);
}

int Table::selectedIndex() const { return selected_; }

void Table::setSelectedIndex(int idx) {
  selected_ = idx;
  clampSelection();
}

bool Table::hasSelection() const {
  return selected_ >= 0 && selected_ < static_cast<int>(rows_.size());
}

void Table::clampSelection() {
  if (rows_.empty()) {
    selected_ = -1;
    scrollOffset_ = 0;
    return;
  }
  if (selected_ < 0) selected_ = 0;
  if (selected_ >= static_cast<int>(rows_.size()))
    selected_ = static_cast<int>(rows_.size()) - 1;
}

void Table::updateScroll(int visibleRows) {
  if (visibleRows <= 0) return;
  if (selected_ < 0) {
    scrollOffset_ = 0;
    return;
  }
  if (selected_ < scrollOffset_) {
    scrollOffset_ = selected_;
  } else if (selected_ >= scrollOffset_ + visibleRows) {
    scrollOffset_ = selected_ - visibleRows + 1;
  }
  if (scrollOffset_ < 0) scrollOffset_ = 0;
  int maxOff = static_cast<int>(rows_.size()) - visibleRows;
  if (maxOff < 0) maxOff = 0;
  if (scrollOffset_ > maxOff) scrollOffset_ = maxOff;
}

void Table::moveUp() {
  if (rows_.empty()) return;
  selected_--;
  if (selected_ < 0) selected_ = static_cast<int>(rows_.size()) - 1;
}

void Table::moveDown() {
  if (rows_.empty()) return;
  selected_++;
  if (selected_ >= static_cast<int>(rows_.size())) selected_ = 0;
}

int Table::scrollOffset() const { return scrollOffset_; }

std::string Table::header() {
  return "Id | [ ] | Category | Subcategory | Total | Date";
}

void Table::toggleChecked(int idx) {
  if (idx < 0 || idx >= static_cast<int>(checked_.size())) return;
  checked_[static_cast<size_t>(idx)] = !checked_[static_cast<size_t>(idx)];
}

std::vector<size_t> Table::checkedIndices() const {
  std::vector<size_t> out;
  for (size_t i = 0; i < checked_.size(); ++i) {
    if (checked_[i]) out.push_back(i);
  }
  return out;
}

void Table::clearChecked() {
  for (auto& c : checked_) c = 0;
}

bool Table::isChecked(int idx) const {
  if (idx < 0 || idx >= static_cast<int>(checked_.size())) return false;
  return checked_[static_cast<size_t>(idx)] != 0;
}

size_t Table::checkedCount() const {
  size_t n = 0;
  for (auto c : checked_)
    if (c) ++n;
  return n;
}

std::string Table::formatAmount(double amount) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << amount;
  return oss.str();
}

std::string Table::formatDate(const std::chrono::system_clock::time_point& tp) {
  std::time_t t = std::chrono::system_clock::to_time_t(tp);
  std::tm* tm = std::localtime(&t);
  if (!tm) return "YYYY-MM-DD";
  char buf[11];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
  return std::string(buf);
}
