#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {
  using namespace etched;

  auto parser =
      ArgumentParser("MyApp", "A simple example",
                     optString<"name">('n', "name", "Your name", "Guest"),
                     optBool<"v">('v', "verbose", "Verbose mode"));

  if (auto result = parser.parse(argc, argv); !result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  if (parser.get<"v">()) {
    std::cout << "Verbose mode ON\n";
  }
  std::cout << "Hello, " << parser.get<"name">().value() << "!\n";
  return 0;
}
