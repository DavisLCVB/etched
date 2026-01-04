#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optString<"path">("-p", "--path", "File path", "/tmp/default.txt"),
      optString<"message">("-m", "--message", "User message", "Hello World"),
      optString<"description">("-d", "--desc", "Description text"));

  parser.parse(argc, argv);

  std::cout << "Values:\n";
  std::cout << "  Path: " << parser.getOption<"path">().value.value() << "\n";
  std::cout << "  Message: " << parser.getOption<"message">().value.value() << "\n";

  if (parser.getOption<"description">().value.has_value()) {
    std::cout << "  Description: " << parser.getOption<"description">().value.value() << "\n";
  }

  return 0;
}
