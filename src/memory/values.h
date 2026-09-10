#pragma once

#include <cstdint>
#include <string>

struct Expense {
  uint32_t id;
  uint32_t amount;
  std::string category;
  std::string subCategory;
  std::string date;
};
