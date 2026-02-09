#ifndef ETCHED_HELP_HPP
#define ETCHED_HELP_HPP

#include <string_view>
#include <tuple>

#include "contracts.hpp"
#include "strings.hpp"

namespace etched::detail {

template <size_t N>
struct HelpBuffer {
  char data[N]{};  // NOLINT
  size_t pos = 0;

  consteval void append(std::string_view sv) {
    for (char c : sv) {
      if (pos < N - 1) {
        data[pos++] = c;
      }
    }
  }

  consteval void append(char c) {
    if (pos < N - 1) {
      data[pos++] = c;
    }
  }
};

template <typename Config, typename OptionsTuple>
consteval auto generateHelp(std::string_view name,  //NOLINT
                            std::string_view desc,  // NOLINT
                            const OptionsTuple& options) {
  HelpBuffer<Config::maxHelpSize> buffer;
  buffer.append(name);
  buffer.append('\n');
  if (!desc.empty()) {
    buffer.append("  ");
    buffer.append(desc);
    buffer.append('\n');
  }
  buffer.append("\nOptions:\n");

  // Options
  auto printOpt = [&](const auto& opt) -> auto {
    using OptType = std::decay_t<decltype(opt)>;
    buffer.append("  ");
    size_t lineStart = buffer.pos;
    // Flags column
    if constexpr (IsPositionalOption<OptType>) {
      buffer.append("<positional>");
    } else if constexpr (IsCommand<OptType>) {
      buffer.append(opt.name);
    } else if constexpr (IsOption<OptType>) {
      bool hasShort = (opt.shortName != '\0');
      bool hasLong = (!opt.longName.empty());

      if (hasShort) {
        buffer.append('-');
        buffer.append(opt.shortName);
      }
      if (hasShort && hasLong) {
        buffer.append(", ");
      }
      if (hasLong) {
        buffer.append("--");
        buffer.append(opt.longName);
      }
    }
    // Padding
    size_t currentCol = buffer.pos - lineStart;
    if (currentCol < Config::colWidth) {
      for (size_t i = 0; i < (Config::colWidth - currentCol); ++i) {
        buffer.append(' ');
      }
    }
    buffer.append("  ");  // Spacer
    // Description
    buffer.append(opt.description);
    if constexpr (IsCommand<OptType>) {
      buffer.append(" (command)");
    }
    buffer.append('\n');
  };
  std::apply([&](const auto&... args) -> auto { (printOpt(args), ...); },
             options);
  buffer.data[buffer.pos] = '\0';
  return String<Config::maxHelpSize>{buffer.data};
}

}  // namespace etched::detail

#endif
