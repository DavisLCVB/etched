#pragma once
#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <type_traits>

#include "concepts.hpp"
#include "converters.hpp"

#ifndef ETCHED_PARSERS_HPP
#define ETCHED_PARSERS_HPP

namespace etched::detail {

struct DefaultParserStrategy {
  template <std::size_t N, IsOption... Options>
  static auto parse(const int argc, std::array<const char*, N> argv,  // NOLINT
                    Options&... opts) -> void {
    for (int i = 1; i < argc; ++i) {
      const char* arg = argv[i];
      if (arg[0] != '-') {
        throw std::invalid_argument(
            std::string("Unexpected positional argument: ") + arg);
      }
      const char* tagName = arg;
      if (arg[0] == '-' && arg[1] == '-') {
        tagName = arg + 2;
      } else if (arg[0] == '-') {
        tagName = arg + 1;
      }

      // Check if this is a terminal option (help/version)
      bool isTerminal = ((isTerminalOption(opts, tagName)) || ...);
      if (isTerminal && i + 1 < argc) {
        throw std::invalid_argument(
            std::string("No arguments allowed after terminal option: ") + arg);
      }

      // Check for help (terminates program immediately)
      ((matchAndSetHelp(opts, tagName, opts...)), ...);

      // Check for callbacks and boolean flags
      bool matchedBool = ((matchAndSetBool(opts, tagName)) || ...);
      bool matchedCallback = ((matchAndSetCallback(opts, tagName)) || ...);
      if (matchedBool || matchedCallback) {
        continue;
      }
      if (i + 1 >= argc) {
        throw std::invalid_argument(std::string("Option requires a value: ") +
                                    arg);
      }
      const char* value = argv[i + 1];
      bool matched = ((matchAndSet(opts, tagName, value)) || ...);
      if (matched) {
        ++i;  // Consume the value argument
      } else {
        throw std::invalid_argument(
            std::string("Unknown option or missing value for option: ") + arg);
      }
    }
  }

  template <IsOption Opt>
  static auto isTerminalOption(const Opt& opt, const char* name)  // NOLINT
      -> bool {
    if constexpr (Opt::tag == "help" || Opt::tag == "version") {
      return matchesName(opt, name);
    }
    return false;
  }

  template <IsOption Opt, IsOption... Options>
  static auto matchAndSetHelp(Opt& opt, const char* name,  // NOLINT
                              Options&... opts)             // NOLINT
      -> void {
    if constexpr (Opt::tag == "help") {
      if (matchesName(opt, name)) {
        printHelp(opts...);
        std::exit(0);
      }
    }
  }

  template <IsOption Opt>
  static auto matchAndSetCallback(Opt& opt, const char* name)  // NOLINT
      -> bool {
    if constexpr (IsCallbackOption<Opt>) {
      if (matchesName(opt, name)) {
        opt.triggerCallback();
        return true;
      }
    }
    return false;
  }

  template <IsOption Opt>
  static auto matchAndSetBool(Opt& opt, const char* name) -> bool {  // NOLINT
    if constexpr (std::is_same_v<typename Opt::ValueType, bool>) {
      if (matchesName(opt, name)) {
        opt.value = true;
        return true;
      }
    }
    return false;
  }

  template <IsOption Opt>
  static auto printHelpOption(const Opt& opt) -> void {
    if (opt.shortName) {
      const char* shortName = opt.shortName.value();
      if (shortName[0] == '-' && shortName[1] != '-') {
        std::cout << shortName;
      }
    }
    if (opt.longName) {
      if (opt.shortName) {
        std::cout << ", ";
      }
      const char* longName = opt.longName.value();
      if (longName[0] == '-' && longName[1] == '-') {
        std::cout << longName;
      }
    }
    if constexpr (!std::is_same_v<typename Opt::ValueType, bool>) {
      std::cout << " <value>";
    }
    if (opt.description) {
      std::cout << "    " << opt.description.value();
    }
    std::cout << "\n";
  }

  template <IsOption... Options>
  static auto printHelp(Options&... opts) -> void {
    ((printHelpOption(opts)), ...);
  }

  template <IsOption Opt>
  static auto matchAndSet(Opt& opt, const char* name,  // NOLINT
                          const char* value)           // NOLINT
      -> bool {
    if constexpr (std::is_same_v<typename Opt::ValueType, bool>) {
      return false;
    }
    if (matchesName(opt, name)) {
      opt.value = fromStr<typename Opt::ValueType>(value);
      return true;
    }
    return false;
  }

  template <IsOption Opt>
  static auto matchesName(const Opt& opt, const char* name) -> bool {
    if (opt.shortName) {
      const char* shortName = opt.shortName.value();
      if (shortName[0] == '-' && shortName[1] != '-') {
        if (strcmp(shortName + 1, name) == 0) {
          return true;
        }
      }
    }
    if (opt.longName) {
      const char* longName = opt.longName.value();
      if (longName[0] == '-' && longName[1] == '-') {
        if (strcmp(longName + 2, name) == 0) {
          return true;
        }
      }
    }
    return false;
  }
};

}  // namespace etched::detail

#endif  // ETCHED_PARSERS_HPP
