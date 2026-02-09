#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      optBool<"verbose">('v', "verbose", "Enable verbose logging"),
      optBool<"debug">('d', "debug", noDesc));

  auto result = parser.parse(argc, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  if (parser.get<"verbose">().valueOr(false)) {
    std::cout << "Verbose mode enabled\n";
  }

  // You can also check for the presence of a flag using has<"tag">()
  if (parser.has<"verbose">()) {
    std::cout << "Verbose flag is present\n";
  }

  if (parser.get<"debug">().valueOr(false)) {
    std::cout << "Debug mode enabled\n";
  }

  // You can also check for the presence of a flag using has<"tag">()
  if (parser.has<"debug">()) {
    std::cout << "Debug flag is present\n";
  }
  return 0;
}
