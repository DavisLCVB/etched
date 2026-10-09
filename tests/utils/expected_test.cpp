#include <gtest/gtest.h>

#include <etched/utils/expected.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

// This file is compiled twice: as C++20, where etched provides its own
// Expected, and as C++23, where Expected aliases std::expected. Tests outside
// the HAS_STD_EXPECTED blocks describe behavior both implementations share.

namespace {

using etched::err;
using etched::Expected;
using etched::Unexpected;

using Result = Expected<int, std::string>;

using BadValueAccess = etched::BadExpectedAccess<std::string>;

// --- Construction and observers ---

TEST(ExpectedTest, HoldsValue) {
  Result result{42};
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(static_cast<bool>(result));
  EXPECT_EQ(*result, 42);
  EXPECT_EQ(result.value(), 42);
}

TEST(ExpectedTest, HoldsError) {
  Result result{Unexpected<std::string>{"invalid"}};
  EXPECT_FALSE(result.has_value());
  EXPECT_FALSE(static_cast<bool>(result));
  EXPECT_EQ(result.error(), "invalid");
}

TEST(ExpectedTest, BuildsErrorWithErrHelper) {
  Result result = err(std::string{"invalid"});
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "invalid");
}

TEST(ExpectedTest, ErrHelperDecaysStringLiterals) {
  auto unexpected = err("invalid");
  static_assert(std::is_same_v<decltype(unexpected), Unexpected<const char*>>);

  Result result = err("invalid");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "invalid");
}

TEST(ExpectedTest, ConvertsUnexpectedOfAnotherErrorType) {
  const Unexpected<const char*> lvalue{"from lvalue"};
  Result fromLvalue = lvalue;
  EXPECT_EQ(fromLvalue.error(), "from lvalue");

  Result fromRvalue = Unexpected<const char*>{"from rvalue"};
  EXPECT_EQ(fromRvalue.error(), "from rvalue");

  Expected<void, std::string> voidResult = err("void");
  EXPECT_EQ(voidResult.error(), "void");
}

TEST(ExpectedTest, ConversionIsExplicitWhenErrorIsOnlyConstructible) {
  struct Explicit {
    explicit Explicit(int code) : value(code) {}

    int value;
  };

  static_assert(
      !std::is_convertible_v<Unexpected<int>, Expected<int, Explicit>>);
  static_assert(
      std::is_constructible_v<Expected<int, Explicit>, Unexpected<int>>);

  Expected<int, Explicit> result{Unexpected<int>{4}};
  EXPECT_EQ(result.error().value, 4);
}

TEST(ExpectedTest, ErrHelperDeducesErrorTypeFromLvalue) {
  const std::string message = "invalid";
  auto unexpected = err(message);
  static_assert(std::is_same_v<decltype(unexpected), Unexpected<std::string>>);
  EXPECT_EQ(unexpected.error(), "invalid");
}

TEST(ExpectedTest, ExplicitBoolIsNotImplicit) {
  static_assert(!std::is_convertible_v<Result, bool>);
  static_assert(std::is_constructible_v<bool, Result>);
}

// --- Value access ---

TEST(ExpectedTest, ValueOnErrorThrows) {
  Result result{Unexpected<std::string>{"invalid"}};
  const Result& constResult = result;

  EXPECT_THROW((void)result.value(), BadValueAccess);
  EXPECT_THROW((void)constResult.value(), BadValueAccess);
  EXPECT_THROW((void)std::move(result).value(), BadValueAccess);
}

TEST(ExpectedTest, ValueExceptionCarriesError) {
  Result result{Unexpected<std::string>{"invalid"}};
  try {
    (void)result.value();
    FAIL() << "value() should throw";
  } catch (const BadValueAccess& exception) {
    EXPECT_EQ(exception.error(), "invalid");
  }
}

TEST(ExpectedTest, ValueExceptionSharesBaseClasses) {
  Result result{Unexpected<std::string>{"invalid"}};
  EXPECT_THROW((void)result.value(), etched::BadExpectedAccess<void>);
  EXPECT_THROW((void)result.value(), std::exception);
}

TEST(ExpectedTest, ValueReturnsMutableReference) {
  Result result{1};
  result.value() = 2;
  EXPECT_EQ(*result, 2);

  *result = 3;
  EXPECT_EQ(result.value(), 3);
}

