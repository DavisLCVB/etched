#include <etched/etched.hpp>
#include <iostream>

void showLicense() {
  std::cout << "MIT License - Copyright (c) 2026\n";
}

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;
  static constexpr int defaultWorkers = 4;
  static constexpr double defaultTimeout = 30.0;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optString<"host">('H', "host", "Server hostname", "0.0.0.0"),
      optInt<"port">('p', "port", "Server port", some(defaultPort)),
      optInt<"workers", uint16_t>('w', "workers", "Worker threads",
                                  some(static_cast<uint16_t>(defaultWorkers))),
      optFloat<"timeout", double>('t', "timeout", "Request timeout (seconds)",
                                  some(defaultTimeout)),
      optString<"config">('c', "config", "Config file path",
                          noDefault<std::string_view>),
      optBool<"verbose">('v', "verbose", "Enable verbose logging"),
      optBool<"debug">('d', "debug", noDesc),
      optCallback<"license">('L', "license", "Show license", showLicense),
      optVersion("Server v1.0.0", 'V', "version", "Show version"),
      optHelp('h', "--help"));

  auto result = parser.parse(argc, argv);

  if (result.has_value() && result.value().shouldExit) {
    return 0;
  }

  if (!result.has_value()) {
    result.error().print();
    return 1;
  }

  std::cout << "\n=== Server Configuration ===\n";
  std::cout << "Host:    " << parser.get<"host">().value() << "\n";
  std::cout << "Port:    " << parser.get<"port">().value() << "\n";
  std::cout << "Workers: " << parser.get<"workers">().value() << "\n";
  std::cout << "Timeout: " << parser.get<"timeout">().value() << "s\n";

  if (parser.get<"config">()) {
    std::cout << "Config:  " << parser.get<"config">().value() << "\n";
  }

  std::cout << "\n=== Flags ===\n";
  std::cout << "Verbose: "
            << (parser.get<"verbose">().value_or(false) ? "ON" : "OFF") << "\n";
  std::cout << "Debug:   "
            << (parser.get<"debug">().value_or(false) ? "ON" : "OFF") << "\n";

  std::cout << "\nServer starting...\n";

  return 0;
}
