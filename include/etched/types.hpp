#ifndef ETCHED_TYPES_HPP
#define ETCHED_TYPES_HPP

#include <cstdio>
#include <string_view>

#include "utils/expected.hpp"

namespace etched {

/**
 * @brief Represents a compile-time error.
 * Used with consteval to trigger compilation failures with custom messages.
 */
class CompileError {
 public:
  /**
   * @brief Construct a new Compile Error object.
   * @param msg The error message to display at compile-time.
   */
  consteval explicit CompileError([[maybe_unused]] const char* msg) {}
};

/**
 * @brief Represents a runtime error occurring during parsing.
 */
class RuntimeError {
 public:
  /**
   * @brief Construct a new Runtime Error object.
   * @param msg The error message.
   * @param context Optional context string (e.g., the token that caused the error).
   */
  constexpr RuntimeError(const char* msg, std::string_view context = "")
      : message_(msg), context_(context) {}

  /**
   * @brief Default constructor.
   */
  RuntimeError() = default;

  /**
   * @brief Prints the error message to stderr.
   */
  void print() const {
    if (context_.empty()) {
      std::fprintf(stderr, "RuntimeError: %s\n", message_);  // NOLINT
    } else {
      std::fprintf(stderr, "RuntimeError: %s\n ('%.*s')\n", message_,  // NOLINT
                   static_cast<int>(context_.size()), context_.data());
    }
  }

  /**
   * @brief Gets the error message.
   * @return The error message string.
   */
  [[nodiscard]] constexpr auto message() const -> const char* {
    return message_;
  }

 private:
  const char* message_{};
  std::string_view context_;
};

/**
 * @brief Default callback type used when no callback is provided for an option.
 */
struct NoCallbackType {
  /**
   * @brief No-op operator.
   */
  void operator()() const {}
};

/**
 * @brief Output status of a parsing operation or callback.
 */
struct Output {
  // Whether the operation succeeded.
  bool success;
  // Whether the application should stop parsing and exit (e.g., after --help).
  bool shouldExit;
};

}  // namespace etched

#endif  // ETCHED_TYPES_HPP
