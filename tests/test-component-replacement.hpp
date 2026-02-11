#pragma once
#include <etched/etched.hpp>
#include <cstdio>
#include <string_view>
#include <vector>

namespace etched::tests {

// 1. Trace Parser for testing replacement
inline bool parserCalled = false;

struct TraceParser {
    template <typename Lexer, typename SymbolTable, typename JumpTable, typename Tuple>
    static auto parse(Lexer&, const SymbolTable&, const JumpTable&, Tuple&) -> Result<Output> {
        parserCalled = true;
        return ok(Output{.success = true, .shouldExit = false});
    }
};

struct TraceConfig {
    using Lexer = etched::detail::DefaultLexer<etched::DefaultLexerConfig>;
    using Parser = TraceParser;
    template <size_t N>
    using SymbolTable = etched::detail::DefaultSymbolTable<N, etched::DefaultSTConfig>;
    template <typename... Args>
    using JumpTable = etched::detail::DefaultJumpTable<Lexer, Args...>;
    using helpConfig = etched::DefaultHelpConfig;
    static constexpr size_t maxArgs = 10;
};

// 2. Custom Lexer for testing replacement
struct MockLexer {
    using TokenType = etched::detail::TokenType;
    
    std::vector<etched::detail::Token> mockTokens;
    size_t currentIdx = 0;
    etched::detail::Token lastToken{};

    MockLexer() = default;

    void setTokens(int, const char**) {}

    auto nextToken(etched::detail::ParsingContext = etched::detail::ParsingContext::DEFAULT) -> Result<etched::detail::Token> {
        if (currentIdx >= mockTokens.size()) {
            lastToken = { "", TokenType::END_OF_INPUT };
            return ok(lastToken);
        }
        lastToken = mockTokens[currentIdx++];
        return ok(lastToken);
    }

    auto currentToken() const -> etched::detail::Token {
        return lastToken;
    }
};

struct DummyParser {
    template <typename Lexer, typename SymbolTable, typename JumpTable, typename Tuple>
    static auto parse(Lexer&, const SymbolTable&, const JumpTable&, Tuple&) -> Result<Output> {
        return ok(Output{.success = true, .shouldExit = false});
    }
};

struct LexerTestConfig {
    using Lexer = MockLexer;
    using Parser = DummyParser;
    template <size_t N>
    using SymbolTable = etched::detail::DefaultSymbolTable<N, etched::DefaultSTConfig>;
    template <typename... Args>
    using JumpTable = etched::detail::DefaultJumpTable<Lexer, Args...>;
    using helpConfig = etched::DefaultHelpConfig;
    static constexpr size_t maxArgs = 10;
};

inline void customParserReplacementTest() {
    auto parser = ArgumentParser(WithConfig<TraceConfig>{},
        "TestApp", "TestDescription",
        optInt<"port">('p', "--port", "Port", some(8080))
    );

    const char* argv[] = {"prog"};
    parserCalled = false;
    auto result = parser.parse(1, argv);

    if (!result.isOk()) throw "Custom parser failed";
    if (!parserCalled) throw "Custom parser was not called";
}

inline void customLexerReplacementTest() {
    auto parser = ArgumentParser(WithConfig<LexerTestConfig>{},
        "TestApp", "TestDescription",
        optInt<"port">('p', "--port", "Port", some(8080))
    );

    const char* argv[] = {"prog"};
    auto result = parser.parse(1, argv);

    if (!result.isOk()) throw "Custom lexer config failed";
}

inline void runComponentReplacementTests() {
    std::printf("  customParserReplacementTest...");
    customParserReplacementTest();
    std::printf(" OK\n");

    std::printf("  customLexerReplacementTest...");
    customLexerReplacementTest();
    std::printf(" OK\n");
}

}  // namespace etched::tests
