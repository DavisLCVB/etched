#pragma once
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "converters.hpp"
#include "option.hpp"
#include "strings.hpp"

#ifndef ETCHED_HELPERS_HPP
#define ETCHED_HELPERS_HPP

namespace etched {

// Generic option helper
template <typename T, detail::String Tag>
consteval auto opt(
    std::optional<const char*> shortName = std::nullopt,    // NOLINT
    std::optional<const char*> longName = std::nullopt,     // NOLINT
    std::optional<const char*> description = std::nullopt,  // NOLINT
    std::optional<T> defaultValue = std::nullopt) {
  std::optional<const char*> shortNameChecked =
      (shortName && shortName.has_value()) ? shortName : std::nullopt;
  std::optional<const char*> longNameChecked =
      (longName && longName.has_value()) ? longName : std::nullopt;
  std::optional<const char*> descriptionChecked =
      (description && description.has_value()) ? description : std::nullopt;
  if (!shortName && !longName) {
    throw std::invalid_argument(
        "At least one of shortName or longName must be provided");
  }
  return detail::Option<T, detail::trim<Tag>()>{
      .value = defaultValue,
      .shortName = shortNameChecked,
      .longName = longNameChecked,
      .description = descriptionChecked,
      .defaultValue = defaultValue,
  };
}

// Type-specific helpers

template <detail::String Tag, typename T = int>
  requires detail::Integer<T>
consteval auto optInt(
    std::optional<const char*> shortName = std::nullopt,    // NOLINT
    std::optional<const char*> longName = std::nullopt,     // NOLINT
    std::optional<const char*> description = std::nullopt,  // NOLINT
    std::optional<int> defaultValue = std::nullopt) {
  return opt<T, Tag>(shortName, longName, description, defaultValue);
}

template <detail::String Tag>
consteval auto optBool(
    std::optional<const char*> shortName = std::nullopt,    // NOLINT
    std::optional<const char*> longName = std::nullopt,     // NOLINT
    std::optional<const char*> description = std::nullopt,  // NOLINT
    std::optional<bool> defaultValue = std::nullopt) {
  return opt<bool, Tag>(shortName, longName, description, defaultValue);
}

template <detail::String Tag>
consteval auto optString(
    std::optional<const char*> shortName = std::nullopt,    // NOLINT
    std::optional<const char*> longName = std::nullopt,     // NOLINT
    std::optional<const char*> description = std::nullopt,  // NOLINT
    std::optional<std::string_view> defaultValue = std::nullopt) {
  return opt<std::string_view, Tag>(shortName, longName, description,
                                    defaultValue);
}

template <detail::String Tag, typename T = double>
  requires std::is_floating_point_v<T>
consteval auto optFloat(
    std::optional<const char*> shortName = std::nullopt,    // NOLINT
    std::optional<const char*> longName = std::nullopt,     // NOLINT
    std::optional<const char*> description = std::nullopt,  // NOLINT
    std::optional<T> defaultValue = std::nullopt) {
  return opt<T, Tag>(shortName, longName, description, defaultValue);
}

template <detail::String Tag, typename CallbackType>
  requires ISCallback<CallbackType>
consteval auto optCallback(
    std::optional<const char*> shortName = std::nullopt,    // NOLINT
    std::optional<const char*> longName = std::nullopt,     // NOLINT
    std::optional<const char*> description = std::nullopt,  // NOLINT
    CallbackType callback = nullptr) {
  std::optional<const char*> shortNameChecked =
      (shortName && shortName.value()) ? shortName : std::nullopt;
  std::optional<const char*> longNameChecked =
      (longName && longName.value()) ? longName : std::nullopt;
  std::optional<const char*> descriptionChecked =
      (description && description.value()) ? description : std::nullopt;
  if (!shortName && !longName) {
    throw std::invalid_argument(
        "At least one of shortName or longName must be provided");
  }
  return detail::OptionWithCallback<bool, detail::trim<Tag>(), CallbackType>{
      .callback = callback,
      .value = std::nullopt,
      .shortName = shortNameChecked,
      .longName = longNameChecked,
      .description = descriptionChecked,
      .defaultValue = std::nullopt,
  };
}

consteval auto optHelp(
    std::optional<const char*> shortName = std::nullopt,   // NOLINT
    std::optional<const char*> longName = std::nullopt) {  // NOLINT
  return opt<bool, "help">(shortName, longName, std::nullopt, std::nullopt);
}

consteval auto optVersion(
    const char* versionString,  // NOLINT - Now required
    std::optional<const char*> shortName = std::nullopt,   // NOLINT
    std::optional<const char*> longName = std::nullopt,    // NOLINT
    std::optional<const char*> description = std::nullopt  // NOLINT
) {
  std::optional<const char*> shortNameChecked =
      (shortName && (shortName.value() != nullptr)) ? shortName : std::nullopt;
  std::optional<const char*> longNameChecked =
      (longName && (longName.value() != nullptr)) ? longName : std::nullopt;
  std::optional<const char*> descriptionChecked =
      (description && (description.value() != nullptr)) ? description
                                                        : std::nullopt;
  if (!shortName && !longName) {
    throw std::invalid_argument(
        "At least one of shortName or longName must be provided");
  }
  auto callback = [versionString]() -> void {
    std::cout << versionString << "\n";
    std::exit(0);
  };
  return detail::OptionWithCallback<bool, "version", decltype(callback)>{
      .callback = callback,
      .value = std::nullopt,
      .shortName = shortNameChecked,
      .longName = longNameChecked,
      .description = descriptionChecked,
      .defaultValue = std::nullopt,
  };
}
}  // namespace etched

#endif  // ETCHED_HELPERS_HPP
