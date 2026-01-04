#pragma once
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

#ifndef ETCHED_SANITIZERS_HPP
#define ETCHED_SANITIZERS_HPP

namespace etched {

namespace detail {

struct BasicSanitizer {
  static constexpr char minASCIICode{32};
  static constexpr char maxASCIICode{126};

  static constexpr auto isValid(const char* arg) -> bool {
    if (arg == nullptr || arg[0] == '\0') {
      return false;
    }
    for (std::size_t i = 0; arg[i] != '\0'; ++i) {
      const char c = arg[i];
      // Allow printable ASCII and common whitespace
      // This allows values with spaces but rejects control characters
      if (c < minASCIICode && c != '\t' && c != '\n' && c != '\r') {
        return false;
      }
      if (c > maxASCIICode) {
        return false;
      }
    }
    return true;
  }

  template <std::size_t N>
  static constexpr auto sanitizeArgs(const int argc,
                                     const char* argv[])  // NOLINT
      -> std::pair<std::array<const char*, N>, int> {
    auto cleanedArgs = std::array<const char*, N>{};
    int cleanedArgc = 0;

    for (int i = 0; i < argc && cleanedArgc < static_cast<int>(N); ++i) {
      const char* arg = argv[i];
      if (isValid(arg)) {
        cleanedArgs[cleanedArgc] = arg;
        ++cleanedArgc;
      } else {
        std::string argStr = arg ? std::string(arg) : "<null>";
        throw std::invalid_argument("Invalid argument detected: " + argStr);
      }
    }

    return {cleanedArgs, cleanedArgc};
  }
};

}  // namespace detail

}  // namespace etched

#endif  // ETCHED_SANITIZERS_HPP
