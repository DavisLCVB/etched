#pragma once
#ifndef ETCHED_UTILS_OPTIONAL_HPP
#define ETCHED_UTILS_OPTIONAL_HPP

#include <optional>
#include <type_traits>
#include <utility>

namespace etched {
template <typename T>
using Optional = std::optional<T>;
using NullOptional = std::nullopt_t;

inline constexpr auto none = std::nullopt;

template <typename T>
[[nodiscard]] constexpr auto some(T&& value) -> Optional<std::decay_t<T>> {
  return std::make_optional(std::forward<T>(value));
}

}  // namespace etched

#endif  // ETCHED_UTILS_OPTIONAL_HPP
