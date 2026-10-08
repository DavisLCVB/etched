#ifndef ETCHED_UTILS_STATIC_OPTIONAL_HPP
#define ETCHED_UTILS_STATIC_OPTIONAL_HPP

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace etched {

// Optional value that is a structural type, so it can be part of a non-type
// template parameter. std::optional cannot: it has private members.
//
// Differences from std::optional:
// - T must be default-constructible; the storage always holds a T.
// - While empty, the storage holds T{}. This invariant keeps two empty
//   optionals template-argument-equivalent. Writing through operator* or
//   value() on an empty optional breaks it.
//
// The data members are public only because structural types require it.
// Use the member functions instead of accessing them directly.
template <std::default_initializable T>
struct StaticOptional {
  using ValueType = T;

  T storage{};
  bool engaged{false};

  constexpr StaticOptional() = default;

  constexpr StaticOptional(std::nullopt_t /*unused*/) {}  // NOLINT

  template <typename U = T>
    requires(std::constructible_from<T, U &&> &&
             !std::same_as<std::remove_cvref_t<U>, StaticOptional> &&
             !std::same_as<std::remove_cvref_t<U>, std::nullopt_t>)
  constexpr explicit(!std::convertible_to<U &&, T>)
      StaticOptional(U &&value)  // NOLINT
      : storage(std::forward<U>(value)), engaged(true) {}

  // --- Observers ---

  [[nodiscard]] constexpr auto has_value() const -> bool {  // NOLINT
    return engaged;
  }

  [[nodiscard]] constexpr explicit operator bool() const { return engaged; }

  [[nodiscard]] constexpr auto value() const & -> const T & {
    if (!engaged) {
      throw std::bad_optional_access();
    }
    return storage;
  }

  [[nodiscard]] constexpr auto value() & -> T & {
    if (!engaged) {
      throw std::bad_optional_access();
    }
    return storage;
  }

  template <typename U>
  [[nodiscard]] constexpr auto value_or(U &&fallback) const  // NOLINT
      -> T {
    return engaged ? storage : static_cast<T>(std::forward<U>(fallback));
  }

  [[nodiscard]] constexpr auto operator*() const & -> const T & {
    return storage;
  }

  [[nodiscard]] constexpr auto operator*() & -> T & { return storage; }

  [[nodiscard]] constexpr auto operator->() const -> const T * {
    return &storage;
  }

  [[nodiscard]] constexpr auto operator->() -> T * { return &storage; }

  // --- Modifiers ---

  constexpr void reset() {
    storage = T{};
    engaged = false;
  }

  // --- Comparison ---

  [[nodiscard]] constexpr auto operator==(const StaticOptional &other) const
      -> bool {
    if (engaged != other.engaged) {
      return false;
    }
    return !engaged || storage == other.storage;
  }

  [[nodiscard]] constexpr auto operator==(std::nullopt_t /*unused*/) const
      -> bool {
    return !engaged;
  }

  [[nodiscard]] constexpr auto operator==(const T &other) const -> bool {
    return engaged && storage == other;
  }
};

}  // namespace etched

#endif  // ETCHED_UTILS_STATIC_OPTIONAL_HPP
