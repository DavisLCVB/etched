#ifndef ETCHED_STRINGS_HPP
#define ETCHED_STRINGS_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace etched::detail {

// Main String type used for option tags
template <std::size_t N>
struct String {
  std::array<char, N> data;  // NOLINT

  consteval String(const char (&str)[N]) {  // NOLINT
    std::copy_n(str, N, data.data());       // NOLINT
  }

  constexpr String() = default;

  [[nodiscard]] constexpr auto view() const -> std::string_view {
    return {data.data(), N > 0 ? N - 1 : 0};
  }

  [[nodiscard]] constexpr operator std::string_view() const { return view(); }

  template <std::size_t M>
  [[nodiscard]] constexpr auto operator==(const String<M>& other) const
      -> bool {  // NOLINT
    if constexpr (N != M) {
      return false;
    }
    return view() == other.view();
  }

  [[nodiscard]] constexpr auto operator==(std::string_view other) const
      -> bool {
    return view() == other;
  }

  [[nodiscard]] constexpr auto operator==(const char* other) const -> bool {
    return view() == std::string_view{other};
  }

  [[nodiscard]] constexpr operator const char*() const { return data.data(); }
};

template <std::size_t N>
String(const char (&)[N]) -> String<N>;  // NOLINT

template <std::size_t Capacity>
struct BoundedString {
  std::array<char, Capacity> data{};
  std::size_t length = 0;

  constexpr BoundedString() = default;

  consteval BoundedString(std::string_view sv)
      : length(std::min(sv.size(), Capacity - 1)) {
    for (std::size_t i = 0; i < length; ++i) {
      data[i] = sv[i];  // NOLINT
    }
    data[length] = '\0';  // NOLINT
  }

  consteval BoundedString(char c) : length(1) {
    if constexpr (Capacity > 1) {
      data[0] = c;     // NOLINT
      data[1] = '\0';  // NOLINT
    }
  }

  [[nodiscard]] constexpr auto view() const -> std::string_view {
    return {data.data(), length};
  }

  constexpr auto operator==(std::string_view other) const -> bool {
    return view() == other;
  }
};

// String helper functions

template <String S>
[[nodiscard]] consteval auto trim() {
  constexpr auto sv = S.view();
  constexpr auto first = sv.find_first_not_of(" \t\n\r");
  if constexpr (first == std::string_view::npos) {
    return String<1>{""};
  }
  constexpr auto last = sv.find_last_not_of(" \t\n\r");
  constexpr auto trimmedSize = last - first + 1;

  String<trimmedSize + 1> result{};
  for (std::size_t i = 0; i < trimmedSize; ++i) {
    result.data[i] = sv[first + i];  // NOLINT
  }
  result.data[trimmedSize] = '\0';  // NOLINT
  return result;
}

[[nodiscard]] consteval auto stripDashes(std::string_view sv)
    -> std::string_view {
  if (sv.empty()) {
    return sv;
  }
  if (sv.starts_with("-")) {
    return stripDashes(sv.substr(1));
  }
  return sv;
}

}  // namespace etched::detail

#endif  // ETCHED_STRINGS_HPP
