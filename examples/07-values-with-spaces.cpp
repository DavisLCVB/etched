#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  auto parser =
      ArgumentParser("ExampleApp", "ExampleDescription",
                     optString<"msg">('m', "message", "Message with spaces",
                                      "No message provided"));

  auto result = parser.parse(argc, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  std::cout << "Message: \"" << parser.get<"msg">().value() << "\"\n";
  return 0;
}
