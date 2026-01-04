#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optBool<"verbose">("-v", "--verbose", "Enable verbose output"),
      optBool<"debug">("-d", "--debug", "Enable debug mode"),
      optBool<"quiet">("-q", "--quiet", "Suppress output"));

  parser.parse(argc, argv);

  auto verbose = parser.getOption<"verbose">();
  auto debug = parser.getOption<"debug">();
  auto quiet = parser.getOption<"quiet">();

  std::cout << "Flags:\n";
  std::cout << "  Verbose: " << (verbose.value.value_or(false) ? "ON" : "OFF") << "\n";
  std::cout << "  Debug: " << (debug.value.value_or(false) ? "ON" : "OFF") << "\n";
  std::cout << "  Quiet: " << (quiet.value.value_or(false) ? "ON" : "OFF") << "\n";

  return 0;
}
