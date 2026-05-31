#include <etched/etched.hpp>
#include <iostream>

void showCredits() {
  std::cout << "Created by: DavisLCVB\n";
}

void showStats() {
  std::cout << "Statistics:\n";
  std::cout << "  - Size: 60K\n";
  std::cout << "  - Time: 0.006s\n";
}

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;
  constexpr std::string_view name = "Your name";

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optInt<"port">('p', "port", "Server port", some(defaultPort)),
      optCallback<"credits">('c', "credits", "Show credits", showCredits),
      optCallback<"stats">('s', "stats", "Show statistics", showStats),
      // You can also use lambda functions for callbacks
      optCallback<"hello">('H', "hello", "Say hello",
                           []() -> void { std::cout << "Hello, world!\n"; }),
      // You can capture variables in lambda callbacks (only if they are constexpr or known at compile time)
      optCallback<"greet">(
          'g', "greet", "Greet the user",
          [name]() -> void { std::cout << "Hello, " << name << "!\n"; }),
      // Callbacks can also signal the parser to exit immediately after execution by returning an Expected<Output, RuntimeError> with shouldExit = true
      optCallback<"terminate">('x', "exit", "Exit the application immediately",
                               []() -> Expected<Output, RuntimeError> {
                                 std::cout << "Terminating application...\n";
                                 return Output{.success = true,
                                               .shouldExit = true};
                               }),
      optHelp('h', "--help"));

  auto result = parser.parse(argc, argv);
  // If a callback signals to exit, we can check the result and exit early.
  if (result.has_value() && result.value().shouldExit) {
    return 0;
  }

  if (!result.has_value()) {
    result.error().print();
    return 1;
  }

  std::cout << "Application continues after callbacks...\n";
  return 0;
}
