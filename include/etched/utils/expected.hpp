#pragma once
#ifndef ETCHED_UTILS_EXPECTED_HPP
#define ETCHED_UTILS_EXPECTED_HPP

#include <version>

#ifndef HAS_STD_EXPECTED
#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
#define HAS_STD_EXPECTED 1
#endif
#endif  // HAS_STD_EXPECT

#if defined(HAS_STD_EXPECTED)
#include <expected>

namespace etched {
template <typename T, typename E>
using Expected = std::expected<T, E>;

template <typename E>
using Unexpected = std::unexpected<E>;

template <typename E>
[[nodiscard]] auto err(E&& error) {
  return std::unexpected(std::forward<E>(error));
}

}  // namespace etched
#else

#include <utility>
#include <variant>

namespace etched {

template <typename E>
class Unexpected {
 public:
  constexpr explicit Unexpected(E&& error) : error_(std::move(error)) {}

  constexpr explicit Unexpected(const E& error) : error_(error) {}

  constexpr auto error() const& -> const E& { return error_; }

  constexpr auto error() & -> E& { return error_; }

  constexpr auto error() && -> E&& { return std::move(error_); }

 private:
  E error_;
};

template <typename T, typename E>
class Expected {
 public:
  constexpr Expected(const T& value) : data_(value) {}

  constexpr Expected(T&& value) : data_(std::move(value)) {}

  constexpr Expected(const Unexpected<E>& error) : data_(error) {}

  constexpr Expected(Unexpected<E>&& error) : data_(std::move(error)) {}

  constexpr Expected(const Expected&) = default;
  constexpr Expected(Expected&&) noexcept = default;
  constexpr auto operator=(const Expected&) -> Expected& = default;
  constexpr auto operator=(Expected&&) noexcept -> Expected& = default;
  ~Expected() = default;

  [[nodiscard]] constexpr auto has_value() const  // NOLINT
      -> bool {
    return std::holds_alternative<T>(data_);
  }

  [[nodiscard]] explicit constexpr operator bool() const { return has_value(); }

  [[nodiscard]] constexpr auto operator*() & -> T& {
    return std::get<T>(data_);
  }

  [[nodiscard]] constexpr auto operator*() const& -> const T& {
    return std::get<T>(data_);
  }

  [[nodiscard]] constexpr auto operator*() && -> T&& {
    return std::get<T>(std::move(data_));
  }

  [[nodiscard]] constexpr auto operator->() -> T* {
    return &std::get<T>(data_);
  }

  [[nodiscard]] constexpr auto operator->() const -> const T* {
    return &std::get<T>(data_);
  }

  [[nodiscard]] constexpr auto value() const& -> const T& {
    if (!has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<T>(data_);
  }

  [[nodiscard]] constexpr auto value() & -> T& {
    if (!has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<T>(data_);
  }

  [[nodiscard]] constexpr auto value() && -> T&& {
    if (!has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<T>(std::move(data_));
  }

  template <typename U>
  [[nodiscard]] constexpr auto value_or(  //NOLINT
      U&& default_value) const& -> T {    //NOLINT
    return has_value() ? std::get<T>(data_)
                       : static_cast<T>(std::forward<U>(default_value));
  }

  template <typename U>
  [[nodiscard]] constexpr auto value_or(U&& default_value) && -> T {  // NOLINT
    return has_value() ? std::get<T>(std::move(data_))
                       : static_cast<T>(std::forward<U>(default_value));
  }

  [[nodiscard]] constexpr auto error() const& -> const E& {
    if (has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<Unexpected<E>>(data_).error();
  }

  [[nodiscard]] constexpr auto error() & -> E& {
    if (has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<Unexpected<E>>(data_).error();
  }

  [[nodiscard]] constexpr auto error() && -> E&& {
    if (has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<Unexpected<E>>(std::move(data_)).error();
  }

 private:
  std::variant<T, Unexpected<E>> data_;
};

template <typename E>
class Expected<void, E> {
 public:
  constexpr Expected() : data_(std::monostate{}) {}

  constexpr Expected(const Unexpected<E>& error) : data_(error) {}

  constexpr Expected(Unexpected<E>&& error) : data_(std::move(error)) {}

  constexpr Expected(const Expected&) = default;
  constexpr Expected(Expected&&) noexcept = default;
  constexpr auto operator=(const Expected&) -> Expected& = default;
  constexpr auto operator=(Expected&&) noexcept -> Expected& = default;
  ~Expected() = default;

  [[nodiscard]] constexpr auto has_value() const  // NOLINT
      -> bool {
    return std::holds_alternative<std::monostate>(data_);
  }

  [[nodiscard]] explicit constexpr operator bool() const { return has_value(); }

  constexpr void value() const {
    if (!has_value()) {
      throw std::bad_variant_access();
    }
  }

  [[nodiscard]] constexpr auto error() const& -> const E& {
    if (has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<Unexpected<E>>(data_).error();
  }

  [[nodiscard]] constexpr auto error() & -> E& {
    if (has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<Unexpected<E>>(data_).error();
  }

  [[nodiscard]] constexpr auto error() && -> E&& {
    if (has_value()) {
      throw std::bad_variant_access();
    }
    return std::get<Unexpected<E>>(std::move(data_)).error();
  }

 private:
  std::variant<std::monostate, Unexpected<E>> data_;
};

template <typename E>
[[nodiscard]] auto err(E&& error) {
  return Unexpected(std::forward<E>(error));
}

}  // namespace etched

#endif

#endif  // ETCHED_UTILS_EXPECTED_HPP
