// Must fail: substr cannot read past the end of the string.
#include <etched/utils/ct_string.hpp>

constexpr etched::detail::CTString str{"abc"};
constexpr auto out = str.substr<2, 2>();

auto main() -> int {
  return 0;
}
