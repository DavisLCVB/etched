#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline void commandBasicTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      cmd<"init">("Initialize project",
          optString<"name">('n', "--name", "Project name", "myproject")),
      cmd<"build">("Build project",
          optBool<"release">('r', "--release", "Release build")));

  const char* argv[] = {"prog", "init", "--name", "testproject"};
  auto result = parser.parse(4, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Command parsing should succeed";
  }

  if (!parser.has<"init">()) {
    throw "init command should be matched";
  }

  const auto& initCmd = parser.get<"init">();
  if (initCmd.get<"name">().value() != "testproject") {
    throw "Command option should be 'testproject'";
  }
}

inline void commandWithSubOptionsTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      cmd<"serve">("Start server",
          optInt<"port">('p', "--port", "Port", some(8080)),
          optString<"host">('h', "--host", "Host", "localhost")));

  const char* argv[] = {"prog", "serve", "-p", "3000", "--host", "0.0.0.0"};
  auto result = parser.parse(6, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Command with options should succeed";
  }

  if (!parser.has<"serve">()) {
    throw "serve command should be matched";
  }

  const auto& serveCmd = parser.get<"serve">();
  if (serveCmd.get<"port">().value() != 3000) {
    throw "Port should be 3000";
  }
  if (serveCmd.get<"host">().value() != "0.0.0.0") {
    throw "Host should be 0.0.0.0";
  }
}

inline void commandDefaultValuesTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      cmd<"run">("Run application",
          optInt<"workers">('w', "--workers", "Workers", some(4))));

  const char* argv[] = {"prog", "run"};
  auto result = parser.parse(2, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Command with defaults should succeed";
  }

  if (!parser.has<"run">()) {
    throw "run command should be matched";
  }

  const auto& runCmd = parser.get<"run">();
  if (runCmd.get<"workers">().value() != 4) {
    throw "Workers should be default 4";
  }
}

inline void commandHelpTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      cmd<"init">("Initialize project",
          optString<"name">('n', "--name", "Project name", "myproject"),
          optHelp('h', "--help")));

  const char* argv[] = {"prog", "init", "--help"};
  
  auto result = parser.parse(3, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Command help parsing should succeed";
  }

  if (!result.value().shouldExit) {
    throw "Command help should signal exit";
  }

  if (!parser.has<"init">()) {
    throw "init command should still be matched even if help was requested";
  }
}

inline void runCommandTests() {
  std::printf("  commandBasicTest...");
  commandBasicTest();
  std::printf(" OK\n");

  std::printf("  commandWithSubOptionsTest...");
  commandWithSubOptionsTest();
  std::printf(" OK\n");

  std::printf("  commandDefaultValuesTest...");
  commandDefaultValuesTest();
  std::printf(" OK\n");

  std::printf("  commandHelpTest...");
  commandHelpTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests