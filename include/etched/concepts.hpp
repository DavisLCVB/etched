#pragma once
#include <array>
#include <concepts>
#include <cstdint>
#include <optional>
#ifndef ETCHED_CONCEPTS_HPP
#define ETCHED_CONCEPTS_HPP

namespace etched {

// Forward declarations
template <typename T>
auto fromStr(const char* str) -> T;

namespace detail {

template <typename T>
concept HasFromStr = requires {
  { fromStr<T>("") } -> std::same_as<T>;
};

}  // namespace detail

template <typename T>
concept IsOption = requires(T t) {
  typename T::ValueType;
  { T::tag } -> std::same_as<decltype((T::tag))>;
  { t.defaultValue } -> std::same_as<std::optional<typename T::ValueType>&>;
  { t.shortName } -> std::same_as<std::optional<const char*>&>;
  { t.longName } -> std::same_as<std::optional<const char*>&>;
  { t.description } -> std::same_as<std::optional<const char*>&>;
};

template <typename T>
concept ISCallback = std::is_invocable_r_v<void, T>;

template <typename T>
concept IsCallbackOption = IsOption<T> && ISCallback<typename T::CallbackT>;

template <typename T>
concept SanitizerStrategy = requires {
  {
    T::template sanitizeArgs<1>(0, static_cast<const char**>(nullptr))
  } -> std::same_as<std::pair<std::array<const char*, 1>, int>>;
};

template <typename T>
concept ParserStrategy = requires {
  {
    T::template parse<1>(0, std::array<const char*, 1>{})
  } -> std::same_as<void>;
};

template <typename... T>
concept IsValidVariadicOptions = (IsOption<T> && ...) && sizeof...(T) > 0 &&
                                 sizeof...(T) < UINT8_MAX;

}  // namespace etched

#endif  // ETCHED_CONCEPTS_HPP
