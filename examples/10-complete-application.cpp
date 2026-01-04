#include <etched/etched.hpp>
#include <iostream>

void showLicense() {
  std::cout << "MIT License - Copyright (c) 2024\n";
}

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optString<"host">("-H", "--host", "Server hostname", "0.0.0.0"),
      optInt<"port">("-p", "--port", "Server port", 8080),
      optInt<"workers", uint16_t>("-w", "--workers", "Worker threads", 4),
      optFloat<"timeout", double>("-t", "--timeout", "Request timeout (seconds)", 30.0),
      optString<"config">("-c", "--config", "Config file path"),
      optBool<"verbose">("-v", "--verbose", "Enable verbose logging"),
      optBool<"debug">("-d", "--debug", "Enable debug mode"),
      optCallback<"license">("-L", "--license", "Show license", showLicense),
      optVersion("Server v1.0.0", "-V", "--version", "Show version"),
      optHelp("-h", "--help"));

  try {
    parser.parse(argc, argv);

    std::cout << "\n=== Server Configuration ===\n";
    std::cout << "Host:    " << parser.getOption<"host">().value.value() << "\n";
    std::cout << "Port:    " << parser.getOption<"port">().value.value() << "\n";
    std::cout << "Workers: " << parser.getOption<"workers">().value.value() << "\n";
    std::cout << "Timeout: " << parser.getOption<"timeout">().value.value() << "s\n";

    if (parser.getOption<"config">().value.has_value()) {
      std::cout << "Config:  " << parser.getOption<"config">().value.value() << "\n";
    }

    std::cout << "\n=== Flags ===\n";
    std::cout << "Verbose: " << (parser.getOption<"verbose">().value.value_or(false) ? "ON" : "OFF") << "\n";
    std::cout << "Debug:   " << (parser.getOption<"debug">().value.value_or(false) ? "ON" : "OFF") << "\n";

    std::cout << "\nServer starting...\n";

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