TEST(ExpectedTest, ArrowAccessesMembers) {
  Expected<std::string, int> result{std::string{"abc"}};
  EXPECT_EQ(result->size(), 3U);

  const auto& constResult = result;
  EXPECT_EQ(constResult->size(), 3U);
}

TEST(ExpectedTest, ValueOrReturnsValueOrFallback) {
  Result withValue{5};
  Result withError{Unexpected<std::string>{"invalid"}};

  EXPECT_EQ(withValue.value_or(9), 5);
  EXPECT_EQ(withError.value_or(9), 9);
}

TEST(ExpectedTest, ValueOrConvertsFallback) {
  Expected<long, std::string> result{Unexpected<std::string>{"invalid"}};
  const short fallback = 7;
  EXPECT_EQ(result.value_or(fallback), 7L);
}

TEST(ExpectedTest, RvalueValueOrMovesValue) {
  Expected<std::string, int> result{std::string{"payload"}};
  std::string value = std::move(result).value_or("fallback");
  EXPECT_EQ(value, "payload");
}

TEST(ExpectedTest, SupportsMoveOnlyValues) {
  Expected<std::unique_ptr<int>, std::string> result{std::make_unique<int>(7)};
  ASSERT_TRUE(result.has_value());

  std::unique_ptr<int> owned = std::move(result).value();
  ASSERT_NE(owned, nullptr);
  EXPECT_EQ(*owned, 7);
}

// --- Error access ---

TEST(ExpectedTest, ErrorReturnsMutableReference) {
  Result result{Unexpected<std::string>{"a"}};
  result.error() += "b";
  EXPECT_EQ(result.error(), "ab");
}

TEST(ExpectedTest, RvalueErrorMovesError) {
  Result result{Unexpected<std::string>{"payload"}};
  std::string error = std::move(result).error();
  EXPECT_EQ(error, "payload");
}

#if !defined(HAS_STD_EXPECTED)
// std::expected leaves error() on a value undefined. The fallback
// implementation throws instead.
TEST(ExpectedFallbackTest, ErrorOnValueThrows) {
  Result result{1};
  const Result& constResult = result;

  EXPECT_THROW((void)result.error(), std::bad_variant_access);
  EXPECT_THROW((void)constResult.error(), std::bad_variant_access);
  EXPECT_THROW((void)std::move(result).error(), std::bad_variant_access);
}
#endif

// --- Copy and move ---

TEST(ExpectedTest, CopiesAndAssigns) {
  Result original{5};
  Result copy = original;
  EXPECT_EQ(*copy, 5);

  copy = Result{Unexpected<std::string>{"invalid"}};
  EXPECT_FALSE(copy.has_value());
  EXPECT_EQ(copy.error(), "invalid");

  copy = original;
  EXPECT_EQ(*copy, 5);
}

// --- Constant evaluation ---

TEST(ExpectedTest, WorksInConstantExpressions) {
  constexpr Expected<int, int> withValue{5};
  static_assert(withValue.has_value());
  static_assert(*withValue == 5);
  static_assert(withValue.value() == 5);
  static_assert(withValue.value_or(1) == 5);

  constexpr Expected<int, int> withError{Unexpected<int>{7}};
  static_assert(!withError.has_value());
  static_assert(withError.error() == 7);
  static_assert(withError.value_or(1) == 1);
}

// --- Expected<void, E> ---

using VoidResult = Expected<void, std::string>;

TEST(ExpectedVoidTest, HoldsSuccess) {
  VoidResult result{};
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(static_cast<bool>(result));
  EXPECT_NO_THROW(result.value());
}

TEST(ExpectedVoidTest, HoldsError) {
  VoidResult result = err(std::string{"failed"});
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), "failed");
  EXPECT_THROW(result.value(), BadValueAccess);
}

TEST(ExpectedVoidTest, WorksInConstantExpressions) {
  constexpr Expected<void, int> success{};
  static_assert(success.has_value());

  constexpr Expected<void, int> failure{Unexpected<int>{3}};
  static_assert(!failure.has_value());
  static_assert(failure.error() == 3);
}

}  // namespace
