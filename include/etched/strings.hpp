#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

#ifndef ETCHED_STRINGS_HPP
#define ETCHED_STRINGS_HPP

namespace etched {

namespace detail {

// Main String type used for option tags
template <std::size_t N>
struct String {
  std::array<char, N> data;

  consteval String(const char (&str)[N]) {  // NOLINT
    std::copy_n(str, N, data.data());
  }

  constexpr String() : data{} {}

  constexpr auto operator==(const String<N>& other) const -> bool {
    return data == other.data;
  }

  constexpr operator const char*() const { return data.data(); }

  static consteval auto isSpace(char c) -> bool {
    return c == ' ' || c == '\t' || c == '\n';
  }

  [[nodiscard]] consteval auto getTrimBounds() const
      -> std::pair<std::size_t, std::size_t> {
    std::size_t start = 0;
    std::size_t end = N - 1;
    while (start < N - 1 && isSpace(data[start])) {
      ++start;
    }
    while (end > start && isSpace(data[end - 1])) {
      --end;
    }
    return std::pair{start, end};
  }
};

template <std::size_t N, std::size_t M>
constexpr auto operator==([[maybe_unused]] const String<N>& n1,
                          [[maybe_unused]] const String<M>& n2) -> bool {
  return false;
}

template <std::size_t N>
constexpr auto operator==(const String<N>& s1, const String<N>& s2) -> bool {
  return s1.data == s2.data;
}

template <std::size_t N, std::size_t M>
constexpr auto operator==(const String<N>& s1, const char (&s2)[M]) -> bool {
  if constexpr (N != M) {
    return false;
  } else {
    for (std::size_t i = 0; i < N; ++i) {
      if (s1.data[i] != s2[i]) {
        return false;
      }
    }
    return true;
  }
}

template <std::size_t N, std::size_t M>
constexpr auto operator==(const char (&s1)[M], const String<N>& s2) -> bool {
  auto str1 = String(s1);
  return str1 == s2;
}

template <std::size_t N>
String(const char (&)[N]) -> String<N>;  // NOLINT

// String helper functions

template <String str>
consteval auto trim() {
  constexpr auto bounds = str.getTrimBounds();
  constexpr auto newSize = bounds.second - bounds.first;
  String<newSize + 1> trimmed{};
  for (std::size_t i = 0; i < newSize; ++i) {
    trimmed.data[i] = str.data[bounds.first + i];
  }
  trimmed.data[newSize] = '\0';
  return trimmed;
}

template <std::size_t N, std::size_t M>
consteval auto equals(const char (&s1)[N],  // NOLINT
                      const char (&s2)[M])  // NOLINT
    -> bool {
  if constexpr (N != M) {
    return false;
  }
  for (std::size_t i = 0; i < N; ++i) {
    if (s1[i] != s2[i]) {
      return false;
    }
  }
  return true;
}

// Overload for comparing const char* pointers (for use in consteval context)
consteval auto equals(const char* s1, const char* s2) -> bool {
  if (s1 == nullptr || s2 == nullptr) {
    return s1 == s2;
  }
  std::size_t i = 0;
  while (s1[i] != '\0' && s2[i] != '\0') {
    if (s1[i] != s2[i]) {
      return false;
    }
    ++i;
  }
  return s1[i] == s2[i];
}

}  // namespace detail

}  // namespace etched

#endif  // ETCHED_STRINGS_HPP
