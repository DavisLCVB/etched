#pragma once
#include <etched/etched.hpp>
#include <cstdio>
#include <vector>
#include <string>

namespace etched::tests {

inline void lexerBasicTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-p", "8080"};
  lexer.setTokens(3, argv);

  auto tok1 = lexer.nextToken().value();
  if (tok1.type != detail::TokenType::SHORT_OPTION || tok1.value != "p") {
    throw "Expected SHORT_OPTION 'p'";
  }

  auto tok2 = lexer.nextToken().value();
  if (tok2.type != detail::TokenType::POSITIONAL || tok2.value != "8080") {
    throw "Expected POSITIONAL '8080'";
  }

  auto tok3 = lexer.nextToken().value();
  if (tok3.type != detail::TokenType::END_OF_INPUT) {
    throw "Expected END_OF_INPUT";
  }
}

inline void lexerShortOptionTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-p", "8080"};
  lexer.setTokens(3, argv);

  auto tok1 = lexer.nextToken().value();
  if (tok1.type != detail::TokenType::SHORT_OPTION || tok1.value != "p") {
    throw "Expected SHORT_OPTION 'p'";
  }
}

inline void lexerLongOptionWithEqualsTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "--port=8080"};
  lexer.setTokens(2, argv);

  auto tok1 = lexer.nextToken().value();
  if (tok1.type != detail::TokenType::LONG_OPTION || tok1.value != "port") {
    throw "Expected LONG_OPTION 'port'";
  }

  auto tok2 = lexer.nextToken().value();
  if (tok2.type != detail::TokenType::POSITIONAL || tok2.value != "8080") {
    throw "Expected POSITIONAL '8080'";
  }
}

inline void lexerSeparatorTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "--", "-p"};
  lexer.setTokens(3, argv);

  auto tok1 = lexer.nextToken().value();
  if (tok1.type != detail::TokenType::SEPARATOR) {
    throw "Expected SEPARATOR";
  }

  auto tok2 = lexer.nextToken().value();
  if (tok2.type != detail::TokenType::POSITIONAL || tok2.value != "-p") {
    throw "Expected POSITIONAL '-p'";
  }
}

inline void lexerClusterTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-abc"};
  lexer.setTokens(2, argv);

  auto tok1 = lexer.nextToken().value();
  if (tok1.type != detail::TokenType::SHORT_OPTION || tok1.value != "a") throw "Cluster a failed";
  
  auto tok2 = lexer.nextToken().value();
  if (tok2.type != detail::TokenType::SHORT_OPTION || tok2.value != "b") throw "Cluster b failed";

  auto tok3 = lexer.nextToken().value();
  if (tok3.type != detail::TokenType::SHORT_OPTION || tok3.value != "c") throw "Cluster c failed";
}

inline void lexerCurrentTokenTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-p"};
  lexer.setTokens(2, argv);

  auto tok1 = lexer.nextToken().value();
  auto curr = lexer.currentToken();
  if (tok1.value != curr.value || tok1.type != curr.type) {
    throw "currentToken mismatch";
  }
}

inline void runLexerTests() {
  std::printf("  lexerBasicTest...");
  lexerBasicTest();
  std::printf(" OK\n");

  std::printf("  lexerShortOptionTest...");
  lexerShortOptionTest();
  std::printf(" OK\n");

  std::printf("  lexerLongOptionWithEqualsTest...");
  lexerLongOptionWithEqualsTest();
  std::printf(" OK\n");

  std::printf("  lexerSeparatorTest...");
  lexerSeparatorTest();
  std::printf(" OK\n");

  std::printf("  lexerClusterTest...");
  lexerClusterTest();
  std::printf(" OK\n");

  std::printf("  lexerCurrentTokenTest...");
  lexerCurrentTokenTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests
