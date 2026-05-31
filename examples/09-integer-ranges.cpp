#include <cstdint>
#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr uint16_t defaultPort = 8080;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      // Etched provides variants of optInt that allow you to specify
      // the size of the integer.
      optInt<"port", uint16_t>('p', "port", "Server port", some(defaultPort)));

  auto result = parser.parse(argc, argv);

  if (!result.has_value()) {
    auto error = result.error();
    // The error message will indicate if the provided value is out of range.
    std::cerr << "Error: " << error.message() << "\n";
    return 1;
  }
  if (parser.has<"port">()) {
    std::cout << "Port provided: " << parser.get<"port">().value() << "\n";
    std::cout << "Port is within range\n";
  } else {
    std::cout << "Port not provided, using default: " << defaultPort << "\n";
  }

  return 0;
}
