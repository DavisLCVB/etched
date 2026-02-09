#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {
  using namespace etched;

  auto parser = ArgumentParser(
      "FileProcessor", "A tool to process multiple files",
      optBool<"verbose">('v', "verbose", "Enable verbose output"),
      optPositional<std::string_view>("Files to process"));

  auto result = parser.parse(argc, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  bool verbose = parser.get<"verbose">().valueOr(false);
  const auto& files = parser.get<"positional">();

  if (files.empty()) {
    std::cout << "No files provided.\n";
    return 0;
  }

  std::cout << "Processing " << files.size() << " files...\n";
  if (verbose) {
    for (size_t i = 0; i < files.size(); ++i) {
      std::cout << "  [" << i + 1 << "] " << files[i] << "\n";
    }
  }

  // You can also iterate using begin/end
  if (!verbose) {
    for (auto file : files) {
      std::cout << " - " << file << "\n";
    }
  }

  return 0;
}
