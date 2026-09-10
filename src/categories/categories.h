#ifndef CATEGORIES_CATEGORIES_H
#define CATEGORIES_CATEGORIES_H

#include <string>
#include <vector>

// Re-export of category vectors owned by src/categories/*.cpp.
// No literal duplication: this header only declares extern references
// to the single source-of-truth vectors defined in expenses.cpp files.

extern std::vector<std::string> categories;
extern std::vector<std::string> auto_categories;
extern std::vector<std::string> care_categories;
extern std::vector<std::string> home_categories;
extern std::vector<std::string> living_categories;
extern std::vector<std::string> medical_categories;
extern std::vector<std::string> utilities_categories;

// Returns read-only reference to the subcategory list for `category`.
// If `category` is unknown, returns an empty static vector.
inline const std::vector<std::string>& getSubcategories(const std::string& category) {
  if (category == "auto") {
    return auto_categories;
  }
  if (category == "care") {
    return care_categories;
  }
  if (category == "home") {
    return home_categories;
  }
  if (category == "living") {
    return living_categories;
  }
  if (category == "medical") {
    return medical_categories;
  }
  if (category == "utilities") {
    return utilities_categories;
  }
  static const std::vector<std::string> empty;
  return empty;
}

#endif
