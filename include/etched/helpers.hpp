#ifndef ETCHED_HELPERS_HPP
#define ETCHED_HELPERS_HPP

#include <iostream>
#include <string_view>
#include <type_traits>

#include "contracts.hpp"
#include "option.hpp"
#include "strings.hpp"
#include "types.hpp"

namespace etched {

/**
 * @brief Constant for missing short option.
 */
constexpr char noShort = '\0';

/**
 * @brief Constant for missing long option.
 */
constexpr std::string_view noLong;

/**
 * @brief Constant for missing description.
 */
constexpr const char* noDesc = "";

template <typename T>
constexpr auto noDefault = etched::none<T>();

/**
 * @defgroup Helpers Option Definition Helpers
 * @brief Functions used to define the command-line interface in a declarative way.
 * @{
 */

// --- Generic Options ---

/**
 * @brief Defines a generic option with a default value.
 * 
 * @tparam T The type of the value (must be deserializable).
 * @tparam Tag Unique compile-time string to identify this option.
 * @param shortName Short flag character (e.g., 'v' for -v). Use '\0' for none.
 * @param longName Long flag (e.g., "verbose" or "--verbose"). Can be empty.
 * @param description Help text for the option.
 * @param defaultValue The value to use if the option is not provided.
 * @return Internal option representation.
 */
template <typename T, detail::String Tag>
[[nodiscard]] consteval auto opt(char shortName, std::string_view longName,
                                 const char* description,
                                 Optional<T> defaultValue) {
  return detail::Option<T, detail::trim<Tag>()>{
      .value = defaultValue,
      .shortName = shortName,
      .longName = detail::stripDashes(longName),
      .description = description,
      .defaultValue = defaultValue};
}

/**
 * @brief Defines a generic option without a default value.
 * 
 * @tparam T The type of the value (must be deserializable).
 * @tparam Tag Unique compile-time string to identify this option.
 * @param shortName Short flag character (e.g., 'v' for -v). Use '\0' for none.
 * @param longName Long flag (e.g., "verbose" or "--verbose").
 * @param description Help text.
 * @return Internal option representation.
 */
template <typename T, detail::String Tag>
[[nodiscard]] consteval auto opt(char shortName, std::string_view longName,
                                 const char* description = {}) {
  return detail::Option<T, detail::trim<Tag>()>{
      .value = {},
      .shortName = shortName,
      .longName = detail::stripDashes(longName),
      .description = description,
      .defaultValue = none<T>()};
}

// --- Specialized Type Helpers ---

/**
 * @brief Defines an integer-based option.
 * 
 * @tparam Tag Unique identifier.
 * @tparam T Integer type (int, uint32_t, etc.). Defaults to int.
 */
template <detail::String Tag, typename T = int>
  requires detail::Integer<T>
[[nodiscard]] consteval auto optInt(char shortName, std::string_view longName,
                                    const char* description,
                                    Optional<T> defaultValue) {
  return opt<T, Tag>(shortName, longName, description, defaultValue);
}

/**
 * @brief Defines an integer-based option explicitly without a default value using Optional<T>.
 */
template <detail::String Tag, typename T = int>
  requires detail::Integer<T>
[[nodiscard]] consteval auto optInt(char shortName, std::string_view longName,
                                    const char* description) {
  return opt<T, Tag>(shortName, longName, description);
}

/**
 * @brief Defines a boolean flag (true if present, false otherwise).
 * 
 * @tparam Tag Unique identifier.
 * @param defaultValue Initial value.
 */
template <detail::String Tag>
[[nodiscard]] consteval auto optBool(char shortName, std::string_view longName,
                                     const char* description,
                                     Optional<bool> defaultValue) {
  return opt<bool, Tag>(shortName, longName, description, defaultValue);
}

/**
 * @brief Defines a boolean flag explicitly without a default value.
 */
template <detail::String Tag>
[[nodiscard]] consteval auto optBool(char shortName, std::string_view longName,
                                     const char* description) {
  return opt<bool, Tag>(shortName, longName, description);
}

/**
 * @brief Defines a string-based option.
 */
template <detail::String Tag>
[[nodiscard]] consteval auto optString(
    char shortName, std::string_view longName, const char* description,
    Optional<std::string_view> defaultValue) {
  return opt<std::string_view, Tag>(shortName, longName, description,
                                    defaultValue);
}

/**
 * @brief Defines a string-based option explicitly without a default value.
 */
template <detail::String Tag>
[[nodiscard]] consteval auto optString(char shortName,
                                       std::string_view longName,
                                       const char* description) {
  return opt<std::string_view, Tag>(shortName, longName, description);
}

/**
 * @brief Defines a floating-point option.
 */
template <detail::String Tag, typename T = double>
  requires std::is_floating_point_v<T>
[[nodiscard]] consteval auto optFloat(char shortName, std::string_view longName,
                                      const char* description,
                                      Optional<T> defaultValue) {
  return opt<T, Tag>(shortName, longName, description, defaultValue);
}

/**
 * @brief Defines a floating-point option explicitly without a default value.
 */
template <detail::String Tag, typename T = double>
  requires std::is_floating_point_v<T>
[[nodiscard]] consteval auto optFloat(char shortName, std::string_view longName,
                                      const char* description) {
  return opt<T, Tag>(shortName, longName, description);
}

// --- Specialized Behavior Helpers ---

/**
 * @brief Defines an option that triggers a callback when encountered.
 * 
 * @tparam Tag Unique identifier.
 * @tparam CallbackType A callable object. Can return void or Result<Output>.
 * @param callback The function to execute.
 */
template <detail::String Tag, typename CallbackType>
  requires detail::IsAction<CallbackType>
[[nodiscard]] consteval auto optCallback(char shortName,
                                         std::string_view longName,
                                         const char* description,
                                         CallbackType callback) {
  return detail::Option<bool, detail::trim<Tag>(), CallbackType>{
      .value = {},
      .shortName = shortName,
      .longName = detail::stripDashes(longName),
      .description = description,
      .defaultValue = {},
      .callback = callback,
  };
}

/**
 * @brief Defines a help option that automatically prints usage and signals exit.
 */
[[nodiscard]] consteval auto optHelp(char shortName,
                                     std::string_view longName) {
  return optCallback<"help">(shortName, longName, "Show help", []() -> auto {
    return ok(Output{.success = true, .shouldExit = true});
  });
}

/**
 * @brief Defines a version option.
 */
[[nodiscard]] consteval auto optVersion(
    [[maybe_unused]] const char* versionString, char shortName,
    std::string_view longName, const char* description = "Show version") {
  return optCallback<"version">(
      shortName, longName, description, [versionString]() -> auto {
        std::cout << versionString << '\n';
        return ok(Output{.success = true, .shouldExit = true});
      });
}

/**
 * @brief Defines a collector for positional arguments.
 * 
 * @tparam T The type of each positional argument.
 */
template <typename T = std::string_view>
[[nodiscard]] consteval auto optPositional(const char* description) {
  return detail::PositionalOption<T>{.value = {},
                                     .description = description,
                                     .shortName = '\0',
                                     .longName = {}};
}

/**
 * @brief Defines a subcommand with its own set of arguments.
 * 
 * @tparam Tag Unique identifier (the name of the command).
 * @param description Description of the subcommand.
 * @param opts Options and arguments specific to this subcommand.
 */
template <detail::String Tag, detail::IsArgument... SubOptions>
[[nodiscard]] consteval auto cmd(const char* description, SubOptions... opts) {
  return detail::Command<Tag, DefaultConfig, SubOptions...>{description,
                                                                    opts...};
}

/** @} */

}  // namespace etched

#endif  // ETCHED_HELPERS_HPP
