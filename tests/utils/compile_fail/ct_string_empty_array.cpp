// Must fail: an empty std::array has no null terminator.
#include <array>
#include <etched/utils/ct_string.hpp>

constexpr etched::detail::CTString str{std::array<char, 0>{}};

auto main() -> int {
  return 0;
}
