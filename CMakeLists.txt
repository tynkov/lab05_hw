cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)
option(CODE_COVERAGE "Enable coverage reporting" OFF)

project(lab05)

add_subdirectory(banking)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)
  file(GLOB BANKING_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp)
  add_executable(check ${BANKING_TEST_SOURCES})
  target_link_libraries(check banking gtest_main gmock_main)
  add_test(NAME check COMMAND check)
endif()

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(check PRIVATE --coverage)
  target_link_options(check PRIVATE --coverage)
endif()
