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

    [[nodiscard]] auto parse() -> Result<Output> {
      auto sRes = parseS();
      if (!sRes.isOk()) {
        return sRes;
      }
      if (sRes.unwrap().shouldExit) {
        return ok(Output{.success = true, .shouldExit = true});
      }
      auto tRes = lexer->nextToken();
      if (!tRes.isOk()) {
        return err<Output>(std::move(tRes.unwrapErr()));
      }
      auto token = tRes.unwrap();
      if (!match(token, TokenType::END_OF_INPUT)) {
        return err<Output>("Unexpected token after end of input", token.value);
      }
      return ok(Output{.success = true, .shouldExit = false});
    }

    [[nodiscard]] auto parseS() -> Result<Output> {
      auto tRes = lexer->nextToken();
      if (!tRes.isOk()) {
        return err<Output>(std::move(tRes.unwrapErr()));
      }
      auto token = tRes.unwrap();
      if (match(token, TokenType::END_OF_INPUT)) {
        return ok(Output{.success = true, .shouldExit = false});
      }
      if (match(token, TokenType::SHORT_OPTION) ||
          match(token, TokenType::LONG_OPTION) ||
          match(token, TokenType::POSITIONAL) ||
          match(token, TokenType::SEPARATOR)) {
        auto rRes = parseR();
        if (!rRes.isOk()) {
          return rRes;
        }
        if (rRes.unwrap().shouldExit) {
          return ok(Output{.success = true, .shouldExit = true});
        }
        return ok(Output{.success = true, .shouldExit = false});
      }
      return err<Output>("Unexpected token", token.value);
    }

    [[nodiscard]] auto parseR() -> Result<Output> {
      while (true) {
        auto token = lexer->currentToken();
        if (match(token, TokenType::SHORT_OPTION) ||
            match(token, TokenType::LONG_OPTION)) {
          auto oRes = parseO();
          if (!oRes.isOk()) {
            return oRes;
          }
          if (oRes.unwrap().shouldExit) {
            return ok(Output{.success = true, .shouldExit = true});
          }
          continue;
        }
        if (match(token, TokenType::SEPARATOR)) {
          auto tRes = lexer->nextToken();
          if (!tRes.isOk()) {
            return err<Output>(std::move(tRes.unwrapErr()));
          }
          return parseP();
        }
        if (match(token, TokenType::POSITIONAL)) {
          auto cmdRes = parseC();
          if (!cmdRes.isOk()) {
            return cmdRes;
          }
          if (cmdRes.unwrap().shouldExit) {
            return ok(Output{.success = true, .shouldExit = true});
          }
          continue;
        }
        if (match(token, TokenType::END_OF_INPUT)) {
          return ok(Output{.success = true, .shouldExit = false});
        }
        return err<Output>("Unexpected token in parseR", token.value);
      }
    }

    [[nodiscard]] auto parseC() -> Result<Output> {
      auto token = lexer->currentToken();
      auto metadata = st->find(token.value);
      if (metadata.hasValue() && metadata.get().type == EntryType::COMMAND) {
        auto result = jt->dispatch(metadata.get().index, *options, *lexer);
        if (!result.isOk()) {
          return result;
        }
        if (result.unwrap().shouldExit) {
          return ok(Output{.success = true, .shouldExit = true});
        }
        return ok(Output{.success = true, .shouldExit = false});
      }

      auto posIdx = st->positionalIndex();
      if (posIdx.hasValue()) {
        auto res = jt->dispatch(posIdx.get(), *options, *lexer);
        if (!res.isOk()) {
          return res;
        }
        return ok(Output{.success = true, .shouldExit = false});
      }

      return err<Output>("Unexpected positional argument", token.value);
    }

    [[nodiscard]] auto parseP() -> Result<Output> {
      while (true) {
        auto token = lexer->currentToken();
        if (match(token, TokenType::POSITIONAL)) {
          auto posIdx = st->positionalIndex();
          if (posIdx.hasValue()) {
            auto res = jt->dispatch(posIdx.get(), *options, *lexer);
            if (!res.isOk()) {
              return res;
            }
          } else {
            return err<Output>("Unexpected positional argument", token.value);
          }
          continue;
        }
        if (match(token, TokenType::END_OF_INPUT)) {
          return ok(Output{.success = true, .shouldExit = false});
        }
        return err<Output>("Expected positional argument", token.value);
      }
    }

    [[nodiscard]] auto parseO() -> Result<Output> {
      auto token = lexer->currentToken();
      if (match(token, TokenType::SHORT_OPTION) ||
          match(token, TokenType::LONG_OPTION)) {
        auto metadata = st->find(token.value);
        if (!metadata.hasValue()) {
          return err<Output>("Unknown option encountered", token.value);
        }
        return jt->dispatch(metadata.get().index, *options, *lexer);
      }
      return err<Output>("Expected option", token.value);
    }

    static auto match(const Token& token, TokenType type) -> bool {
      return token.type == type;
    }
  };

  template <typename Lexer, typename SymbolTable, typename JumpTable,
            typename Tuple>
  [[nodiscard]] static auto parse(Lexer& lexer, const SymbolTable& st,
                                  const JumpTable& jt, Tuple& options)
      -> Result<Output> {
    Implementation<Lexer, SymbolTable, JumpTable, Tuple> impl(lexer, st, jt,
                                                              options);
    return impl.parse();
  }
};

}  // namespace etched::detail

#endif  // ETCHED_DEFAULT_HPP
