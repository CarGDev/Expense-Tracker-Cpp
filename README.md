# Expense Tracker — C++ Implementation

By **Carlos Gutierrez** — Semester 4, Week 3.

> This repository implements the **C++ track** of the assignment. The course assigns the same application in two languages to compare how each handles data structures, memory management, concurrency, and error handling.

## 1. Assignment Context

**Python 1 — Project Overview:** each group designs and implements an application with specific requirements in two assigned languages, emphasizing language-specific features.

**Option 1 assigned here: Expense Tracker** — record, view, and categorize expenses; filter by date/category; calculate totals.

The brief gives language examples:

* **Python:** `dict` storage, dynamic typing, `datetime`.
* **C++:** `struct`/`class` for expenses, STL containers, explicit memory management.

This repo is the **C++ implementation** — Python counterpart: [CarGDev/Expense-Tracker-Python](https://github.com/CarGDev/Expense-Tracker-Python).

## 2. Core Requirements

* **Data storage** — expense with `date`, `amount`, `category`, `description` → `ExpenseRecord` in `src/memory/core.h:11`
* **Filter & search** — by date range (`getExpensesByDateTime`) and category/subcategory (`src/categories/categories.h:21`)
* **Summary** — total by category and overall (`getTotalAmount()`, `getExpenseCount()`)

## 3. C++ Highlights

* Strongly-typed `struct ExpenseRecord` vs Python `dict`
* STL: `std::vector`, `std::optional`, `std::chrono::system_clock::time_point` + `parseYYYYMMDD` (`src/memory/core.h:33`)
* Value semantics & RAII for ncurses windows (`src/tui/layout.*`)
* Terminal UI with `ncurses` (`src/main.cpp:1`, `src/tui/`)

## 4. Project Structure

```
src/
  main.cpp                 # ncurses app entry, layout & focus loop
  memory/core.{h,cpp}      # ExpenseRecord, ExpenseMemory
  memory/values.h          # auxiliary Expense type
  categories/              # auto/care/home/living/medical/utilities + categories.h
  tui/layout.* focus.*     # window layout, focus manager
  tui/popup_add_expense.*  # add flow
  tui/popup_get_expense.*  # filter/search flow
  tui/widgets/             # input, menu, table
Makefile                   # g++ -std=c++17 -lncurses
LICENSE                    # MIT
```

## 5. Prerequisites

* `g++` with C++17, `make`, `ncurses`
* macOS: `brew install ncurses`

## 6. Build & Run

```bash
make
./expenses
make clean
```

Binary: `expenses` at repo root (gitignored). Requires a terminal with sufficient size; small terminals show a placeholder (`src/tui/layout.cpp`).

## 7. Usage (TUI)

* `Tab` / `Shift+Tab` / `h`/`j`/`k`/`l` — navigate; `Enter` — activate
* **Menu:** `add_expense` (popup form), `get_expenses` (filter popup), `remove_expenses` (table selection)
* **Filter pane:** `Filter from/to` (YYYY-MM-DD), `import`, `Category`/`Subcategory`
* **Table:** `j`/`k` move, `Space` check, `Enter` delete checked
* `Esc` — exit edit/popup; `q` — quit

## 8. License

MIT © 2026 Carlos Gutierrez — see [LICENSE](./LICENSE).
