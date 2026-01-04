#pragma once
#include <array>
#include <cstddef>
#include <stdexcept>
#include <tuple>

#include "concepts.hpp"
#include "parsers.hpp"
#include "sanitizers.hpp"
#include "strings.hpp"

#ifndef ETCHED_ARGUMENT_PARSER_HPP
#define ETCHED_ARGUMENT_PARSER_HPP

namespace etched {

template <ParserStrategy Strategy = detail::DefaultParserStrategy,
          SanitizerStrategy Sanitizer = detail::BasicSanitizer,
          IsOption... Options>
  requires IsValidVariadicOptions<Options...>
class ArgumentParser {
 public:
  consteval ArgumentParser(Options... opts) : options_(initOptions(opts)...) {
    validateUniqueTags();
    validateUniqueFlags();
  }

  void parse(const int argc, const char* argv[]) {  // NOLINT
    constexpr std::size_t argcMax = 256;
    if (argc > static_cast<int>(argcMax)) {
      throw std::invalid_argument(
          "Too many arguments: maximum " + std::to_string(argcMax) +
          " supported, got " + std::to_string(argc));
    }
    auto [cleanedArgs, cleanedArgc] =
        Sanitizer::template sanitizeArgs<argcMax>(argc, argv);
    std::apply(
        [cleanedArgc, cleanedArgs](auto&... opts) -> auto {  // NOLINT
          Strategy::parse(cleanedArgc, cleanedArgs, opts...);
        },
        options_);
  }

  template <detail::String Tag>
  auto getOption() -> auto& {
    return std::get<findOptionIdx<Tag>()>(options_);
  }

  auto getOptions() { return options_; }

 private:
  std::tuple<Options...> options_;

  template <IsOption Opt>
  static consteval auto initOptions(Opt opt) -> Opt {
    if (opt.defaultValue.has_value()) {
      opt.value = opt.defaultValue.value();
    }
    return opt;
  }

  // Validate unique tags at compile-time
  template <std::size_t I = 0, std::size_t J = I + 1>
  static consteval auto validateUniqueTags() -> void {
    if constexpr (I < sizeof...(Options)) {
      if constexpr (J < sizeof...(Options)) {
        using Opt1 = std::tuple_element_t<I, std::tuple<Options...>>;
        using Opt2 = std::tuple_element_t<J, std::tuple<Options...>>;
        if constexpr (Opt1::tag == Opt2::tag) {
          throw std::invalid_argument("Duplicate option tag detected");
        }
        validateUniqueTags<I, J + 1>();
      } else {
        validateUniqueTags<I + 1, I + 2>();
      }
    }
  }

  // Validate unique flags at compile-time
  template <std::size_t I = 0, std::size_t J = I + 1>
  consteval auto validateUniqueFlags() const -> void {
    if constexpr (I < sizeof...(Options)) {
      if constexpr (J < sizeof...(Options)) {
        const auto& opt1 = std::get<I>(options_);
        const auto& opt2 = std::get<J>(options_);
        if (opt1.shortName && opt2.shortName) {
          if (detail::equals(opt1.shortName.value(), opt2.shortName.value())) {
            throw std::invalid_argument("Duplicate short flag detected");
          }
        }
        if (opt1.longName && opt2.longName) {
          if (detail::equals(opt1.longName.value(), opt2.longName.value())) {
            throw std::invalid_argument("Duplicate long flag detected");
          }
        }
        validateUniqueFlags<I, J + 1>();
      } else {
        validateUniqueFlags<I + 1, I + 2>();
      }
    }
  }

  template <detail::String Tag, std::size_t Index = 0>
  static constexpr auto findOptionIdx() -> std::size_t {
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

#endif  // ETCHED_ARGUMENT_PARSER_HPP
