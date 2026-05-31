#pragma once
#include <etched/etched.hpp>
#include <cstdio>
#include <string_view>
#include <vector>

namespace etched::tests {

inline void positionalArgumentBasicTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      optPositional<std::string_view>("Files"));

  const char* argv[] = {"prog", "file1.txt", "file2.txt", "file3.txt"};
  auto result = parser.parse(4, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Positional parsing failed";
  }

  const auto& pos = parser.get<"positional">();
  if (pos.size() != 3) throw "Positional size mismatch";
  
  std::vector<std::string_view> expected = {"file1.txt", "file2.txt", "file3.txt"};
  size_t i = 0;
  for (auto val : pos) {
      if (val != expected[i++]) throw "Positional value mismatch";
  }
}

inline void positionalIntTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      optPositional<int>("Numbers"));

  const char* argv[] = {"prog", "10", "20", "30"};
  auto result = parser.parse(4, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Positional int parsing failed";
  }

  const auto& pos = parser.get<"positional">();
  if (pos.size() != 3) throw "Positional size mismatch";
  
  int expected[] = {10, 20, 30};
  for (size_t i = 0; i < 3; ++i) {
      if (pos[i] != expected[i]) throw "Positional int value mismatch";
  }
}

inline void positionalEmptyTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      optPositional<std::string_view>("Files"));

  const char* argv[] = {"prog"};
  auto result = parser.parse(1, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Positional empty parsing failed";
  }

  const auto& pos = parser.get<"positional">();
  if (!pos.empty()) throw "Positional should be empty";
}

inline void runPositionalTests() {
  std::printf("  positionalArgumentBasicTest...");
  positionalArgumentBasicTest();
  std::printf(" OK\n");

  std::printf("  positionalIntTest...");
  positionalIntTest();
  std::printf(" OK\n");

  std::printf("  positionalEmptyTest...");
  positionalEmptyTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests