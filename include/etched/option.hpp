#ifndef ETCHED_OPTION_HPP
#define ETCHED_OPTION_HPP

#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include "config.hpp"
#include "contracts.hpp"
#include "jump_table.hpp"
#include "orchestrator.hpp"
#include "strings.hpp"
#include "symbol_table.hpp"

namespace etched::detail {

template <typename T>
struct PositionalContainer {
  std::vector<T> values;

  [[nodiscard]] auto begin() -> typename std::vector<T>::iterator {
    return values.begin();
  }

  [[nodiscard]] auto end() -> typename std::vector<T>::iterator {
    return values.end();
  }

  [[nodiscard]] auto begin() const -> typename std::vector<T>::const_iterator {
    return values.begin();
  }

  [[nodiscard]] auto end() const -> typename std::vector<T>::const_iterator {
    return values.end();
  }

  [[nodiscard]] auto size() const -> size_t { return values.size(); }

  [[nodiscard]] auto empty() const -> bool { return values.empty(); }

  [[nodiscard]] auto operator[](size_t idx) const -> const T& {
    return values[idx];
  }

  [[nodiscard]] auto operator[](size_t idx) -> T& { return values[idx]; }

  void add(T val) { values.push_back(std::move(val)); }
};

template <IsDeserializable OptValueType>
struct PositionalOption {
  using InnerValueType = OptValueType;
  using ValueType = PositionalContainer<OptValueType>;
  static constexpr String tag = "positional";

  ValueType value;
  std::string_view description;
  Optional<ValueType> defaultValue = none<ValueType>();

  char shortName = '\0';
  std::string_view longName;

  [[nodiscard]] auto trigger() -> Result<Output> {
    return ok(Output{.success = true, .shouldExit = false});
  }
};

template <IsDeserializable OptValueType, String OptTag,
          IsAction OptCallbackType = NoCallbackType>
struct Option {
  using ValueType = OptValueType;
  using CallbackType = OptCallbackType;

  Optional<OptValueType> value;
  char shortName{};
  std::string_view longName;
  std::string_view description;
  Optional<OptValueType> defaultValue;
  static constexpr auto tag = OptTag;
  [[no_unique_address]] CallbackType callback = CallbackType{};

  [[nodiscard]] auto trigger() -> Result<Output> {
    if constexpr (!std::is_same_v<CallbackType, NoCallbackType>) {
      if constexpr (std::is_same_v<std::invoke_result_t<CallbackType>, void>) {
        callback();
      } else {
        auto res = callback();
        if (!res.isOk()) {
          return res;
        }
        return res;
      }
    }
    return ok(Output{.success = true, .shouldExit = false});
  }
};

template <String CmdTag, typename Conf = DefaultConfig, IsArgument... Options>
struct Command {
  using Config = Conf;
  using OptionsTuple = std::tuple<Options...>;

  static constexpr auto tag = CmdTag;
  std::string_view name = CmdTag.view();
  std::string_view description;
  bool matched = false;

  using OrchestratorType =
      etched::DefaultOrchestrator<Conf, Options...>;

  OrchestratorType orchestrator;

  consteval Command(const char* desc, Options... opts)
      : description(desc), orchestrator(CmdTag.view(), desc, opts...) {}

  [[nodiscard]] auto parse(typename Config::Lexer& lexer) -> Result<Output> {
    matched = true;
    return orchestrator.parseRecursive(lexer);
  }

  template <String Tag>
  [[nodiscard]] auto getOption() -> auto& {
    return orchestrator.template getOption<Tag>();
  }

  template <String Tag>
  [[nodiscard]] auto getOption() const -> const auto& {
    return orchestrator.template getOption<Tag>();
  }

  template <String Tag>
  [[nodiscard]] auto get() const -> decltype(auto) {
    return orchestrator.template get<Tag>();
  }

  template <String Tag>
  [[nodiscard]] auto has() const -> bool {
    return orchestrator.template has<Tag>();
  }

  [[nodiscard]] auto getOptions() -> typename OrchestratorType::OptionsTuple {
    return orchestrator.getOptions();
  }
};

}  // namespace etched::detail

#endif  // ETCHED_OPTION_HPP
