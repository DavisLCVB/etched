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
    -> Result<T> {
  T value;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value,
                                   conversionBase);
  if (ec == std::errc::invalid_argument) {
    return err<T>("Invalid argument for integer deserialization");
  }
  if (ec == std::errc::result_out_of_range) {
    return err<T>("Integer deserialization out of range");
  }
  if (ptr != str.data() + str.size()) {
    return err<T>("Extra characters found after integer deserialization");
  }
  return ok(value);
}

// Boolean deserialization
[[nodiscard]] inline auto deserialize(std::string_view str,
                                      [[maybe_unused]] bool* dummy)
    -> Result<bool> {
  if (str == "true" || str == "1" || str == "ON") {
    return ok(true);
  }
  if (str == "false" || str == "0" || str == "OFF") {
    return ok(false);
  }
  return err<bool>("Invalid argument for boolean deserialization");
}

// Floating-point deserialization
template <std::floating_point T>
[[nodiscard]] auto deserialize(std::string_view str, [[maybe_unused]] T* dummy)
    -> Result<T> {
  T value;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
  if (ec == std::errc::invalid_argument) {
    return err<T>("Invalid argument for floating-point deserialization");
  }
  if (ec == std::errc::result_out_of_range) {
    return err<T>("Floating-point deserialization out of range");
  }
  if (ptr != str.data() + str.size()) {
    return err<T>(
        "Extra characters found after floating-point deserialization");
  }
  return ok(value);
}

// String deserialization
[[nodiscard]] inline auto deserialize(std::string_view str,
                                      [[maybe_unused]] std::string* dummy)
    -> Result<std::string> {
  if (str.empty()) {
    return err<std::string>("Cannot deserialize empty string");
  }
  return ok(std::string(str));
}

[[nodiscard]] inline auto deserialize(std::string_view str,
                                      [[maybe_unused]] std::string_view* dummy)
    -> Result<std::string_view> {
  if (str.empty()) {
    return err<std::string_view>("Cannot deserialize empty string_view");
  }
  return ok(str);
}

}  // namespace etched

#endif  // ETCHED_CONVERTERS_HPP
