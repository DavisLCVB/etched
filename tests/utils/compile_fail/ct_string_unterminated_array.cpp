// Must fail: CTString requires a null-terminated std::array.
#include <array>
#include <etched/utils/ct_string.hpp>

constexpr etched::detail::CTString str{std::array<char, 2>{'a', 'b'}};

auto main() -> int {
  return 0;
}
