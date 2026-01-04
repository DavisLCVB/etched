#pragma once
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string_view>

#ifndef ETCHED_CONVERTERS_HPP
#define ETCHED_CONVERTERS_HPP

namespace etched {

// Forward declaration
template <typename T>
auto fromStr(const char* str) -> T;

namespace detail {

template <typename T>
concept SignedInteger = std::is_integral_v<T> && std::is_signed_v<T>;

template <typename T>
concept UnsignedInteger = std::is_integral_v<T> && std::is_unsigned_v<T>;

template <typename T>
concept Integer = SignedInteger<T> || UnsignedInteger<T>;

template <SignedInteger T>
constexpr auto fromStrSigInt(const char* str) -> T {
  int64_t val = std::stoll(str);
  T min = std::numeric_limits<T>::min();
  T max = std::numeric_limits<T>::max();
  if (val < static_cast<int64_t>(min) || val > static_cast<int64_t>(max)) {
    throw std::out_of_range("Value out of range");
  }
  return static_cast<T>(val);
}

template <UnsignedInteger T>
constexpr auto fromStrUnsigInt(const char* str) -> T {
  uint64_t val = std::stoull(str);
  T max = std::numeric_limits<T>::max();
  if (val > static_cast<uint64_t>(max)) {
    throw std::out_of_range("Value out of range");
  }
  return static_cast<T>(val);
}

}  // namespace detail

// Integer specializations
template <detail::Integer T>
auto fromStr(const char* str) -> T {
  if (!str) {
    throw std::invalid_argument("Null pointer passed to fromStr");
  }
  if constexpr (detail::SignedInteger<T>) {
    return detail::fromStrSigInt<T>(str);
  } else if constexpr (detail::UnsignedInteger<T>) {
    return detail::fromStrUnsigInt<T>(str);
  }
}

// Floating point specializations
template <>
inline auto fromStr<double>(const char* str) -> double {
  if (!str) {
    throw std::invalid_argument("Null pointer passed to fromStr<double>");
  }
  return std::stod(str);
}

template <>
inline auto fromStr<float>(const char* str) -> float {
  if (!str) {
    throw std::invalid_argument("Null pointer passed to fromStr<float>");
  }
  return std::stof(str);
}

// String specializations
template <>
inline auto fromStr<const char*>(const char* str) -> const char* {
  return str;
}

template <>
inline auto fromStr<std::string_view>(const char* str) -> std::string_view {
  return str;
}

// Character specialization
template <>
inline auto fromStr<char>(const char* str) -> char {
  if (str == nullptr || str[0] == '\0' || str[1] != '\0') {
    throw std::invalid_argument("Invalid char value");
  }
  return str[0];
}

}  // namespace etched

#endif  // ETCHED_CONVERTERS_HPP
