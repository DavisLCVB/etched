#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;
  static constexpr double defaultTimeout = 30.0;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optInt<"port">('p', "port", "Server port", some(defaultPort)),
      // String options return std::string_view when accessed.
      optString<"host">('H', "host", "Server host", "localhost"),
      optFloat<"timeout">('t', noLong, "Connection timeout",
                          some(defaultTimeout)),
      optBool<"verbose">('v', "verbose", "Enable verbose logging"));

  auto result = parser.parse(argc, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  std::cout << "Host:    " << parser.get<"host">().value() << "\n";
  std::cout << "Port:    " << parser.get<"port">().value() << "\n";
  std::cout << "Timeout: " << parser.get<"timeout">().value() << "s\n";
  std::cout << "Verbose: " << (parser.has<"verbose">() ? "yes" : "no") << "\n";
  return 0;
}
