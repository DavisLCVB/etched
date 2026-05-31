#pragma once
#include <cstdio>
#include <etched/etched.hpp>
#include <string_view>

namespace etched::tests {

// 1. Test LexerConfig::allowClusters = false
struct NoClustersConfig : public etched::DefaultConfig {
  struct LexerConfig {
    static constexpr bool allowClusters = false;
    static constexpr bool prefferValues = false;
  };

  using Lexer = etched::detail::DefaultLexer<LexerConfig>;
  template <typename... Args>
  using JumpTable = etched::detail::DefaultJumpTable<Lexer, Args...>;
};

inline void testNoClusters() {
  using namespace etched;
  auto parser =
      ArgumentParser(WithConfig<NoClustersConfig>{},
          "TestApp", "TestDescription",
          optBool<"flag1">('a', "flag1", "Flag 1"),
          optBool<"flag2">('b', "flag2", "Flag 2"));

  const char* argv[] = {"prog", "-ab"};
  auto result = parser.parse(2, argv);
  if (result.has_value()) {
    throw "Should have failed for clustered options";
  }
  // Optional: check error message
  // if (result.error().message() != std::string_view("Clustered short options are not allowed")) ...
}

// 2. Test LexerConfig::prefferValues = true
struct PrefferValuesConfig : public etched::DefaultConfig {
  struct LexerConfig {
    static constexpr bool allowClusters = true;
    static constexpr bool prefferValues = true;
  };

  using Lexer = etched::detail::DefaultLexer<LexerConfig>;
  template <typename... Args>
  using JumpTable = etched::detail::DefaultJumpTable<Lexer, Args...>;
};

inline void testPrefferValues() {
  using namespace etched;
  auto parser = ArgumentParser(WithConfig<PrefferValuesConfig>{},
      "TestApp", "TestDescription", optString<"val">('v', "value", "Value"));

  // In prefferValues = true, if we are waiting for a value, "-x" should be
  // treated as a positional/value, not a short option.
  const char* argv[] = {"prog", "-v", "-x"};
  auto result = parser.parse(3, argv);

  if (!result.has_value())
    throw "PrefferValues failed";
  auto val = parser.get<"val">();
  if (!val.has_value() || val.value() != "-x")
    throw "Value should have been '-x'";
}

// 3. Test Custom SymbolTableConfig
struct TinySTConfig : public etched::DefaultSTConfig {
  static constexpr size_t maxStringSize = 4;  // Very small
};

struct TinySTAppConfig : public etched::DefaultConfig {
  template <size_t N>
  using SymbolTable = etched::detail::DefaultSymbolTable<N, TinySTConfig>;
};

inline void testTinySymbolTable() {
  using namespace etched;
  // If maxStringSize is 4, "longname" should be truncated to "lon" (3 chars +
  // null)
  auto parser =
      ArgumentParser(WithConfig<TinySTAppConfig>{},
          "TestApp", "TestDescription",
          optBool<"longname">('l', "longname", "Long name"));

  const char* argv[] = {"prog", "--longname"};
  auto result = parser.parse(2, argv);
  // It should fail because "longname" was truncated to "lon" in the symbol table
  if (result.has_value())
    throw "Should have failed to find --longname due to truncation";
}

// 4. Test HelpConfig::colWidth
struct WideHelpConfig : public etched::DefaultHelpConfig {
  static constexpr size_t colWidth = 40;
};

struct WideHelpAppConfig : public etched::DefaultConfig {
  using helpConfig = WideHelpConfig;
};

inline void testHelpColWidth() {
  using namespace etched;
  auto parser =
      ArgumentParser(WithConfig<WideHelpAppConfig>{},
          "TestApp", "TestDescription",
          optBool<"f">('f', "flag", "Flag description"));

  auto help = parser.help();
  std::string_view help_sv = help;

  size_t flagPos = help_sv.find("--flag");
  if (flagPos == std::string_view::npos)
    throw "--flag not found in help";

  size_t descPos = help_sv.find("Flag description");
  if (descPos == std::string_view::npos)
    throw "description not found";

  size_t distance = descPos - (flagPos + 6);  // 6 is length of "--flag"

  if (distance < 30)
    throw "Help column width not respected";
}

inline void runConfigVariationTests() {
  std::printf("  testNoClusters...");
  testNoClusters();
  std::printf(" OK\n");

  std::printf("  testPrefferValues...");
  testPrefferValues();
  std::printf(" OK\n");

  std::printf("  testHelpColWidth...");
  testHelpColWidth();
  std::printf(" OK\n");

  std::printf("  testTinySymbolTable...");
  testTinySymbolTable();
  std::printf(" OK\n");
}

}  // namespace etched::tests