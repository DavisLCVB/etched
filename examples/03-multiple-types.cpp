#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"count", int32_t>("-c", "--count", "Item count", 10),
      optFloat<"rate", double>("-r", "--rate", "Rate multiplier", 1.5),
      optString<"name">("-n", "--name", "User name", "guest"),
      optInt<"size", uint64_t>("-s", "--size", "File size in bytes", 1024));

  parser.parse(argc, argv);

  std::cout << "Configuration:\n";
  std::cout << "  Count (int32): " << parser.getOption<"count">().value.value() << "\n";
  std::cout << "  Rate (double): " << parser.getOption<"rate">().value.value() << "\n";
  std::cout << "  Name (string): " << parser.getOption<"name">().value.value() << "\n";
  std::cout << "  Size (uint64): " << parser.getOption<"size">().value.value() << "\n";

  return 0;
}
