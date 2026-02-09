#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;

  auto parser = ArgumentParser(
      "MyApp", "ExampleDescription",
      optInt<"port">('p', "port", "Server port", some(defaultPort)),
      // Version option, you can customize the version string and the option flags
      optVersion("MyApp v2.5.1", 'v', "version", "Show version"),
      // Help options doesn't include the version information.
      optHelp('h', "--help"));

  auto result = parser.parse(argc, argv);
  if (result.isOk() && result.unwrap().shouldExit) {
    return 0;
  }

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  std::cout << "Application running...\n";
  return 0;
}
