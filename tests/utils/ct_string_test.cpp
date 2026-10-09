#include <gtest/gtest.h>

#include <array>
#include <cstring>
#include <etched/utils/ct_string.hpp>
#include <string_view>
#include <type_traits>

namespace {

using etched::detail::CTString;
using etched::detail::stripPrefix;
using etched::detail::trim;

// --- Construction ---

TEST(CTStringTest, ConstructsFromLiteral) {
  constexpr CTString str{"etched"};
  static_assert(str.size() == 6);
  static_assert(!str.empty());
  static_assert(str.view() == "etched");
  EXPECT_EQ(str.view(), std::string_view{"etched"});
}

TEST(CTStringTest, DeducesSizeIncludingTerminator) {
  constexpr CTString str{"abc"};
  static_assert(std::is_same_v<std::remove_cv_t<decltype(str)>, CTString<4>>);
}

TEST(CTStringTest, ConstructsFromEmptyLiteral) {
  constexpr CTString str{""};
  static_assert(str.size() == 0);
  static_assert(str.empty());
  static_assert(str.view().empty());
  EXPECT_TRUE(str.view().empty());
}

TEST(CTStringTest, ConstructsFromSingleCharacter) {
  constexpr CTString str{"x"};
  static_assert(str.size() == 1);
  static_assert(str[0] == 'x');
  EXPECT_EQ(str.view(), std::string_view{"x"});
}

TEST(CTStringTest, ConstructsFromNullTerminatedArray) {
  constexpr CTString str{std::array<char, 3>{'o', 'k', '\0'}};
  static_assert(str.size() == 2);
  static_assert(str.view() == "ok");
  EXPECT_EQ(str.view(), std::string_view{"ok"});
}

// --- Element access ---

TEST(CTStringTest, IndexesCharacters) {
  constexpr CTString str{"abc"};
  static_assert(str[0] == 'a' && str[1] == 'b' && str[2] == 'c');
  static_assert(str[3] == '\0');
}

TEST(CTStringTest, IteratesWithoutTerminator) {
  constexpr CTString str{"abc"};
  static_assert(str.end() - str.begin() == 3);

  std::string_view collected{str.begin(), str.end()};
  EXPECT_EQ(collected, std::string_view{"abc"});
}

// --- Conversions ---

TEST(CTStringTest, ConvertsImplicitlyToStringView) {
  static_assert(std::is_convertible_v<CTString<4>, std::string_view>);

  // Static storage: a constexpr string_view cannot point into a local.
  static constexpr CTString str{"abc"};
  constexpr std::string_view view = str;
  static_assert(view == "abc");
  EXPECT_EQ(view.size(), 3U);
}

TEST(CTStringTest, DoesNotConvertImplicitlyToPointerOrBool) {
  static_assert(!std::is_convertible_v<CTString<4>, const char*>);
  static_assert(!std::is_convertible_v<CTString<4>, bool>);
}

TEST(CTStringTest, ExposesNullTerminatedCString) {
  constexpr CTString str{"abc"};
  EXPECT_EQ(std::strlen(str.c_str()), 3U);
  EXPECT_STREQ(str.c_str(), "abc");
}

// --- Comparison ---

TEST(CTStringTest, ComparesWithCTString) {
  static_assert(CTString{"abc"} == CTString{"abc"});
  static_assert(!(CTString{"abc"} == CTString{"abd"}));
  static_assert(!(CTString{"abc"} == CTString{"abcd"}));
  static_assert(!(CTString{"abcd"} == CTString{"abc"}));
  static_assert(CTString{""} == CTString{""});
}

TEST(CTStringTest, ComparesWithStringView) {
  constexpr CTString str{"abc"};
  static_assert(str == std::string_view{"abc"});
  static_assert(!(str == std::string_view{"ab"}));
  EXPECT_TRUE(str == std::string_view{"abc"});
}

TEST(CTStringTest, ComparesWithCString) {
  constexpr CTString str{"abc"};
  static_assert(str == "abc");
  static_assert(!(str == "abcd"));

  const char* runtime = "abc";
  EXPECT_TRUE(str == runtime);
}

// --- Predicates ---

TEST(CTStringTest, ChecksPrefix) {
  constexpr CTString str{"--port"};
  static_assert(str.startsWith("--"));
  static_assert(str.startsWith('-'));
  static_assert(!str.startsWith("port"));
  static_assert(!CTString{""}.startsWith('-'));
}

TEST(CTStringTest, ChecksSuffix) {
  constexpr CTString str{"file.txt"};
  static_assert(str.endsWith(".txt"));
  static_assert(str.endsWith('t'));
  static_assert(!str.endsWith(".md"));
  static_assert(!CTString{""}.endsWith('t'));
}

TEST(CTStringTest, ChecksContainment) {
  constexpr CTString str{"--long-name"};
  static_assert(str.contains("long"));
  static_assert(str.contains(""));
  static_assert(!str.contains("short"));
}

TEST(CTStringTest, FindsSubstrings) {
  constexpr CTString str{"a-b-c"};
  static_assert(str.find("-") == 1);
  static_assert(str.find("-", 2) == 3);
  static_assert(str.find("x") == std::string_view::npos);
}

// --- Transformations ---

TEST(CTStringTest, ExtractsSubstring) {
  constexpr CTString str{"etched"};
  constexpr auto middle = str.substr<1, 3>();
  static_assert(
      std::is_same_v<std::remove_cv_t<decltype(middle)>, CTString<4>>);
  static_assert(middle.view() == "tch");
  static_assert(str.substr<0, 6>().view() == "etched");
  static_assert(str.substr<6, 0>().empty());
}

TEST(CTStringTest, Concatenates) {
  constexpr auto joined = CTString{"--"} + CTString{"port"};
  static_assert(
      std::is_same_v<std::remove_cv_t<decltype(joined)>, CTString<7>>);
  static_assert(joined.view() == "--port");
  static_assert((CTString{""} + CTString{"x"}).view() == "x");
  static_assert((CTString{"x"} + CTString{""}).view() == "x");
  EXPECT_STREQ(joined.c_str(), "--port");
}

TEST(CTStringTest, TrimsWhitespace) {
  static_assert(trim<CTString{"  name \t\n"}>().view() == "name");
  static_assert(trim<CTString{"name"}>().view() == "name");
  static_assert(trim<CTString{"a b"}>().view() == "a b");
  static_assert(trim<CTString{" \t\r\n"}>().empty());
  static_assert(trim<CTString{""}>().empty());
}

TEST(CTStringTest, StripsRepeatedPrefix) {
  static_assert(stripPrefix<CTString{"--port"}, '-'>().view() == "port");
  static_assert(stripPrefix<CTString{"port"}, '-'>().view() == "port");
  static_assert(stripPrefix<CTString{"a-b"}, '-'>().view() == "a-b");
  static_assert(stripPrefix<CTString{"---"}, '-'>().empty());
}

// --- Non-type template parameter ---

template <CTString S>
struct Tag {
  static constexpr auto value = S;
};

TEST(CTStringTest, WorksAsNonTypeTemplateParameter) {
  static_assert(Tag<"port">::value.view() == "port");
  static_assert(std::is_same_v<Tag<"port">, Tag<CTString{"port"}>>);
  static_assert(!std::is_same_v<Tag<"port">, Tag<"host">>);
}

}  // namespace
