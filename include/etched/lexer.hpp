#ifndef ETCHED_LEXER_HPP
#define ETCHED_LEXER_HPP

#include <cctype>
#include <cstdint>
#include <string_view>

#include "contracts.hpp"
#include "types.hpp"

namespace etched::detail {

template <IsLexerConfig Config>
class DefaultLexer {
 public:
  using TokenType = etched::detail::TokenType;
  enum class RemainingType : uint8_t { LONG, SHORT, NONE };

  struct Remaining {
    std::string_view value;
    RemainingType type = RemainingType::NONE;
  };

  DefaultLexer() = default;

  void setTokens(int argc, const char** argv) {
    tokens_ = argv;
    argc_ = static_cast<size_t>(argc);
    currentIndex_ = 1;  // Skip program name
    escaped_ = false;
  }

  [[nodiscard]] auto currentToken() const -> Token { return lastToken_; }

  [[nodiscard]] auto nextToken(ParsingContext ctx = ParsingContext::DEFAULT)
      -> Expected<Token, RuntimeError> {
    if (escaped_) {
      if (currentIndex_ >= argc_) {
        lastToken_ = Token{.value = "", .type = TokenType::END_OF_INPUT};
        return lastToken_;
      }
      std::string_view token = tokens_[currentIndex_];
      return handlePositional(token, ctx);
    }
    if (remaining_.type != RemainingType::NONE) {
      return handleRemaining(ctx);
    }
    if (currentIndex_ >= argc_) {
      lastToken_ = Token{.value = "", .type = TokenType::END_OF_INPUT};
      return lastToken_;
    }
    std::string_view token = tokens_[currentIndex_];
    if (token == "--") {
      return handleSeparator(ctx);
    }
    if (ctx == ParsingContext::WAITING_FOR_VALUE && Config::prefferValues) {
      return handlePositional(token, ctx);
    }
    if (token.starts_with("--") && token.size() > 2) {
      return handleLongOption(token, ctx);
    }
    if (token.starts_with("-") && token.size() > 1) {
      if (std::isdigit(static_cast<unsigned char>(token[1])) != 0 &&
          ctx == ParsingContext::WAITING_FOR_VALUE) {
        return handlePositional(token, ctx);
      }
      if (std::isdigit(static_cast<unsigned char>(token[1])) == 0 ||
          ctx == ParsingContext::DEFAULT) {
        return handleShortOption(token, ctx);
      }
    }
    return handlePositional(token, ctx);
  }

 protected:
  auto handlePositional(std::string_view token,
                        [[maybe_unused]] ParsingContext ctx) -> Expected<Token, RuntimeError> {
    lastToken_ = Token{.value = token, .type = TokenType::POSITIONAL};
    currentIndex_++;
    return lastToken_;
  }

  auto handleRemaining(ParsingContext ctx) -> Expected<Token, RuntimeError> {
    if (remaining_.type == RemainingType::LONG) {
      auto value = remaining_.value;
      remaining_ = Remaining{};
      lastToken_ = Token{value, TokenType::POSITIONAL};
      return lastToken_;
    }
    if (remaining_.type == RemainingType::SHORT) {
      if (ctx == ParsingContext::WAITING_FOR_VALUE) {
        auto value = remaining_.value;
        remaining_ = Remaining{};
        currentIndex_++;
        lastToken_ = Token{value, TokenType::POSITIONAL};
        return lastToken_;
      }
      auto value = remaining_.value.substr(0, 1);
      if (remaining_.value.size() > 1) {
        remaining_.value = remaining_.value.substr(1);
      } else {
        remaining_ = Remaining{};
        currentIndex_++;
      }
      lastToken_ = Token{value, TokenType::SHORT_OPTION};
      return lastToken_;
    }
    lastToken_ = Token{.value = "", .type = TokenType::END_OF_INPUT};
    return lastToken_;
  }

  auto handleSeparator([[maybe_unused]] ParsingContext ctx) -> Expected<Token, RuntimeError> {
    escaped_ = true;
    ++currentIndex_;
    lastToken_ = Token{.value = "--", .type = TokenType::SEPARATOR};
    return lastToken_;
  }

  auto handleLongOption(std::string_view token,
                        [[maybe_unused]] ParsingContext ctx) -> Expected<Token, RuntimeError> {
    size_t equalPos = token.find('=');
    if (equalPos != std::string_view::npos) {
      lastToken_ = Token{.value = token.substr(2, equalPos - 2),
                         .type = TokenType::LONG_OPTION};
      remaining_ = Remaining{token.substr(equalPos + 1), RemainingType::LONG};
    } else {
      lastToken_ =
          Token{.value = token.substr(2), .type = TokenType::LONG_OPTION};
    }
    currentIndex_++;
    if (ctx == ParsingContext::DEFAULT) {
      return lastToken_;
    }
    if (ctx == ParsingContext::WAITING_FOR_VALUE) {
      if constexpr (!Config::prefferValues) {
        return etched::err(RuntimeError{"Passing long option as value is not allowed"});
      }
      auto value = lastToken_.value;
      lastToken_ = Token{.value = value, .type = TokenType::POSITIONAL};
      return lastToken_;
    }
    return etched::err(RuntimeError{"Invalid context for long option"});
  }

  auto handleShortOption(std::string_view token, ParsingContext ctx)
      -> Expected<Token, RuntimeError> {
    if (token.size() > 2) {
      if constexpr (!Config::allowClusters) {
        return etched::err(RuntimeError{"Clustered short options are not allowed"});
      }
      lastToken_ =
          Token{.value = token.substr(1, 1), .type = TokenType::SHORT_OPTION};
      remaining_ = Remaining{token.substr(2), RemainingType::SHORT};
    } else {
      lastToken_ =
          Token{.value = token.substr(1), .type = TokenType::SHORT_OPTION};
      currentIndex_++;
    }
    if (ctx == ParsingContext::DEFAULT) {
      return lastToken_;
    }
    if (ctx == ParsingContext::WAITING_FOR_VALUE) {
      if constexpr (!Config::prefferValues) {
        return etched::err(RuntimeError{"Passing short option as value is not allowed"});
      }
      auto value = lastToken_.value;
      lastToken_ = Token{.value = value, .type = TokenType::POSITIONAL};
      return lastToken_;
    }
    return etched::err(RuntimeError{"Invalid context for short option"});
  }

 private:
  const char** tokens_ = nullptr;
  size_t argc_ = 0;
  size_t currentIndex_ = 0;
  bool escaped_ = false;
  Remaining remaining_;
  Token lastToken_{};
};

}  // namespace etched::detail

#endif  // ETCHED_LEXER_HPP