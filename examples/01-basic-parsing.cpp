#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optInt<"port">('p', "port", "Server port", some(defaultPort)),
      optString<"host">(noShort, "host", "Server host", "localhost"),
      optBool<"verbose">('v', "verbose", "Enable verbose output"));

  auto result = parser.parse(argc, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    std::cout << parser.help();
    return 1;
  }

  std::cout << "Host: " << parser.get<"host">().value() << "\n";
  std::cout << "Port: " << parser.get<"port">().value() << "\n";
  return 0;
}
