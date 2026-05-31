#ifndef ETCHED_TYPES_HPP
#define ETCHED_TYPES_HPP

#include <cstdio>
#include <exception>
#include <string_view>
#include <utility>
#include <variant>

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
 * @brief A monadic container that holds either a value of type V or a RuntimeError.
 * @tparam V The type of the value held in case of success.
 */
template <typename V>
class Result {
 public:
  /**
   * @brief Construct from a value.
   * @tparam U Type compatible with V.
   * @param value The value to store.
   */
  template <typename U>
    requires(!std::is_same_v<std::decay_t<U>, Result> &&
             !std::is_same_v<std::decay_t<U>, RuntimeError>)
  Result(U&& value) : result_(std::forward<U>(value)) {}

  /**
   * @brief Construct from a RuntimeError.
   * @param error The error to store.
   */
  Result(RuntimeError error) : result_(std::move(error)) {}

  /**
   * @brief Checks if the result contains a value.
   * @return true if successful, false if it contains an error.
   */
  [[nodiscard]] auto isOk() const noexcept -> bool {
    return std::holds_alternative<V>(result_);
  }

  /**
   * @brief Access the value. Terminates the program if it contains an error.
   * @return The stored value.
   */
  [[nodiscard]] auto unwrap() const& -> V {
    if (!isOk()) [[unlikely]] {
      terminateWithError();
    }
    return std::get<V>(result_);
  }

  /**
   * @brief Access the value (move version). Terminates the program if it contains an error.
   * @return The stored value.
   */
  [[nodiscard]] auto unwrap() && -> V {
    if (!isOk()) [[unlikely]] {
      terminateWithError();
    }
    return std::get<V>(std::move(result_));
  }

  /**
   * @brief Access the error. Terminates the program if it contains a value.
   * @return The stored RuntimeError.
   */
  [[nodiscard]] auto unwrapErr() const& -> RuntimeError {
    if (std::holds_alternative<V>(result_)) {
      terminateWithError();
    }
    return std::get<RuntimeError>(result_);
  }

  /**
   * @brief Access the error (move version). Terminates the program if it contains a value.
   * @return The stored RuntimeError.
   */
  [[nodiscard]] auto unwrapErr() && -> RuntimeError {
    if (std::holds_alternative<V>(result_)) {
      terminateWithError();
    }
    return std::get<RuntimeError>(std::move(result_));
  }

 private:
  std::variant<V, RuntimeError> result_;

  [[noreturn]] void terminateWithError() const {
    RuntimeError err;
    if (isOk()) {
      err = RuntimeError("Called unwrapErr on an ok Result");
    } else {
      err = std::get<RuntimeError>(result_);
    }
    err.print();
    std::terminate();
  }
};

/**
 * @brief Specialization of Result for void.
 */
template <>
class Result<void> {
 public:
  /**
   * @brief Success constructor.
   */
  Result() : result_(std::monostate{}) {}

  /**
   * @brief Error constructor.
   * @param error The RuntimeError.
   */
  Result(RuntimeError error) : result_(error) {}

  /**
   * @brief Checks if successful.
   * @return true if successful.
   */
  [[nodiscard]] auto isOk() const noexcept -> bool {
    return std::holds_alternative<std::monostate>(result_);
  }

  /**
   * @brief Terminates the program if it contains an error.
   */
  void unwrap() const {
    if (!isOk()) {
      std::terminate();
    }
  }

  /**
   * @brief Access the error. Terminates if it's a success.
   * @return The stored RuntimeError.
   */
  [[nodiscard]] auto unwrapErr() const& -> RuntimeError {
    if (isOk()) {
      RuntimeError error("Called unwrapErr on an ok Result");
      error.print();
      std::terminate();
    }
    return std::get<RuntimeError>(result_);
  }

  /**
   * @brief Access the error (move version). Terminates if it's a success.
   * @return The stored RuntimeError.
   */
  [[nodiscard]] auto unwrapErr() && -> RuntimeError {
    if (isOk()) {
      RuntimeError error("Called unwrapErr on an ok Result");
      error.print();
      std::terminate();
    }
    return std::get<RuntimeError>(result_);
  }

 private:
  std::variant<std::monostate, RuntimeError> result_;
};

/**
 * @brief Creates a successful Result<void>.
 * @return An ok result.
 */
[[nodiscard]] inline auto ok() -> Result<void> {
  return {};
}

/**
 * @brief Creates a successful Result with a value.
 * @tparam V Value type.
 * @param value The value.
 * @return An ok result containing the value.
 */
template <typename V>
[[nodiscard]] auto ok(V&& value) -> Result<std::decay_t<V>> {
  return Result<std::decay_t<V>>(std::forward<V>(value));
}

/**
 * @brief Creates an error Result.
 * @tparam V Value type (defaults to void).
 * @param error The error.
 * @return A Result containing the error.
 */
template <typename V = void>
[[nodiscard]] auto err(const RuntimeError& error) -> Result<V> {
  return Result<V>(error);
}

/**
 * @brief Creates an error Result (move version).
 * @tparam V Value type (defaults to void).
 * @param error The error.
 * @return A Result containing the error.
 */
template <typename V = void>
[[nodiscard]] auto err(RuntimeError&& error) -> Result<V> {
  return Result<V>(std::move(error));
}

/**
 * @brief Creates an error Result from a message and optional context.
 * @tparam V Value type (defaults to void).
 * @param msg The error message.
 * @param context The context.
 * @return A Result containing the error.
 */
template <typename V = void>
[[nodiscard]] auto err(const char* msg, std::string_view context = "")
    -> Result<V> {
  return Result<V>(RuntimeError(msg, context));
}

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
