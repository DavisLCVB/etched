#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optInt<"port">('p', "port", "Server port", some(defaultPort)),
      optString<"config">('c', "config", "Config file path"),
      // You can put long names with or without dashes (Will be normalized to long names without dashes internally)
      optBool<"verbose">('v', "--verbose", "Verbose output"),
      // Yoy can define custom flags that trigger the help message
      optHelp('h', "myHelp"));

  auto result = parser.parse(argc, argv);
  if (result.has_value() && result.value().shouldExit) {
    return 0;
  }

  if (!result.has_value()) {
    result.error().print();
    // Help always is generated. optHelp is only for assing a flag to trigger it.
    std::cout << parser.help() << "\n";
    return 1;
  }

  std::cout << "Program started successfully!\n";
  return 0;
}
