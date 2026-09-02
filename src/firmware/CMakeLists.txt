cmake_minimum_required(VERSION 3.16)
project(holobox_sensing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Run clang-tidy during the build" OFF)
option(ENABLE_COVERAGE "Instrument the build for test coverage (gcc/gcov)" OFF)
option(BUILD_TESTING "Build unit tests" ON)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)

if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
  endif()
endif()

if(ENABLE_COVERAGE)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

add_library(sensing_core src/example.cpp)
target_include_directories(sensing_core PUBLIC include)

add_executable(sensing_app src/main.cpp)
target_link_libraries(sensing_app PRIVATE sensing_core)

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
