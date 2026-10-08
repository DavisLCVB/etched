#pragma once
#include <cstdio>
#include <etched/utils/ct_string.hpp>
#include <etched/utils/static_optional.hpp>
#include <optional>
#include <string_view>
#include <type_traits>

namespace etched::tests {

namespace static_optional_detail {

struct Props {
  StaticOptional<int> defaultValue;
  StaticOptional<detail::CTString<5>> metavar;
};

template <Props P>
struct OptionStub {
  static constexpr auto props = P;
};

}  // namespace static_optional_detail

inline void staticOptionalEmptyTest() {
  constexpr StaticOptional<int> empty{};
  constexpr StaticOptional<int> fromNullopt = std::nullopt;
  static_assert(!empty.has_value() && !empty);
  static_assert(empty == std::nullopt && empty == fromNullopt);
  static_assert(empty.value_or(7) == 7);

  bool threw = false;
  try {
    (void)empty.value();
  } catch (const std::bad_optional_access&) {
    threw = true;
  }
  if (!threw) {
    throw "value() on an empty StaticOptional should throw";
  }
}

inline void staticOptionalEngagedTest() {
  constexpr StaticOptional<int> port = 8080;
  static_assert(port.has_value() && static_cast<bool>(port));
  static_assert(port.value() == 8080 && *port == 8080);
  static_assert(port.value_or(1) == 8080);
  static_assert(port == 8080 && !(port == 80) && !(port == std::nullopt));
  static_assert(port == StaticOptional<int>{8080});

  StaticOptional<std::string_view> name = std::string_view{"out"};
  if (name->size() != 3) {
    throw "operator-> should reach the stored value";
  }
  name.reset();
  if (name.has_value() || *name != std::string_view{}) {
    throw "reset() should empty the optional and restore T{}";
  }
}

inline void staticOptionalNttpTest() {
  using static_optional_detail::OptionStub;
  using static_optional_detail::Props;

  using WithDefault =
      OptionStub<Props{.defaultValue = 8080, .metavar = std::nullopt}>;
  using WithMetavar = OptionStub<Props{.defaultValue = std::nullopt,
                                       .metavar = detail::CTString{"PORT"}}>;
  using Empty = OptionStub<Props{}>;

  static_assert(WithDefault::props.defaultValue.value() == 8080);
  static_assert(!WithDefault::props.metavar.has_value());
  static_assert(WithMetavar::props.metavar.value().view() == "PORT");
  static_assert(!Empty::props.defaultValue.has_value());

  // Empty optionals must be template-argument-equivalent.
  static_assert(std::is_same_v<OptionStub<Props{}>,
                               OptionStub<Props{.defaultValue = std::nullopt,
                                                .metavar = std::nullopt}>>);
}

inline void runStaticOptionalTests() {
  std::printf("  staticOptionalEmptyTest...");
  staticOptionalEmptyTest();
  std::printf(" OK\n");

  std::printf("  staticOptionalEngagedTest...");
  staticOptionalEngagedTest();
  std::printf(" OK\n");

  std::printf("  staticOptionalNttpTest...");
  staticOptionalNttpTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests
