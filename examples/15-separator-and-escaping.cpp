#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {
  using namespace etched;

  auto parser = ArgumentParser("Escaper", "Example of using -- separator",
                               optBool<"all">('a', "all", "Select all"),
                               optPositional<std::string_view>("Items"),
                               optHelp('h', "help"));

  auto result = parser.parse(argc, argv);

  if (!result.has_value()) {
    result.error().print();
    return 1;
  }

  if (parser.get<"all">().value_or(false)) {
    std::cout << "Option --all is active\n";
  }

  const auto& items = parser.get<"positional">();
  std::cout << "Items found (" << items.size() << "):\n";
  for (auto item : items) {
    std::cout << "  - " << item << "\n";
  }

  if (items.empty()) {
    std::cout << "\nTry running: " << argv[0] << " -a -- -a --not-an-option\n";
    std::cout << "Everything after '--' is a positional argument.\n";
  }

  return 0;
}
