#ifndef ETCHED_DEFAULT_HPP
#define ETCHED_DEFAULT_HPP

#include "contracts.hpp"

namespace etched::detail {

struct DefaultParser {
  template <IsLexer Lexer, typename SymbolTable, typename JumpTable,
            typename Tuple>
  struct Implementation {
    Lexer* lexer;
    const SymbolTable* st;
    const JumpTable* jt;
    Tuple* options;

    Implementation(Lexer& l, const SymbolTable& s, const JumpTable& j, Tuple& o)
        : lexer(&l), st(&s), jt(&j), options(&o) {}

    [[nodiscard]] auto parse() -> Expected<Output, RuntimeError> {
      auto sRes = parseS();
      if (!sRes.has_value()) {
        return sRes;
      }
      if (sRes.value().shouldExit) {
        return Output{.success = true, .shouldExit = true};
      }
      auto tRes = lexer->nextToken();
      if (!tRes.has_value()) {
        return etched::err(std::move(tRes.error()));
      }
      auto token = tRes.value();
      if (!match(token, TokenType::END_OF_INPUT)) {
        return etched::err(RuntimeError{"Unexpected token after end of input", token.value});
      }
      return Output{.success = true, .shouldExit = false};
    }

    [[nodiscard]] auto parseS() -> Expected<Output, RuntimeError> {
      auto tRes = lexer->nextToken();
      if (!tRes.has_value()) {
        return etched::err(std::move(tRes.error()));
      }
      auto token = tRes.value();
      if (match(token, TokenType::END_OF_INPUT)) {
        return Output{.success = true, .shouldExit = false};
      }
      if (match(token, TokenType::SHORT_OPTION) ||
          match(token, TokenType::LONG_OPTION) ||
          match(token, TokenType::POSITIONAL) ||
          match(token, TokenType::SEPARATOR)) {
        auto rRes = parseR();
        if (!rRes.has_value()) {
          return rRes;
        }
        if (rRes.value().shouldExit) {
          return Output{.success = true, .shouldExit = true};
        }
        return Output{.success = true, .shouldExit = false};
      }
      return etched::err(RuntimeError{"Unexpected token", token.value});
    }

    [[nodiscard]] auto parseR() -> Expected<Output, RuntimeError> {
      while (true) {
        auto token = lexer->currentToken();
        if (match(token, TokenType::SHORT_OPTION) ||
            match(token, TokenType::LONG_OPTION)) {
          auto oRes = parseO();
          if (!oRes.has_value()) {
            return oRes;
          }
          if (oRes.value().shouldExit) {
            return Output{.success = true, .shouldExit = true};
          }
          continue;
        }
        if (match(token, TokenType::SEPARATOR)) {
          auto tRes = lexer->nextToken();
          if (!tRes.has_value()) {
            return etched::err(std::move(tRes.error()));
          }
          return parseP();
        }
        if (match(token, TokenType::POSITIONAL)) {
          auto cmdRes = parseC();
          if (!cmdRes.has_value()) {
            return cmdRes;
          }
          if (cmdRes.value().shouldExit) {
            return Output{.success = true, .shouldExit = true};
          }
          continue;
        }
        if (match(token, TokenType::END_OF_INPUT)) {
          return Output{.success = true, .shouldExit = false};
        }
        return etched::err(RuntimeError{"Unexpected token in parseR", token.value});
      }
    }

    [[nodiscard]] auto parseC() -> Expected<Output, RuntimeError> {
      auto token = lexer->currentToken();
      auto metadata = st->find(token.value);
      if (metadata.has_value() && metadata.value().type == EntryType::COMMAND) {
        auto result = jt->dispatch(metadata.value().index, *options, *lexer);
        if (!result.has_value()) {
          return result;
        }
        if (result.value().shouldExit) {
          return Output{.success = true, .shouldExit = true};
        }
        return Output{.success = true, .shouldExit = false};
      }

      auto posIdx = st->positionalIndex();
      if (posIdx.has_value()) {
        auto res = jt->dispatch(posIdx.value(), *options, *lexer);
        if (!res.has_value()) {
          return res;
        }
        return Output{.success = true, .shouldExit = false};
      }

      return etched::err(RuntimeError{"Unexpected positional argument", token.value});
    }

    [[nodiscard]] auto parseP() -> Expected<Output, RuntimeError> {
      while (true) {
        auto token = lexer->currentToken();
        if (match(token, TokenType::POSITIONAL)) {
          auto posIdx = st->positionalIndex();
          if (posIdx.has_value()) {
            auto res = jt->dispatch(posIdx.value(), *options, *lexer);
            if (!res.has_value()) {
              return res;
            }
          } else {
            return etched::err(RuntimeError{"Unexpected positional argument", token.value});
          }
          continue;
        }
        if (match(token, TokenType::END_OF_INPUT)) {
          return Output{.success = true, .shouldExit = false};
        }
        return etched::err(RuntimeError{"Expected positional argument", token.value});
      }
    }

    [[nodiscard]] auto parseO() -> Expected<Output, RuntimeError> {
      auto token = lexer->currentToken();
      if (match(token, TokenType::SHORT_OPTION) ||
          match(token, TokenType::LONG_OPTION)) {
        auto metadata = st->find(token.value);
        if (!metadata.has_value()) {
          return etched::err(RuntimeError{"Unknown option encountered", token.value});
        }
        return jt->dispatch(metadata.value().index, *options, *lexer);
      }
      return etched::err(RuntimeError{"Expected option", token.value});
    }

    static auto match(const Token& token, TokenType type) -> bool {
      return token.type == type;
    }
  };

  template <typename Lexer, typename SymbolTable, typename JumpTable,
            typename Tuple>
  [[nodiscard]] static auto parse(Lexer& lexer, const SymbolTable& st,
                                  const JumpTable& jt, Tuple& options)
      -> Expected<Output, RuntimeError> {
    Implementation<Lexer, SymbolTable, JumpTable, Tuple> impl(lexer, st, jt,
                                                              options);
    return impl.parse();
  }
};

}  // namespace etched::detail

#endif  // ETCHED_DEFAULT_HPP
