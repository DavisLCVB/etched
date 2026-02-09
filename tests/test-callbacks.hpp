#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline int callbackCounter = 0;

inline void testCallback() {
  callbackCounter++;
}

inline auto testCallbackWithResult() -> Result<Output> {
  callbackCounter += 10;
  return ok(Output{.success = true, .shouldExit = false});
}

inline void callbackBasicTest() {
  callbackCounter = 0;

  auto parser = ArgumentParser("TestApp", "TestDescription",
      optCallback<"action">('a', "--action", "Action", testCallback),
      optInt<"value">('v', "--value", "Value", some(0)));

  const char* argv[] = {"prog", "-a", "-v", "42"};
  auto result = parser.parse(4, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Callback parsing should succeed";
  }

  if (callbackCounter != 1) {
    std::fprintf(stderr, "callbackCounter = %d\n", callbackCounter);
    throw "Callback should have been called once";
  }

  if (parser.get<"value">().value() != 42) {
    throw "Value should be 42";
  }
}

inline void callbackMultipleTest() {
  callbackCounter = 0;

  auto parser = ArgumentParser("TestApp", "TestDescription",
      optCallback<"inc">('i', "--increment", "Increment", testCallback),
      optCallback<"add">('a', "--add", "Add 10", testCallbackWithResult));

  const char* argv[] = {"prog", "-i", "-a", "-i"};
  auto result = parser.parse(4, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Multiple callbacks should succeed";
  }

  if (callbackCounter != 12) {
    std::fprintf(stderr, "callbackCounter = %d (expected 12)\n", callbackCounter);
    throw "Callbacks should total 12 (1 + 10 + 1)";
  }
}

inline auto testCallbackExit() -> Result<Output> {
  return ok(Output{.success = true, .shouldExit = true});
}

inline void callbackExitTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      optCallback<"exit">('e', "--exit", "Exit", testCallbackExit));

  const char* argv[] = {"prog", "-e"};
  auto result = parser.parse(2, argv);

  if (!result.isOk()) {
    throw "Exit callback parsing should succeed";
  }

  if (!result.unwrap().shouldExit) {
    throw "shouldExit should be true";
  }
}

inline void runCallbackTests() {
  std::printf("  callbackBasicTest...");
  callbackBasicTest();
  std::printf(" OK\n");

  std::printf("  callbackMultipleTest...");
  callbackMultipleTest();
  std::printf(" OK\n");

  std::printf("  callbackExitTest...");
  callbackExitTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests