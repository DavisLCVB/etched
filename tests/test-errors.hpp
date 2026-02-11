#pragma once
#include <etched/etched.hpp>
#include <cstdio>
#include <string_view>

namespace etched::tests {

struct SmallConfig : DefaultConfig {
  static constexpr size_t maxArgs = 2;
};

inline void errorMissingValueTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optInt<"port">('p', "--port", "Port"));

  const char* argv[] = {"prog", "--port"};
  auto result = parser.parse(2, argv);

  if (result.isOk()) {
    throw "Should fail when value is missing for long option";
  }
}

inline void errorInvalidIntTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optInt<"port">('p', "--port", "Port"));

  const char* argv[] = {"prog", "-p", "abc"};
  auto result = parser.parse(3, argv);

  if (result.isOk()) {
    throw "Should fail when value is not a valid integer";
  }
}

inline void errorUnknownOptionTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optBool<"verbose">('v', "--verbose", "Verbose"));

  const char* argv[] = {"prog", "--unknown"};
  auto result = parser.parse(2, argv);

  if (result.isOk()) {
    throw "Should fail when an unknown option is provided";
  }
}

inline void errorExceedMaxArgsTest() {
  auto parser = ArgumentParser(WithConfig<SmallConfig>{},
      "TestApp", "TestDescription", optBool<"v">('v', "--verbose", "Verbose"));

  const char* argv[] = {"prog", "-v", "-v", "-v"};
  auto result = parser.parse(4, argv);

  if (result.isOk()) {
    throw "Should fail when exceeding maxArgs";
  }
}

inline void errorMissingSubcommandTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               cmd<"init">("Init project"));

  const char* argv[] = {"prog"};
  auto result = parser.parse(1, argv);

  if (!result.isOk()) {
    throw "Parser should be OK even if no command is provided (default behavior)";
  }
}

inline void errorUnexpectedPositionalTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optBool<"verbose">('v', "--verbose", "Verbose"));

  const char* argv[] = {"prog", "unexpected"};
  auto result = parser.parse(2, argv);

  if (result.isOk()) {
    throw "Should fail when an unexpected positional argument is provided";
  }
}

inline void runErrorTests() {
  std::printf("  errorMissingValueTest...");
  errorMissingValueTest();
  std::printf(" OK\n");

  std::printf("  errorInvalidIntTest...");
  errorInvalidIntTest();
  std::printf(" OK\n");

  std::printf("  errorUnknownOptionTest...");
  errorUnknownOptionTest();
  std::printf(" OK\n");

  std::printf("  errorExceedMaxArgsTest...");
  errorExceedMaxArgsTest();
  std::printf(" OK\n");

  std::printf("  errorMissingSubcommandTest...");
  errorMissingSubcommandTest();
  std::printf(" OK\n");

  std::printf("  errorUnexpectedPositionalTest...");
  errorUnexpectedPositionalTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests
