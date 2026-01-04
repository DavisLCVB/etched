#include "etched-tests.hpp"
#include <etched/etched.hpp>
#include <iostream>

auto main() -> int {
  std::cout << "Etched library included successfully: VERSION "
            << etched::version << "\n";
  etched::tests::mainTests();
  std::cout << "All tests passed successfully.\n";
  return 0;
}
