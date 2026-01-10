#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

#ifndef ETCHED_STRINGS_HPP
#define ETCHED_STRINGS_HPP

namespace etched::detail {

// Main String type used for option tags
template <std::size_t N>
struct String {
  std::array<char, N> data;

  consteval String(const char (&str)[N]) {  // NOLINT
    std::copy_n(str, N, data.data());       // NOLINT
  }

  constexpr String() = default;

  [[nodiscard]] constexpr auto view() const -> std::string_view {
    return {data.data(), N > 0 ? N - 1 : 0};
  }

  constexpr operator std::string_view() const { return view(); }

  template <std::size_t M>
  constexpr bool operator==(const String<M>& other) const {  // NOLINT
    if constexpr (N != M) {
      return false;
    }
    return data == other.data;
  }

  constexpr auto operator==(std::string_view other) const -> bool {
    return view() == other;
  }

  constexpr auto operator==(const char* other) const -> bool {
    return view() == std::string_view{other};
  }

  constexpr operator const char*() const { return data.data(); }
};

template <std::size_t N>
String(const char (&)[N]) -> String<N>;  // NOLINT

// String helper functions

template <String S>
consteval auto trim() {
  constexpr auto sv = S.view();
  constexpr auto first = sv.find_first_not_of(' \t\n\r');
  if (first == std::string_view::npos) {
    return String<1>{""};
  }
  constexpr auto last = sv.find_last_not_of(' \t\n\r');
  constexpr auto trimmedSize = last - first + 1;

  String<trimmedSize + 1> result{};
  for (std::size_t i = 0; i < trimmedSize; ++i) {
    result.data[i] = sv[first + i];
  }
  result.data[trimmedSize] = '\0';
  return result;
}

}  // namespace etched::detail

#endif  // ETCHED_STRINGS_HPP
