#include <gtest/gtest.h>

#include <etched/utils/ct_string.hpp>
#include <string_view>

namespace {

using etched::detail::CTString;

TEST(CTStringTest, ConstructsFromLiteral) {
  constexpr CTString str{"etched"};
  static_assert(str.size() == 6);
  EXPECT_EQ(str.view(), std::string_view{"etched"});
}

}  // namespace
