#ifndef ORCHESTRATOR_HPP
#define ORCHESTRATOR_HPP

#include "contracts.hpp"
#include "help.hpp"
#include "strings.hpp"
#include "types.hpp"

namespace etched {

template <detail::IsOrchestratorConfig Config, typename... Options>
class DefaultOrchestrator {
  using Lexer = typename Config::Lexer;
  using Parser = typename Config::Parser;
  using SymbolTable = typename Config::template SymbolTable<sizeof...(Options)>;
  using JumpTable = typename Config::template JumpTable<Options...>;
  using helpConfig = typename Config::helpConfig;

 public:
  using OptionsTuple = std::tuple<Options...>;

 private:
  static_assert(detail::IsLexer<Lexer>, "Lexer must satisfy IsLexer concept");
  static_assert(detail::IsSymbolTable<SymbolTable, Options...>,
                "SymbolTable must satisfy IsSymbolTable concept");
  static_assert(detail::IsJumpTable<JumpTable, Lexer, OptionsTuple>,
                "JumpTable must satisfy IsJumpTable concept");
  static_assert(
      detail::IsParser<Parser, Lexer, SymbolTable, JumpTable, OptionsTuple>,
      "Parser must satisfy IsParser concept");

 public:
  consteval DefaultOrchestrator(std::string_view name,
                                std::string_view description, Options... opts)
      : options_(initOptions(opts)...),
        symbolTable_(SymbolTable::create(options_)),
        jumpTable_(JumpTable::create()),
        lexer_(),
        helpText_(detail::generateHelp<helpConfig>(
            name, description, std::make_tuple(initOptions(opts)...))) {
    if (!symbolTable_.saltFound) {
      throw CompileError("Could not find a perfect hash salt.");
    }
    validateUniqueTags();
    validateUniqueFlags();
  }

  [[nodiscard]] auto parse(const int argc, const char* argv[])  //NOLINT
      -> Result<Output> {                                       // NOLINT
    lexer_.setTokens(argc, argv);
    auto res = Parser::parse(lexer_, symbolTable_, jumpTable_, options_);
    if (res.isOk() && res.unwrap().shouldExit) {
      if constexpr (hasHelpOption()) {
        if (has<"help">()) {
          printHelp();
        }
      }
    }
    return res;
  }

  [[nodiscard]] auto parseRecursive(Lexer& lexer) -> Result<Output> {
    auto res = Parser::parse(lexer, symbolTable_, jumpTable_, options_);
    if (res.isOk() && res.unwrap().shouldExit) {
      if constexpr (hasHelpOption()) {
        if (has<"help">()) {
          printHelp();
        }
      }
    }
    return res;
  }

  void printHelp() const { std::puts(helpText_.data.data()); }

  [[nodiscard]] auto helpText() const -> std::string_view {
    return helpText_.view();
  }

  [[nodiscard]] auto getOptions() -> OptionsTuple { return options_; }

  template <detail::String Tag>
  [[nodiscard]] auto getOption() const -> const auto& {
    return std::get<findOptionIdx<Tag>()>(options_);
  }

  template <detail::String Tag>
  [[nodiscard]] auto get() const -> decltype(auto) {
    const auto& opt = getOption<Tag>();
    using OptType = std::decay_t<decltype(opt)>;
    if constexpr (detail::IsPositionalOption<OptType>) {
      return (opt.value);
    } else if constexpr (detail::IsOption<OptType>) {
      return opt.value;
    } else {
      return getOption<Tag>();
    }
  }

  template <detail::String Tag>
  [[nodiscard]] auto has() const -> bool {
    const auto& opt = getOption<Tag>();
    using OptType = std::decay_t<decltype(opt)>;
    if constexpr (detail::IsPositionalOption<OptType>) {
      return !opt.value.empty();
    } else if constexpr (detail::IsOption<OptType>) {
      return opt.value.has_value();
    } else {
      return opt.matched;
    }
  }

 private:
  OptionsTuple options_;
  SymbolTable symbolTable_;
  JumpTable jumpTable_;
  Lexer lexer_;
  detail::String<helpConfig::maxHelpSize> helpText_;

  [[nodiscard]] static consteval auto hasHelpOption() -> bool {
    if constexpr (sizeof...(Options) == 0) {
      return false;
    }
    return []<size_t... Is>(std::index_sequence<Is...>) -> auto {  // NOLINT
      return (
          (std::tuple_element_t<Is, std::tuple<Options...>>::tag == "help") ||
          ...);
    }(std::make_index_sequence<sizeof...(Options)>{});
  }

  template <detail::IsArgument Arg>
  [[nodiscard]] static consteval auto initOptions(Arg arg) -> Arg {
    if constexpr (detail::IsOption<Arg>) {
      if (arg.defaultValue.has_value()) {
        arg.value = arg.defaultValue.value_or(typename Arg::ValueType{});
      }
    }
    return arg;
  }

  template <std::size_t I = 0, std::size_t J = I + 1>
  static consteval auto validateUniqueTags() -> void {
    if constexpr (I < sizeof...(Options)) {
      if constexpr (J < sizeof...(Options)) {
        using Opt1 = std::tuple_element_t<I, std::tuple<Options...>>;
        using Opt2 = std::tuple_element_t<J, std::tuple<Options...>>;
        if constexpr (Opt1::tag == Opt2::tag) {
          throw CompileError("Duplicate option tag detected");
        }
        validateUniqueTags<I, J + 1>();
      } else {
        validateUniqueTags<I + 1, I + 2>();
      }
    }
  }

  template <std::size_t I = 0, std::size_t J = I + 1>
  consteval auto validateUniqueFlags() const -> void {  // NOLINT
    if constexpr (I < sizeof...(Options)) {
      if constexpr (J < sizeof...(Options)) {
        using Opt1 = std::tuple_element_t<I, OptionsTuple>;
        using Opt2 = std::tuple_element_t<J, OptionsTuple>;
        if constexpr (detail::IsOption<Opt1> && detail::IsOption<Opt2>) {
          const auto& opt1 = std::get<I>(options_);
          const auto& opt2 = std::get<J>(options_);
          if (opt1.shortName != '\0' && opt2.shortName != '\0') {
            if (opt1.shortName == opt2.shortName) {
              throw CompileError("Duplicate short flag detected");
            }
          }
          if (!opt1.longName.empty() && !opt2.longName.empty()) {
            if (opt1.longName == opt2.longName) {
              throw CompileError("Duplicate long flag detected");
            }
          }
        }
        validateUniqueFlags<I, J + 1>();
      } else {
        validateUniqueFlags<I + 1, I + 2>();
      }
    }
  }

  template <detail::String Tag, std::size_t Index = 0>
  [[nodiscard]] static constexpr auto findOptionIdx() -> std::size_t {
    if constexpr (Index >= sizeof...(Options)) {
      static_assert(Index < sizeof...(Options), "Option not found");
    } else {
      using CurrentOpt = std::tuple_element_t<Index, std::tuple<Options...>>;
      if constexpr (CurrentOpt::tag == Tag) {
        return Index;
      } else {
        return findOptionIdx<Tag, Index + 1>();
      }
    }
  }
};
}  // namespace etched

#endif  // ORCHESTRATOR_HPP
