#ifndef ETCHED_UTILS_STRING_HPP
#define ETCHED_UTILS_STRING_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace etched::detail {

template <std::size_t N>
struct CTString {
  std::array<char, N> data{};

  consteval CTString(const char (&str)[N]) {  // NOLINT
    std::copy_n(str, N, data.begin());        // NOLINT
  }

  consteval CTString(const std::array<char, N>& arr) {  // NOLINT
    std::copy_n(arr.begin(), N, data.begin());
  }

  constexpr CTString() = default;

  // --- Size / element access ---

  [[nodiscard]] static constexpr auto size() -> std::size_t {
    return N > 0 ? N - 1 : 0;
  }

  [[nodiscard]] static constexpr auto empty() -> bool { return size() == 0; }

  [[nodiscard]] constexpr auto operator[](std::size_t i) const -> char {
    return data[i];
  }

  [[nodiscard]] constexpr auto begin() const { return data.begin(); }
  [[nodiscard]] constexpr auto end() const { return data.begin() + size(); }

  // --- Conversions ---

  [[nodiscard]] constexpr auto view() const -> std::string_view {
    return {data.data(), size()};
  }

  [[nodiscard]] constexpr operator std::string_view() const { return view(); }
  [[nodiscard]] constexpr operator const char*() const { return data.data(); }

  // --- Comparison ---

  template <std::size_t M>
  [[nodiscard]] constexpr auto operator==(const CTString<M>& other) const
      -> bool {
    if constexpr (N != M) {
      return false;
    }
    return view() == other.view();
  }

  [[nodiscard]] constexpr auto operator==(std::string_view sv) const -> bool {
    return view() == sv;
  }

  [[nodiscard]] constexpr auto operator==(const char* str) const -> bool {
    return view() == std::string_view{str};
  }

  // --- String predicates (delegated to string_view) ---

  [[nodiscard]] constexpr auto startsWith(std::string_view sv) const -> bool {
    return view().starts_with(sv);
  }

  [[nodiscard]] constexpr auto startsWith(char c) const -> bool {
    return view().starts_with(c);
  }

  [[nodiscard]] constexpr auto endsWith(std::string_view sv) const -> bool {
    return view().ends_with(sv);
  }

  [[nodiscard]] constexpr auto endsWith(char c) const -> bool {
    return view().ends_with(c);
  }

  [[nodiscard]] constexpr auto contains(std::string_view sv) const -> bool {
    return view().find(sv) != std::string_view::npos;
  }

  [[nodiscard]] constexpr auto find(std::string_view sv,
                                    std::size_t pos = 0) const -> std::size_t {
    return view().find(sv, pos);
  }

  // --- Compile-time transformations ---

  template <std::size_t Offset, std::size_t Len>
  [[nodiscard]] consteval auto substr() const -> CTString<Len + 1> {
    static_assert(Offset + Len <= N - 1, "CTString::substr out of bounds");
    CTString<Len + 1> result{};
    for (std::size_t i = 0; i < Len; ++i) {
      result.data[i] = data[Offset + i];
    }
    result.data[Len] = '\0';
    return result;
  }
};

// Deduction guide: CTString{"hello"} -> CTString<6>
template <std::size_t N>
CTString(const char (&)[N]) -> CTString<N>;  // NOLINT

// --- Non-member operators ---

// (N-1) chars + (M-1) chars + null terminator = N+M-1 array size
template <std::size_t N, std::size_t M>
[[nodiscard]] consteval auto operator+(const CTString<N>& a,
                                       const CTString<M>& b)
    -> CTString<N + M - 1> {
  CTString<N + M - 1> result{};
  constexpr auto aLen = CTString<N>::size();
  constexpr auto bLen = CTString<M>::size();
  for (std::size_t i = 0; i < aLen; ++i) { result.data[i] = a.data[i]; }
  for (std::size_t i = 0; i < bLen; ++i) { result.data[aLen + i] = b.data[i]; }
  result.data[aLen + bLen] = '\0';
  return result;
}

// --- Compile-time free functions ---

// Strips leading and trailing whitespace
template <CTString S>
[[nodiscard]] consteval auto trim() {
  constexpr auto sv = S.view();
  constexpr auto first = sv.find_first_not_of(" \t\n\r");
  if constexpr (first == std::string_view::npos) {
    return CTString<1>{""};
  } else {
    constexpr auto last = sv.find_last_not_of(" \t\n\r");
    constexpr auto len = last - first + 1;
    CTString<len + 1> result{};
    for (std::size_t i = 0; i < len; ++i) {
      result.data[i] = sv[first + i];
    }
    result.data[len] = '\0';
    return result;
  }
}

// Removes all leading occurrences of char Prefix
template <CTString S, char Prefix>
[[nodiscard]] consteval auto stripPrefix() {
  constexpr auto sv = S.view();
  constexpr auto first = sv.find_first_not_of(Prefix);
  if constexpr (first == std::string_view::npos) {
    return CTString<1>{""};
  } else {
    constexpr auto len = sv.size() - first;
    CTString<len + 1> result{};
    for (std::size_t i = 0; i < len; ++i) {
      result.data[i] = sv[first + i];
    }
    result.data[len] = '\0';
    return result;
  }
}

}  // namespace etched::detail

#endif  // ETCHED_UTILS_STRING_HPP
