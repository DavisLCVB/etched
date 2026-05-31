#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline void argumentParserBuildSymbolTableTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      optInt<"port">('p', "--port", "Port", some(8080)),
      optString<"host">('h', "--host", "Host", "localhost"));

  const char* argv[] = {"prog", "--port", "9000"};
  auto result = parser.parse(3, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Parser should succeed with --port";
  }

  auto port = parser.get<"port">().value();
  if (port != 9000) {
    std::fprintf(stderr, "Got port: %d\n", port);
    throw "Port should be 9000";
  }
}

inline void argumentParserShortOptionTest() {
  constexpr auto parser = ArgumentParser("TestApp", "TestDescription",
      optInt<"port">('p', "--port", "Port", some(8080)));

  const char* argv[] = {"prog", "-p", "3000"};
  auto mutableParser = parser;
  auto result = mutableParser.parse(3, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Parser should succeed with -p";
  }

  if (mutableParser.get<"port">().value() != 3000) {
    throw "Port should be 3000";
  }
}

inline void argumentParserMultipleOptionsTest() {
  constexpr auto parser = ArgumentParser("TestApp", "TestDescription",
      optInt<"port">('p', "--port", "Port", some(8080)),
      optString<"host">('h', "--host", "Host", "localhost"));

  const char* argv[] = {"prog", "--port", "9000", "--host", "127.0.0.1"};
  auto mutableParser = parser;
  auto result = mutableParser.parse(5, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Parser should succeed";
  }

  if (mutableParser.get<"port">().value() != 9000) {
    throw "Port should be 9000";
  }
  if (mutableParser.get<"host">().value() != "127.0.0.1") {
    throw "Host should be 127.0.0.1";
  }
}

inline void argumentParserDefaultValuesTest() {
  constexpr auto parser = ArgumentParser("TestApp", "TestDescription",
      optInt<"port">('p', "--port", "Port", some(8080)),
      optString<"host">('h', "--host", "Host", "localhost"));

  const char* argv[] = {"prog"};
  auto mutableParser = parser;
  auto result = mutableParser.parse(1, argv);

  if (!result.has_value()) {
    throw "Parser should succeed with no args";
  }

  if (mutableParser.get<"port">().value() != 8080) {
    throw "Port should be default 8080";
  }
  if (mutableParser.get<"host">().value() != "localhost") {
    throw "Host should be default localhost";
  }
}

inline void argumentParserUnknownOptionTest() {
  constexpr auto parser = ArgumentParser("TestApp", "TestDescription",
      optInt<"port">('p', "--port", "Port", some(8080)));

  const char* argv[] = {"prog", "--unknown", "value"};
  auto mutableParser = parser;
  auto result = mutableParser.parse(3, argv);

  if (result.has_value()) {
    throw "Parser should fail with unknown option";
  }
}

inline void argumentParserBoolFlagTest() {
  constexpr auto parser = ArgumentParser("TestApp", "TestDescription",
      optBool<"verbose">('v', "--verbose", "Verbose"));

  const char* argv[] = {"prog", "-v"};
  auto mutableParser = parser;
  auto result = mutableParser.parse(2, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Parser should succeed with bool flag";
  }

  if (!mutableParser.has<"verbose">() || !mutableParser.get<"verbose">().value_or(false)) {
    throw "Verbose should be true";
  }
}

inline void argumentParserMixedShortLongTest() {
  constexpr auto parser = ArgumentParser("TestApp", "TestDescription",
      optInt<"port">('p', "--port", "Port", some(8080)),
      optString<"host">('h', "--host", "Host", "localhost"),
      optBool<"verbose">('v', "--verbose", "Verbose"));

  const char* argv[] = {"prog", "-p", "3000", "--host", "example.com", "-v"};
  auto mutableParser = parser;
  auto result = mutableParser.parse(6, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Parser should succeed with mixed options";
  }

  if (mutableParser.get<"port">().value() != 3000) {
    throw "Port should be 3000";
  }
  if (mutableParser.get<"host">().value() != "example.com") {
    throw "Host should be example.com";
  }
  if (!mutableParser.get<"verbose">().value_or(false)) {
    throw "Verbose should be true";
  }
}

inline void runArgumentParserTests() {
  std::printf("  argumentParserBuildSymbolTableTest...");
  argumentParserBuildSymbolTableTest();
  std::printf(" OK\n");

  std::printf("  argumentParserShortOptionTest...");
  argumentParserShortOptionTest();
  std::printf(" OK\n");

  std::printf("  argumentParserMultipleOptionsTest...");
  argumentParserMultipleOptionsTest();
  std::printf(" OK\n");

  std::printf("  argumentParserDefaultValuesTest...");
  argumentParserDefaultValuesTest();
  std::printf(" OK\n");

  std::printf("  argumentParserUnknownOptionTest...");
  argumentParserUnknownOptionTest();
  std::printf(" OK\n");

  std::printf("  argumentParserBoolFlagTest...");
  argumentParserBoolFlagTest();
  std::printf(" OK\n");

  std::printf("  argumentParserMixedShortLongTest...");
  argumentParserMixedShortLongTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests