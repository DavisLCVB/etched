#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optInt<"port">('p', "port", "Server port", some(defaultPort)));

  auto result = parser.parse(argc, argv);

  // Custom error handling
  if (!result.isOk()) {
    std::cerr << "Custom error handling:\n";
    auto error = result.unwrapErr();
    std::cerr << "ExampleApp says: " << error.message() << "\n";
    return 1;
  }

  std::cout << "Port: " << parser.get<"port">().value() << "\n";
  return 0;
}
