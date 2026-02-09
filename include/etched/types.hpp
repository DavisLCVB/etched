#ifndef ETCHED_TYPES_HPP
#define ETCHED_TYPES_HPP

#include <cstdio>
#include <exception>
#include <string_view>
#include <utility>
#include <variant>

namespace etched {

class CompileError {
 public:
  consteval explicit CompileError([[maybe_unused]] const char* msg) {}
};

class RuntimeError {
 public:
  constexpr RuntimeError(const char* msg, std::string_view context = "")
      : message_(msg), context_(context) {}

  RuntimeError() = default;

  void print() const {
    if (context_.empty()) {
      std::fprintf(stderr, "RuntimeError: %s\n", message_);  // NOLINT
    } else {
      std::fprintf(stderr, "RuntimeError: %s\n ('%.*s')\n", message_,  // NOLINT
                   static_cast<int>(context_.size()), context_.data());
    }
  }

  [[nodiscard]] constexpr auto message() const -> const char* {
    return message_;
  }

 private:
  const char* message_{};
  std::string_view context_;
};

template <typename V>
class Result {
 public:
  template <typename U>
    requires(!std::is_same_v<std::decay_t<U>, Result> &&
             !std::is_same_v<std::decay_t<U>, RuntimeError>)
  Result(U&& value) : result_(std::forward<U>(value)) {}

  Result(RuntimeError error) : result_(std::move(error)) {}

  [[nodiscard]] auto isOk() const noexcept -> bool {
    return std::holds_alternative<V>(result_);
  }

  [[nodiscard]] auto unwrap() const& -> V {
    if (!isOk()) [[unlikely]] {
      terminateWithError();
    }
    return std::get<V>(result_);
  }

  [[nodiscard]] auto unwrap() && -> V {
    if (!isOk()) [[unlikely]] {
      terminateWithError();
    }
    return std::get<V>(std::move(result_));
  }

  [[nodiscard]] auto unwrapErr() const& -> RuntimeError {
    if (std::holds_alternative<V>(result_)) {
      terminateWithError();
    }
    return std::get<RuntimeError>(result_);
  }

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

template <>
class Result<void> {
 public:
  Result() : result_(std::monostate{}) {}

  Result(RuntimeError error) : result_(error) {}

  [[nodiscard]] auto isOk() const noexcept -> bool {
    return std::holds_alternative<std::monostate>(result_);
  }

  void unwrap() const {
    if (!isOk()) {
      std::terminate();
    }
  }

  [[nodiscard]] auto unwrapErr() const& -> RuntimeError {
    if (isOk()) {
      RuntimeError error("Called unwrapErr on an ok Result");
      error.print();
      std::terminate();
    }
    return std::get<RuntimeError>(result_);
  }

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

[[nodiscard]] inline auto ok() -> Result<void> {
  return {};
}

template <typename V>
[[nodiscard]] auto ok(V&& value) -> Result<std::decay_t<V>> {
  return Result<std::decay_t<V>>(std::forward<V>(value));
}

template <typename V = void>
[[nodiscard]] auto err(const RuntimeError& error) -> Result<V> {
  return Result<V>(error);
}

template <typename V = void>
[[nodiscard]] auto err(RuntimeError&& error) -> Result<V> {
  return Result<V>(std::move(error));
}

template <typename V = void>
[[nodiscard]] auto err(const char* msg, std::string_view context = "")
    -> Result<V> {
  return Result<V>(RuntimeError(msg, context));
}

template <typename T>
class Optional {
 public:
  constexpr Optional() : data_(std::monostate{}) {}

  constexpr Optional(T value) : data_(std::move(value)) {}

  // Allow implicit conversion from compatible types
  template <typename U>
    requires std::convertible_to<U, T> &&
             (!std::same_as<std::decay_t<U>, Optional>)
  constexpr Optional(U&& value) : data_(T(std::forward<U>(value))) {}

  [[nodiscard]] constexpr auto hasValue() const noexcept -> bool {
    return std::holds_alternative<T>(data_);
  }

  [[nodiscard]] explicit constexpr operator bool() const noexcept {
    return hasValue();
  }

  [[nodiscard]] constexpr auto get() const -> T {
    if (!hasValue()) {
      std::terminate();
    }
    return std::get<T>(data_);
  }

  // Alias for compatibility with std::optional API
  [[nodiscard]] constexpr auto value() const -> T { return get(); }

  [[nodiscard]] constexpr auto valueOr(const T& defaultValue) const -> T {
    if (hasValue()) {
      return std::get<T>(data_);
    }
    return defaultValue;
  }

 private:
  std::variant<std::monostate, T> data_;
};

template <typename T>
[[nodiscard]] constexpr auto some(T&& value) -> Optional<std::decay_t<T>> {
  return Optional<std::decay_t<T>>(std::forward<T>(value));
}

template <typename T>
[[nodiscard]] constexpr auto none() -> Optional<T> {
  return Optional<T>();
}

struct NoCallbackType {
  void operator()() const {}
};

struct Output {
  bool success;
  bool shouldExit;
};

}  // namespace etched

#endif  // ETCHED_TYPES_HPP