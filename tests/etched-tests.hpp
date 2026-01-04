#pragma once
#include <ctime>
#ifndef ETCHED_LIB_ETCHED_TEST_HPP
#define ETCHED_LIB_ETCHED_TEST_HPP

#include "etched-parser-tests.hpp"

namespace etched::tests {
auto mainTests() -> void {
  parserTests();
}
}  // namespace etched::tests

#endif  // ETCHED_LIB_ETCHED_TEST_HPP
