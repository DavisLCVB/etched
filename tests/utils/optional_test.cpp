#include <gtest/gtest.h>

#include <etched/utils/optional.hpp>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace {

using etched::none;
using etched::NullOptional;
using etched::Optional;
using etched::some;

// --- Aliases ---

TEST(OptionalTest, AliasesStandardTypes) {
  static_assert(std::is_same_v<Optional<int>, std::optional<int>>);
  static_assert(std::is_same_v<NullOptional, std::nullopt_t>);
  static_assert(std::is_same_v<std::remove_cv_t<decltype(none)>, NullOptional>);
}

// --- some() ---

TEST(OptionalTest, SomeHoldsValue) {
  auto value = some(42);
  static_assert(std::is_same_v<decltype(value), Optional<int>>);
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, 42);
}

TEST(OptionalTest, SomeDecaysLvalueReferences) {
  const std::string text = "etched";
  auto value = some(text);
  static_assert(std::is_same_v<decltype(value), Optional<std::string>>);
  EXPECT_EQ(*value, "etched");
}

TEST(OptionalTest, SomeDecaysStringLiterals) {
  auto value = some("etched");
  static_assert(std::is_same_v<decltype(value), Optional<const char*>>);
  EXPECT_STREQ(*value, "etched");
}

TEST(OptionalTest, SomeMovesMoveOnlyValues) {
  auto value = some(std::make_unique<int>(7));
  static_assert(
      std::is_same_v<decltype(value), Optional<std::unique_ptr<int>>>);
  ASSERT_NE(*value, nullptr);
  EXPECT_EQ(**value, 7);
}

// --- none ---

TEST(OptionalTest, NoneComparesEqualToEmpty) {
  const Optional<int> empty;
  EXPECT_TRUE(empty == none);
  EXPECT_FALSE(some(1) == none);
}

TEST(OptionalTest, AssigningNoneResets) {
  Optional<int> value = some(1);
  value = none;
  EXPECT_FALSE(value.has_value());
}

// --- Constant evaluation ---

TEST(OptionalTest, WorksInConstantExpressions) {
  constexpr Optional<int> value = some(5);
  static_assert(value.has_value());
  static_assert(*value == 5);

  constexpr Optional<int> empty = none;
  static_assert(!empty.has_value());
  static_assert(empty == none);
}

}  // namespace
