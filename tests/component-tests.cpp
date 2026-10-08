#include <cstdio>
#include <etched/etched.hpp>

#include "test-advanced.hpp"
#include "test-argument-parser.hpp"
#include "test-callbacks.hpp"
#include "test-commands.hpp"
#include "test-component-replacement.hpp"
#include "test-config-variations.hpp"
#include "test-custom-types.hpp"
#include "test-errors.hpp"
#include "test-hash.hpp"
#include "test-lexer.hpp"
#include "test-nested-commands.hpp"
#include "test-positional.hpp"
#include "test-regression.hpp"
#include "test-symbol-table.hpp"

auto main() -> int {
  try {
    std::printf("Running component tests...\n\n");

    std::printf("=== Lexer Tests ===\n");
    etched::tests::runLexerTests();

    std::printf("\n=== Hash Tests ===\n");
    etched::tests::runHashTests();

    std::printf("\n=== SymbolTable Tests ===\n");
    etched::tests::runSymbolTableTests();

    std::printf("\n=== ArgumentParser Tests ===\n");
    etched::tests::runArgumentParserTests();

    std::printf("\n=== Command Tests ===\n");
    etched::tests::runCommandTests();
    etched::tests::runNestedCommandTests();

    std::printf("\n=== Custom Type Tests ===\n");
    etched::tests::runCustomTypeTests();

    std::printf("\n=== Callback Tests ===\n");
    etched::tests::runCallbackTests();

    std::printf("\n=== Error Handling Tests ===\n");
    etched::tests::runErrorTests();

    std::printf("\n=== Advanced Tests ===\n");
    etched::tests::runAdvancedTests();

    std::printf("\n=== Positional Tests ===\n");
    etched::tests::runPositionalTests();

    std::printf("\n=== Regression Tests ===\n");
    etched::tests::runRegressionTests();

    std::printf("\n=== Component Replacement Tests ===\n");
    etched::tests::runComponentReplacementTests();

    std::printf("\n=== Config Variation Tests ===\n");
    etched::tests::runConfigVariationTests();

    std::printf("\nAll component tests passed!\n");
    return 0;
  } catch (const char* msg) {
    std::fprintf(stderr, "\nTEST FAILED: %s\n", msg);
    return 1;
  }
}
