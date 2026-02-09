#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline void optionValueNotPositionalTest() {
  // Regression: Ensure that an option value that looks like a positional
  // is correctly consumed as the option's value.
  auto parser = ArgumentParser("TestApp", "TestDescription",
      optString<"output">('o', "--output", "Output file", "default.txt"),
      optPositional<std::string_view>("Input files"));

  // "out.txt" should be consumed by -o, "in.txt" should be positional
  const char* argv[] = {"prog", "-o", "out.txt", "in.txt"};
  auto result = parser.parse(4, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Regression test failed";
  }

  if (parser.get<"output">().value() != "out.txt") {
    throw "Option value not correctly consumed";
  }

  auto positionals = parser.get<"positional">();
  if (positionals.size() != 1 || positionals[0] != "in.txt") {
    throw "Positional argument not correctly parsed";
  }
}

inline void shortOptionJoinedValueTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optInt<"port">('p', "--port", "Port", some(8080)));

  const char* argv[] = {"prog", "-p9000"};
  auto result = parser.parse(2, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Short option with joined value failed to parse";
  }

  if (parser.get<"port">().value() != 9000) {
    std::fprintf(stderr, "Got port: %d\n", parser.get<"port">().value());
    throw "Port should be 9000";
  }
}

inline void runRegressionTests() {
  std::printf("  optionValueNotPositionalTest...");
  optionValueNotPositionalTest();
  std::printf(" OK\n");

  std::printf("  shortOptionJoinedValueTest...");
  shortOptionJoinedValueTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests
