#pragma once
#include <optional>
#include "concepts.hpp"
#include "strings.hpp"

#ifndef ETCHED_OPTION_HPP
#define ETCHED_OPTION_HPP

namespace etched::detail {

template <typename T, String OptTag>
  requires HasFromStr<T>
struct Option {
  std::optional<T> value;
  std::optional<const char*> shortName = std::nullopt;
  std::optional<const char*> longName = std::nullopt;
  std::optional<const char*> description = std::nullopt;
  std::optional<T> defaultValue = std::nullopt;
  using ValueType = T;
  static constexpr auto tag = OptTag;
};

template <typename T, String OptTag, typename CallbackType>
  requires HasFromStr<T> && ISCallback<CallbackType>
struct OptionWithCallback {
  using CallbackT = CallbackType;
  CallbackType callback;
  std::optional<T> value;
  std::optional<const char*> shortName = std::nullopt;
  std::optional<const char*> longName = std::nullopt;
  std::optional<const char*> description = std::nullopt;
  std::optional<T> defaultValue = std::nullopt;
  using ValueType = T;
  static constexpr auto tag = OptTag;

  void triggerCallback() { callback(); }
};
}  // namespace etched::detail

#endif  // ETCHED_OPTION_HPP
