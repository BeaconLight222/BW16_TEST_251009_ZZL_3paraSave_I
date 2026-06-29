#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace test_harness {

inline int& failure_count() {
  static int count = 0;
  return count;
}

inline int& pass_count() {
  static int count = 0;
  return count;
}

inline void describe(const char* suite_name, const std::function<void()>& body) {
  std::cout << suite_name << std::endl;
  body();
}

inline void it(const char* example_name, const std::function<void()>& body) {
  std::cout << "  it " << example_name << std::endl;
  try {
    body();
    ++pass_count();
  } catch (const std::exception& ex) {
    ++failure_count();
    std::cerr << "    FAILED: " << ex.what() << std::endl;
  } catch (...) {
    ++failure_count();
    std::cerr << "    FAILED: unknown exception" << std::endl;
  }
}

template <typename T>
void expect_eq(const T& actual, const T& expected, const char* expression) {
  if (!(actual == expected)) {
    std::ostringstream message;
    message << expression << " (expected " << expected << ", got " << actual << ")";
    throw std::runtime_error(message.str());
  }
}

template <typename T>
void expect_true(T value, const char* expression) {
  if (!value) {
    std::ostringstream message;
    message << expression << " expected true";
    throw std::runtime_error(message.str());
  }
}

template <typename T>
void expect_false(T value, const char* expression) {
  if (value) {
    std::ostringstream message;
    message << expression << " expected false";
    throw std::runtime_error(message.str());
  }
}

inline int run_all_tests() {
  const int failures = failure_count();
  const int passes = pass_count();
  std::cout << "\n" << passes << " passed, " << failures << " failed" << std::endl;
  return failures == 0 ? 0 : 1;
}

}  // namespace test_harness

using test_harness::describe;
using test_harness::it;

#define expect_eq(actual, expected) test_harness::expect_eq((actual), (expected), #actual)
#define expect_true(value) test_harness::expect_true((value), #value)
#define expect_false(value) test_harness::expect_false((value), #value)
