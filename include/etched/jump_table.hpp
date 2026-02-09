#ifndef JUMP_TABLE_HPP
#define JUMP_TABLE_HPP

#include <array>
#include <tuple>
#include <utility>

#include "contracts.hpp"
#include "converters.hpp"
#include "types.hpp"

namespace etched::detail {

template <IsLexer Lexer, IsArgument... Args>
class DefaultJumpTable {
  using ArgumentTuple = std::tuple<Args...>;
  using Handler = Result<Output> (*)(ArgumentTuple&, Lexer&);
  std::array<Handler, sizeof...(Args)> handlers_;

  template <size_t I>
  static auto handle(ArgumentTuple& args, Lexer& l) -> Result<Output> {
    auto& item = std::get<I>(args);
    using ItemType = std::decay_t<decltype(item)>;
    if constexpr (IsCommand<ItemType>) {
      return item.parse(l);
    } else if constexpr (IsPositionalOption<ItemType>) {
      auto token = l.currentToken();
      using InnerType = typename ItemType::InnerValueType;
      auto dRes = deserialize(token.value, static_cast<InnerType*>(nullptr));
      if (!dRes.isOk()) {
        return err<Output>(dRes.unwrapErr());
      }
      item.value.add(dRes.unwrap());
      auto tRes = l.nextToken();
      if (!tRes.isOk()) {
        return err<Output>(std::move(tRes.unwrapErr()));
      }
      return ok(Output{.success = true, .shouldExit = false});
    } else {
      if constexpr (IsOptionFlag<ItemType>) {
        item.value = true;
        auto tRes = l.nextToken();
        if (!tRes.isOk()) {
          return err<Output>(std::move(tRes.unwrapErr()));
        }
      } else {
        using ValueType = typename ItemType::ValueType;
        auto ntRes = l.nextToken(ParsingContext::WAITING_FOR_VALUE);
        if (!ntRes.isOk()) {
          return err<Output>(std::move(ntRes.unwrapErr()));
        }
        auto nextToken = ntRes.unwrap();
        if (nextToken.type == Lexer::TokenType::END_OF_INPUT) {
          return err<Output>(
              "Expected value for option but found end of input");
        }
        std::string_view token = nextToken.value;
        auto result = deserialize(token, static_cast<ValueType*>(nullptr));
        if (!result.isOk()) {
          return err<Output>(result.unwrapErr());
        }
        item.value = result.unwrap();
        auto tRes = l.nextToken();
        if (!tRes.isOk()) {
          return err<Output>(std::move(tRes.unwrapErr()));
        }
      }
      return item.trigger();
    }
  }

 public:
  [[nodiscard]] auto dispatch(size_t index, ArgumentTuple& args, Lexer& l) const
      -> Result<Output> {
    if (index >= handlers_.size()) {
      return err<Output>("Invalid jump table index");
    }
    return handlers_[index](args, l);  // NOLINT
  }

  [[nodiscard]] static consteval auto create() {
    auto jt = DefaultJumpTable<Lexer, Args...>{};
    jt.handlers_ = []<size_t... Is>(std::index_sequence<Is...>) -> auto {
      return std::array<Handler, sizeof...(Is)>{&handle<Is>...};
    }(std::make_index_sequence<sizeof...(Args)>{});
    return jt;
  }
};

}  // namespace etched::detail

#endif  // JUMP_TABLE_HPP
