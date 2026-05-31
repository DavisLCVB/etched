#ifndef ETCHED_CONVERTERS_HPP
#define ETCHED_CONVERTERS_HPP

#include <charconv>
#include <concepts>
#include <string>

#include "types.hpp"

namespace etched {

constexpr int32_t conversionBase = 10;

// Integer deserialization
template <std::integral T>
  requires(!std::same_as<T, bool>)
[[nodiscard]] auto deserialize(std::string_view str, [[maybe_unused]] T* dummy)
    -> Expected<T, RuntimeError> {
  T value;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value,
                                   conversionBase);
  if (ec == std::errc::invalid_argument) {
    return etched::err(RuntimeError{"Invalid argument for integer deserialization"});
  }
  if (ec == std::errc::result_out_of_range) {
    return etched::err(RuntimeError{"Integer deserialization out of range"});
  }
  if (ptr != str.data() + str.size()) {
    return etched::err(RuntimeError{"Extra characters found after integer deserialization"});
  }
  return value;
}

// Boolean deserialization
[[nodiscard]] inline auto deserialize(std::string_view str,
                                      [[maybe_unused]] bool* dummy)
    -> Expected<bool, RuntimeError> {
  if (str == "true" || str == "1" || str == "ON") {
    return true;
  }
  if (str == "false" || str == "0" || str == "OFF") {
    return false;
  }
  return etched::err(RuntimeError{"Invalid argument for boolean deserialization"});
}

// Floating-point deserialization
template <std::floating_point T>
[[nodiscard]] auto deserialize(std::string_view str, [[maybe_unused]] T* dummy)
    -> Expected<T, RuntimeError> {
  T value;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
  if (ec == std::errc::invalid_argument) {
    return etched::err(RuntimeError{"Invalid argument for floating-point deserialization"});
  }
  if (ec == std::errc::result_out_of_range) {
    return etched::err(RuntimeError{"Floating-point deserialization out of range"});
  }
  if (ptr != str.data() + str.size()) {
    return etched::err(RuntimeError{
        "Extra characters found after floating-point deserialization"});
  }
  return value;
}

// String deserialization
[[nodiscard]] inline auto deserialize(std::string_view str,
                                      [[maybe_unused]] std::string* dummy)
    -> Expected<std::string, RuntimeError> {
  if (str.empty()) {
    return etched::err(RuntimeError{"Cannot deserialize empty string"});
  }
  return std::string(str);
}

[[nodiscard]] inline auto deserialize(std::string_view str,
                                      [[maybe_unused]] std::string_view* dummy)
    -> Expected<std::string_view, RuntimeError> {
  if (str.empty()) {
    return etched::err(RuntimeError{"Cannot deserialize empty string_view"});
  }
  return str;
}

}  // namespace etched

#endif  // ETCHED_CONVERTERS_HPP
