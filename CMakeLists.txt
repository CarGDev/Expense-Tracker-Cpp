cmake_minimum_required(VERSION 3.15)
project(expenses CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG v7.0.3
)
FetchContent_MakeAvailable(ftxui)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(expenses ${SOURCES})
target_link_libraries(expenses PRIVATE ftxui::screen ftxui::dom ftxui::component)
# Alias target ftxui::ftxui is provided by newer versions; link above covers all.
# For compatibility with spec's ftxui::ftxui single target:
if(TARGET ftxui::ftxui)
  target_link_libraries(expenses PRIVATE ftxui::ftxui)
endif()
