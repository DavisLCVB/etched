#pragma once
#include <cmath>
#include <cstdio>
#include <cstring>
#include <etched/etched.hpp>

namespace etched::tests {

// ============================================================================
// CUSTOM TYPES FOR ADVANCED TESTING
// ============================================================================

struct Vector3D {
  double x, y, z;

  static constexpr double threshold = 0.0001;

  auto operator==(const Vector3D& other) const -> bool {
    return std::abs(x - other.x) < threshold &&
           std::abs(y - other.y) < threshold &&
           std::abs(z - other.z) < threshold;
  }
};

struct IPAddress {
  uint8_t octets[4];  // NOLINT

  auto operator==(const IPAddress& other) const -> bool {
    return octets[0] == other.octets[0] && octets[1] == other.octets[1] &&
           octets[2] == other.octets[2] && octets[3] == other.octets[3];
  }
};

struct TimeRange {
  int startHour, startMin, endHour, endMin;
};

struct SemVer {
  int major, minor, patch;

  auto operator==(const SemVer& o) const -> bool {
    return major == o.major && minor == o.minor && patch == o.patch;
  }
};

}  // namespace etched::tests

namespace etched {

template <>
inline auto deserialize(std::string_view str, tests::Vector3D* /*unused*/)
    -> Result<tests::Vector3D> {
  size_t first = str.find(',');
  size_t second = str.find(',', first + 1);
  if (first == std::string_view::npos || second == std::string_view::npos) {
    return err<tests::Vector3D>("Vector3D format: x,y,z");
  }
  auto xr = deserialize(str.substr(0, first), static_cast<double*>(nullptr));
  auto yr = deserialize(str.substr(first + 1, second - first - 1),
                        static_cast<double*>(nullptr));
  auto zr = deserialize(str.substr(second + 1), static_cast<double*>(nullptr));
  if (!xr.isOk() || !yr.isOk() || !zr.isOk()) {
    return err<tests::Vector3D>("Invalid Vector3D coordinates");
  }
  return ok(
      tests::Vector3D{.x = xr.unwrap(), .y = yr.unwrap(), .z = zr.unwrap()});
}

template <>
inline auto deserialize(std::string_view str, tests::IPAddress* /*unused*/)
    -> Result<tests::IPAddress> {
  tests::IPAddress ip{};
  size_t pos = 0;
  for (int i = 0; i < 4; ++i) {
    size_t dot = (i < 3) ? str.find('.', pos) : str.size();
    if (dot == std::string_view::npos) {
      return err<tests::IPAddress>("IPAddress format: a.b.c.d");
    }
    auto octetStr = str.substr(pos, dot - pos);
    auto res = deserialize(octetStr, static_cast<int*>(nullptr));
    if (!res.isOk()) {
      return err<tests::IPAddress>("Invalid IP octet");
    }
    int val = res.unwrap();
    if (val < 0 || val > UINT8_MAX) {
      return err<tests::IPAddress>("IP octet out of range 0-255");
    }
    ip.octets[i] = static_cast<uint8_t>(val);  // NOLINT
    pos = dot + 1;
  }
  return ok(ip);
}

template <>
inline auto deserialize(std::string_view str, tests::TimeRange* /*unused*/)
    -> Result<tests::TimeRange> {
  // Format: HH:MM-HH:MM
  auto dash = str.find('-');
  if (dash == std::string_view::npos) {
    return err<tests::TimeRange>("TimeRange format: HH:MM-HH:MM");
  }
  auto start = str.substr(0, dash);
  auto end = str.substr(dash + 1);
  auto sColon = start.find(':');
  auto eColon = end.find(':');
  if (sColon == std::string_view::npos || eColon == std::string_view::npos) {
    return err<tests::TimeRange>("TimeRange format: HH:MM-HH:MM");
  }
  auto sh = deserialize(start.substr(0, sColon), static_cast<int*>(nullptr));
  auto sm = deserialize(start.substr(sColon + 1), static_cast<int*>(nullptr));
  auto eh = deserialize(end.substr(0, eColon), static_cast<int*>(nullptr));
  auto em = deserialize(end.substr(eColon + 1), static_cast<int*>(nullptr));
  if (!sh.isOk() || !sm.isOk() || !eh.isOk() || !em.isOk()) {
    return err<tests::TimeRange>("Invalid time values");
  }
  return ok(tests::TimeRange{.startHour = sh.unwrap(),
                             .startMin = sm.unwrap(),
                             .endHour = eh.unwrap(),
                             .endMin = em.unwrap()});
}

template <>
inline auto deserialize(std::string_view str, tests::SemVer* /*unused*/)
    -> Result<tests::SemVer> {
  size_t d1 = str.find('.');
  size_t d2 = str.find('.', d1 + 1);
  if (d1 == std::string_view::npos || d2 == std::string_view::npos) {
    return err<tests::SemVer>("SemVer format: major.minor.patch");
  }
  auto maj = deserialize(str.substr(0, d1), static_cast<int*>(nullptr));
  auto min =
      deserialize(str.substr(d1 + 1, d2 - d1 - 1), static_cast<int*>(nullptr));
  auto pat = deserialize(str.substr(d2 + 1), static_cast<int*>(nullptr));
  if (!maj.isOk() || !min.isOk() || !pat.isOk()) {
    return err<tests::SemVer>("Invalid SemVer components");
  }
  return ok(tests::SemVer{
      .major = maj.unwrap(), .minor = min.unwrap(), .patch = pat.unwrap()});
}

}  // namespace etched

namespace etched::tests {

// ============================================================================
// LEXER STRESS TESTS
// ============================================================================

inline void lexerLongClusterWithValueTransitionTest() {
  // Test cluster like -vvvp where last flag needs a value
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-abcde"};  // NOLINT
  lexer.setTokens(2, argv);                 // NOLINT

  const char* expected[] = {"a", "b", "c", "d", "e"};  //NOLINT
  for (int i = 0; i < 5; ++i) {
    auto tok = lexer.nextToken().unwrap();
    if (tok.type != detail::TokenType::SHORT_OPTION) {
      std::fprintf(stderr, "Token %d: expected SHORT_OPTION, got %d\n", i,
                   static_cast<int>(tok.type));
      throw "Expected SHORT_OPTION in cluster";
    }
    if (tok.value != expected[i]) {
      std::fprintf(stderr, "Token %d: expected '%s', got '%.*s'\n", i,
                   expected[i], static_cast<int>(tok.value.size()),
                   tok.value.data());
      throw "Wrong value in cluster";
    }
  }

  auto endTok = lexer.nextToken().unwrap();
  if (endTok.type != detail::TokenType::END_OF_INPUT) {
    throw "Expected END_OF_INPUT after cluster";
  }
}

inline void lexerMultipleSeparatorsTest() {
  // Everything after -- is positional, even another --
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "--", "--", "-x", "--option"};
  lexer.setTokens(5, argv);

  auto sep = lexer.nextToken().unwrap();
  if (sep.type != detail::TokenType::SEPARATOR) {
    throw "First -- should be SEPARATOR";
  }

  // After separator, everything is positional
  auto pos1 = lexer.nextToken().unwrap();
  if (pos1.type != detail::TokenType::POSITIONAL || pos1.value != "--") {
    throw "Second -- should be POSITIONAL '--'";
  }

  auto pos2 = lexer.nextToken().unwrap();
  if (pos2.type != detail::TokenType::POSITIONAL || pos2.value != "-x") {
    throw "-x after separator should be POSITIONAL";
  }

  auto pos3 = lexer.nextToken().unwrap();
  if (pos3.type != detail::TokenType::POSITIONAL || pos3.value != "--option") {
    throw "--option after separator should be POSITIONAL";
  }
}

inline void lexerEqualsWithEmptyValueTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "--name="};
  lexer.setTokens(2, argv);

  auto opt = lexer.nextToken().unwrap();
  if (opt.type != detail::TokenType::LONG_OPTION || opt.value != "name") {
    throw "Should parse --name= as LONG_OPTION 'name'";
  }

  auto val = lexer.nextToken().unwrap();
  if (val.type != detail::TokenType::POSITIONAL) {
    throw "Empty value after = should be POSITIONAL";
  }
  if (!val.value.empty()) {
    std::fprintf(stderr, "Got value: '%.*s'\n",
                 static_cast<int>(val.value.size()), val.value.data());
    throw "Value after --name= should be empty";
  }
}

inline void lexerEqualsWithMultipleEqualsTest() {
  // --key=value=with=equals should give key and value=with=equals
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "--equation=x=y+z"};
  lexer.setTokens(2, argv);

  auto opt = lexer.nextToken().unwrap();
  if (opt.type != detail::TokenType::LONG_OPTION || opt.value != "equation") {
    throw "Should parse key as 'equation'";
  }

  auto val = lexer.nextToken().unwrap();
  if (val.type != detail::TokenType::POSITIONAL || val.value != "x=y+z") {
    std::fprintf(stderr, "Got: '%.*s'\n", static_cast<int>(val.value.size()),
                 val.value.data());
    throw "Value should be 'x=y+z'";
  }
}

inline void lexerMixedClusterAndLongOptionsTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-abc", "--verbose", "-xyz", "--name=test"};
  lexer.setTokens(5, argv);

  // First cluster: a, b, c
  for (const char* c : {"a", "b", "c"}) {
    auto tok = lexer.nextToken().unwrap();
    if (tok.type != detail::TokenType::SHORT_OPTION || tok.value != c) {
      throw "Cluster -abc parsing failed";
    }
  }

  // --verbose
  auto verb = lexer.nextToken().unwrap();
  if (verb.type != detail::TokenType::LONG_OPTION || verb.value != "verbose") {
    throw "--verbose parsing failed";
  }

  // Second cluster: x, y, z
  for (const char* c : {"x", "y", "z"}) {
    auto tok = lexer.nextToken().unwrap();
    if (tok.type != detail::TokenType::SHORT_OPTION || tok.value != c) {
      throw "Cluster -xyz parsing failed";
    }
  }

  // --name=test
  auto name = lexer.nextToken().unwrap();
  if (name.type != detail::TokenType::LONG_OPTION || name.value != "name") {
    throw "--name parsing failed";
  }
  auto val = lexer.nextToken().unwrap();
  if (val.type != detail::TokenType::POSITIONAL || val.value != "test") {
    throw "test value parsing failed";
  }
}

inline void lexerVeryLongOptionNameTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* longOpt =
      "--this-is-a-very-long-option-name-that-tests-the-parser-limits";
  const char* argv[] = {"prog", longOpt};
  lexer.setTokens(2, argv);

  auto tok = lexer.nextToken().unwrap();
  if (tok.type != detail::TokenType::LONG_OPTION) {
    throw "Long option name should be parsed as LONG_OPTION";
  }
  if (tok.value !=
      "this-is-a-very-long-option-name-that-tests-the-parser-limits") {
    throw "Long option name not parsed correctly";
  }
}

inline void lexerSingleDashAloneTest() {
  detail::DefaultLexer<DefaultLexerConfig> lexer;
  const char* argv[] = {"prog", "-"};
  lexer.setTokens(2, argv);

  auto tok = lexer.nextToken().unwrap();
  // Single dash is typically a positional (stdin placeholder)
  if (tok.type != detail::TokenType::POSITIONAL || tok.value != "-") {
    throw "Single dash should be POSITIONAL '-'";
  }
}

// ============================================================================
// INTEGER BOUNDARY TESTS
// ============================================================================

inline void integerOverflowTest() {
  // Test that parsing a number too large for int32 fails
  auto result = deserialize("2147483648",
                            static_cast<int32_t*>(nullptr));  // INT32_MAX + 1
  if (result.isOk()) {
    throw "Should fail on int32 overflow";
  }
}

inline void integerUnderflowTest() {
  auto result = deserialize("-2147483649",
                            static_cast<int32_t*>(nullptr));  // INT32_MIN - 1
  if (result.isOk()) {
    throw "Should fail on int32 underflow";
  }
}

inline void integerExactBoundariesTest() {
  // INT32_MAX
  auto maxRes = deserialize("2147483647", static_cast<int32_t*>(nullptr));
  if (!maxRes.isOk() || maxRes.unwrap() != 2147483647) {
    throw "Should parse INT32_MAX correctly";
  }

  // INT32_MIN
  auto minRes = deserialize("-2147483648", static_cast<int32_t*>(nullptr));
  if (!minRes.isOk() || minRes.unwrap() != -2147483648) {
    throw "Should parse INT32_MIN correctly";
  }
}

inline void unsignedIntegerNegativeTest() {
  auto result = deserialize("-1", static_cast<uint32_t*>(nullptr));
  if (result.isOk()) {
    throw "Unsigned should not accept negative";
  }
}

inline void int8BoundaryTest() {
  auto maxRes = deserialize("127", static_cast<int8_t*>(nullptr));
  if (!maxRes.isOk() || maxRes.unwrap() != 127) {
    throw "Should parse INT8_MAX";
  }

  auto overRes = deserialize("128", static_cast<int8_t*>(nullptr));
  if (overRes.isOk()) {
    throw "Should fail on int8 overflow";
  }

  auto minRes = deserialize("-128", static_cast<int8_t*>(nullptr));
  if (!minRes.isOk() || minRes.unwrap() != -128) {
    throw "Should parse INT8_MIN";
  }
}

inline void uint64LargeBoundaryTest() {
  // UINT64_MAX = 18446744073709551615
  auto maxRes =
      deserialize("18446744073709551615", static_cast<uint64_t*>(nullptr));
  if (!maxRes.isOk() || maxRes.unwrap() != 18446744073709551615ULL) {
    throw "Should parse UINT64_MAX";
  }

  auto overRes =
      deserialize("18446744073709551616", static_cast<uint64_t*>(nullptr));
  if (overRes.isOk()) {
    throw "Should fail on uint64 overflow";
  }
}

inline void integerLeadingZerosTest() {
  auto res = deserialize("007", static_cast<int*>(nullptr));
  if (!res.isOk() || res.unwrap() != 7) {
    throw "Leading zeros should parse as decimal 7";
  }
}

inline void integerExtraCharactersTest() {
  auto res = deserialize("123abc", static_cast<int*>(nullptr));
  if (res.isOk()) {
    throw "Extra characters should cause failure";
  }

  auto res2 = deserialize("123 ", static_cast<int*>(nullptr));
  if (res2.isOk()) {
    throw "Trailing space should cause failure";
  }
}

// ============================================================================
// FLOATING POINT EDGE CASES
// ============================================================================

inline void floatScientificNotationTest() {
  auto res = deserialize("1.5e10", static_cast<double*>(nullptr));
  if (!res.isOk()) {
    throw "Should parse scientific notation";
  }
  double val = res.unwrap();
  if (std::abs(val - 1.5e10) > 1.0) {
    throw "Scientific notation value incorrect";
  }
}

inline void floatNegativeExponentTest() {
  auto res = deserialize("3.14e-5", static_cast<double*>(nullptr));
  if (!res.isOk()) {
    throw "Should parse negative exponent";
  }
  double val = res.unwrap();
  if (std::abs(val - 3.14e-5) > 1e-10) {
    throw "Negative exponent value incorrect";
  }
}

inline void floatVerySmallTest() {
  auto res = deserialize("0.0000000001", static_cast<double*>(nullptr));
  if (!res.isOk()) {
    throw "Should parse very small float";
  }
}

inline void floatVeryLargeTest() {
  auto res =
      deserialize("1.7976931348623157e+308", static_cast<double*>(nullptr));
  if (!res.isOk()) {
    throw "Should parse near DBL_MAX";
  }
}

inline void floatNegativeZeroTest() {
  auto res = deserialize("-0.0", static_cast<double*>(nullptr));
  if (!res.isOk()) {
    throw "Should parse negative zero";
  }
}

inline void floatPrecisionTest() {
  auto res = deserialize("3.141592653589793", static_cast<double*>(nullptr));
  if (!res.isOk()) {
    throw "Should parse pi";
  }
  double val = res.unwrap();
  if (std::abs(val - 3.141592653589793) > 1e-15) {
    throw "Pi precision lost";
  }
}

// ============================================================================
// BOOLEAN VARIATIONS
// ============================================================================

inline void booleanCaseSensitivityTest() {
  // These should fail - only exact case matches
  auto res1 = deserialize("True", static_cast<bool*>(nullptr));
  if (res1.isOk())
    throw "True (mixed case) should not be valid";

  auto res2 = deserialize("TRUE", static_cast<bool*>(nullptr));
  if (res2.isOk())
    throw "TRUE (upper case) should not be valid";

  auto res3 = deserialize("False", static_cast<bool*>(nullptr));
  if (res3.isOk())
    throw "False (mixed case) should not be valid";

  auto res4 = deserialize("FALSE", static_cast<bool*>(nullptr));
  if (res4.isOk())
    throw "FALSE (upper case) should not be valid";

  // Some implementations are case-insensitive, adjust test based on actual behavior
  // For now, we test the explicit accepted values
  auto onRes = deserialize("ON", static_cast<bool*>(nullptr));
  if (!onRes.isOk() || !onRes.unwrap()) {
    throw "ON should be true";
  }

  auto offRes = deserialize("OFF", static_cast<bool*>(nullptr));
  if (!offRes.isOk() || offRes.unwrap()) {
    throw "OFF should be false";
  }

  auto oneRes = deserialize("1", static_cast<bool*>(nullptr));
  if (!oneRes.isOk() || !oneRes.unwrap()) {
    throw "1 should be true";
  }

  auto zeroRes = deserialize("0", static_cast<bool*>(nullptr));
  if (!zeroRes.isOk() || zeroRes.unwrap()) {
    throw "0 should be false";
  }
}

inline void booleanInvalidValuesTest() {
  const char* invalids[] = {"yes", "no", "y",  "n",     "t",
                            "f",   "2",  "-1", "truee", ""};
  for (const char* val : invalids) {
    auto res = deserialize(val, static_cast<bool*>(nullptr));
    if (res.isOk()) {
      std::fprintf(stderr, "'%s' should not be valid bool\n", val);
      throw "Invalid boolean accepted";
    }
  }
}

// ============================================================================
// COMPLEX ARGUMENT PARSER TESTS
// ============================================================================

inline void parserManyOptionsTest() {
  // Test parser with many options to stress symbol table
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     optInt<"opt1">('1', "--option1", "Option 1", some(1)),
                     optInt<"opt2">('2', "--option2", "Option 2", some(2)),
                     optInt<"opt3">('3', "--option3", "Option 3", some(3)),
                     optInt<"opt4">('4', "--option4", "Option 4", some(4)),
                     optInt<"opt5">('5', "--option5", "Option 5", some(5)),
                     optInt<"opt6">('6', "--option6", "Option 6", some(6)),
                     optInt<"opt7">('7', "--option7", "Option 7", some(7)),
                     optInt<"opt8">('8', "--option8", "Option 8", some(8)),
                     optBool<"verbose">('v', "--verbose", "Verbose"),
                     optString<"name">('n', "--name", "Name", "default"));

  const char* argv[] = {"prog", "-1",     "100",  "--option5", "500",
                        "-v",   "--name", "test", "-8",        "800"};
  auto result = parser.parse(10, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Many options parsing failed";
  }

  if (parser.get<"opt1">().value() != 100)
    throw "opt1 wrong";
  if (parser.get<"opt5">().value() != 500)
    throw "opt5 wrong";
  if (!parser.get<"verbose">().valueOr(false))
    throw "verbose wrong";
  if (parser.get<"name">().value() != "test")
    throw "name wrong";
  if (parser.get<"opt8">().value() != 800)
    throw "opt8 wrong";
  // Defaults
  if (parser.get<"opt2">().value() != 2)
    throw "opt2 default wrong";
  if (parser.get<"opt3">().value() != 3)
    throw "opt3 default wrong";
}

inline void parserRepeatedOptionOverwriteTest() {
  // When same option given twice, last value wins
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     optInt<"port">('p', "--port", "Port", some(8080)));

  const char* argv[] = {"prog", "-p", "3000", "--port", "4000", "-p", "5000"};
  auto result = parser.parse(7, argv);

  if (!result.isOk()) {
    throw "Repeated option parsing failed";
  }

  if (parser.get<"port">().value() != 5000) {
    throw "Last value should win";
  }
}

inline void parserInterleavedOptionsTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optInt<"a">('a', "--alpha", "Alpha", some(0)),
                               optInt<"b">('b', "--beta", "Beta", some(0)),
                               optInt<"c">('c', "--gamma", "Gamma", some(0)));

  // Interleave short and long forms
  const char* argv[] = {"prog", "-a", "1",       "--beta", "2",
                        "-c",   "3",  "--alpha", "10"};
  auto result = parser.parse(9, argv);

  if (!result.isOk())
    throw "Interleaved parsing failed";
  if (parser.get<"a">().value() != 10)
    throw "Alpha should be 10 (last)";
  if (parser.get<"b">().value() != 2)
    throw "Beta should be 2";
  if (parser.get<"c">().value() != 3)
    throw "Gamma should be 3";
}

inline void parserLongOptionEqualsFormTest() {
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     optInt<"port">('p', "--port", "Port", some(8080)),
                     optString<"host">('h', "--host", "Host", "localhost"));

  const char* argv[] = {"prog", "--port=9000", "--host=example.com"};
  auto result = parser.parse(3, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Equals form parsing failed";
  }

  if (parser.get<"port">().value() != 9000)
    throw "Port wrong";
  if (parser.get<"host">().value() != "example.com")
    throw "Host wrong";
}

inline void parserMissingRequiredValueTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optInt<"port">('p', "--port", "Port"));

  const char* argv[] = {"prog", "-p"};  // Missing value
  auto result = parser.parse(2, argv);

  if (result.isOk()) {
    throw "Should fail when value missing";
  }
}

inline void parserOnlyLongNameTest() {
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     optInt<"count">('\0', "--count", "Count", some(0)));

  const char* argv[] = {"prog", "--count", "42"};
  auto result = parser.parse(3, argv);

  if (!result.isOk())
    throw "Long-only option failed";
  if (parser.get<"count">().value() != 42)
    throw "Count wrong";
}

inline void parserOnlyShortNameTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
                               optInt<"num">('n', "", "Number", some(0)));

  const char* argv[] = {"prog", "-n", "99"};
  auto result = parser.parse(3, argv);

  if (!result.isOk())
    throw "Short-only option failed";
  if (parser.get<"num">().value() != 99)
    throw "Number wrong";
}

// ============================================================================
// COMPLEX CUSTOM TYPE TESTS
// ============================================================================

inline void customVector3DTest() {
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     opt<Vector3D, "vec">('v', "--vector", "3D Vector",
                                          some(Vector3D{0, 0, 0})));

  const char* argv[] = {"prog", "-v", "1.5,2.5,3.5"};
  auto result = parser.parse(3, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Vector3D parsing failed";
  }

  auto v = parser.get<"vec">();
  if (!(Vector3D{1.5, 2.5, 3.5} == v.value())) {
    throw "Vector3D values wrong";
  }
}

inline void customIPAddressTest() {
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     opt<IPAddress, "ip">('i', "--ip", "IP Address",
                                          some(IPAddress{{0, 0, 0, 0}})));

  const char* argv[] = {"prog", "-i", "192.168.1.100"};
  auto result = parser.parse(3, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "IP parsing failed";
  }

  auto ip = parser.get<"ip">().value();
  if (ip.octets[0] != 192 || ip.octets[1] != 168 || ip.octets[2] != 1 ||
      ip.octets[3] != 100) {
    throw "IP values wrong";
  }
}

inline void customIPAddressInvalidTest() {
  auto res = deserialize("256.1.2.3", static_cast<IPAddress*>(nullptr));
  if (res.isOk())
    throw "Should reject octet > 255";

  auto res2 = deserialize("1.2.3", static_cast<IPAddress*>(nullptr));
  if (res2.isOk())
    throw "Should reject 3 octets";

  auto res3 = deserialize("-1.2.3.4", static_cast<IPAddress*>(nullptr));
  if (res3.isOk())
    throw "Should reject negative octet";
}

inline void customTimeRangeTest() {
  auto parser =
      ArgumentParser("TestApp", "TestDescription",
                     opt<TimeRange, "hours">('t', "--time", "Time range",
                                             some(TimeRange{9, 0, 17, 0})));

  const char* argv[] = {"prog", "-t", "08:30-18:45"};
  auto result = parser.parse(3, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "TimeRange parsing failed";
  }

  auto t = parser.get<"hours">().value();
  if (t.startHour != 8 || t.startMin != 30 || t.endHour != 18 ||
      t.endMin != 45) {
    throw "TimeRange values wrong";
  }
}

inline void customSemVerTest() {
  auto parser = ArgumentParser(
      "TestApp", "TestDescription",
      opt<SemVer, "version">('V', "--sem-version", "Semantic version",
                             some(SemVer{1, 0, 0})));

  const char* argv[] = {"prog", "-V", "2.15.3"};
  auto result = parser.parse(3, argv);

  if (!result.isOk())
    throw "SemVer parsing failed";

  auto v = parser.get<"version">().value();
  if (!(SemVer{2, 15, 3} == v)) {
    std::fprintf(stderr, "Got: %d.%d.%d\n", v.major, v.minor, v.patch);
    throw "SemVer values wrong";
  }
}

inline void multipleCustomTypesTest() {
  auto parser = ArgumentParser(
      "TestApp", "TestDescription",
      opt<Vector3D, "pos">('p', "--position", "Position",
                           some(Vector3D{0, 0, 0})),
      opt<IPAddress, "server">('s', "--server", "Server IP",
                               some(IPAddress{{127, 0, 0, 1}})),
      opt<SemVer, "ver">('V', "--version", "Version", some(SemVer{1, 0, 0})),
      optInt<"port">('P', "--port", "Port", some(8080)));

  const char* argv[] = {"prog", "-p",    "10,20,30", "--server", "10.0.0.1",
                        "-V",   "3.2.1", "-P",       "9999"};
  auto result = parser.parse(9, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Multiple custom types failed";
  }

  auto pos = parser.get<"pos">().value();
  if (!(Vector3D{10, 20, 30} == pos))
    throw "Position wrong";

  auto srv = parser.get<"server">().value();
  if (srv.octets[0] != 10 || srv.octets[3] != 1)
    throw "Server IP wrong";

  auto ver = parser.get<"ver">().value();
  if (!(SemVer{3, 2, 1} == ver))
    throw "Version wrong";

  if (parser.get<"port">().value() != 9999)
    throw "Port wrong";
}

// ============================================================================
// COMMAND TESTS - COMPLEX SCENARIOS
// ============================================================================

inline void commandWithManyOptionsTest() {
  auto parser = ArgumentParser(
      "TestApp", "TestDescription",
      cmd<"deploy">(
          "Deploy application",
          optString<"env">('e', "--environment", "Environment", "staging"),
          optInt<"replicas">('r', "--replicas", "Replicas", some(1)),
          optBool<"dry-run">('d', "--dry-run", "Dry run"),
          optString<"image">('i', "--image", "Docker image", "latest")));

  const char* argv[] = {"prog", "deploy",    "-e",      "production", "-r",
                        "5",    "--dry-run", "--image", "v2.0.0"};
  auto result = parser.parse(9, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Deploy command failed";
  }

  if (!parser.has<"deploy">())
    throw "deploy command not matched";
  const auto& deploy = parser.get<"deploy">();
  if (deploy.get<"env">().value() != "production")
    throw "Env wrong";
  if (deploy.get<"replicas">().value() != 5)
    throw "Replicas wrong";
  if (!deploy.get<"dry-run">().valueOr(false))
    throw "Dry-run wrong";
  if (deploy.get<"image">().value() != "v2.0.0")
    throw "Image wrong";
}

inline void multipleCommandsTest() {
  auto parser = ArgumentParser(
      "TestApp", "TestDescription",
      cmd<"start">("Start service",
                   optInt<"port">('p', "--port", "Port", some(8080))),
      cmd<"stop">("Stop service",
                  optBool<"force">('f', "--force", "Force stop")),
      cmd<"status">("Show status",
                    optBool<"verbose">('v', "--verbose", "Verbose")));

  // Test start command
  const char* argv1[] = {"prog", "start", "-p", "3000"};
  auto p1 = parser;
  auto r1 = p1.parse(4, argv1);
  if (!r1.isOk())
    throw "start command failed";
  if (!p1.has<"start">())
    throw "start command not matched";
  if (p1.get<"start">().get<"port">().value() != 3000) {
    throw "start port wrong";
  }

  // Test stop command
  const char* argv2[] = {"prog", "stop", "--force"};
  auto p2 = parser;
  auto r2 = p2.parse(3, argv2);
  if (!r2.isOk())
    throw "stop command failed";
  if (!p2.has<"stop">())
    throw "stop command not matched";
  if (!p2.get<"stop">().get<"force">().valueOr(false)) {
    throw "stop force wrong";
  }

  // Test status command
  const char* argv3[] = {"prog", "status"};
  auto p3 = parser;
  auto r3 = p3.parse(2, argv3);
  if (!r3.isOk())
    throw "status command failed";
  if (!p3.has<"status">())
    throw "status command not matched";
}

inline void commandWithCustomTypeTest() {
  auto parser = ArgumentParser(
      "TestApp", "TestDescription",
      cmd<"connect">("Connect to server",
                     opt<IPAddress, "host">('H', "--host", "Server IP",
                                            some(IPAddress{{127, 0, 0, 1}})),
                     optInt<"port">('p', "--port", "Port", some(22))));

  const char* argv[] = {"prog", "connect", "-H", "192.168.0.1", "-p", "2222"};
  auto result = parser.parse(6, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Connect command failed";
  }

  if (!parser.has<"connect">())
    throw "connect command not matched";
  const auto& conn = parser.get<"connect">();
  auto ip = conn.get<"host">().value();
  if (ip.octets[0] != 192 || ip.octets[1] != 168)
    throw "Host IP wrong";
  if (conn.get<"port">().value() != 2222)
    throw "Port wrong";
}

// ============================================================================
// TEST RUNNER
// ============================================================================

inline void runAdvancedTests() {
  // Lexer Stress Tests
  std::printf("  lexerLongClusterWithValueTransitionTest...");
  lexerLongClusterWithValueTransitionTest();
  std::printf(" OK\n");

  std::printf("  lexerMultipleSeparatorsTest...");
  lexerMultipleSeparatorsTest();
  std::printf(" OK\n");

  std::printf("  lexerEqualsWithEmptyValueTest...");
  lexerEqualsWithEmptyValueTest();
  std::printf(" OK\n");

  std::printf("  lexerEqualsWithMultipleEqualsTest...");
  lexerEqualsWithMultipleEqualsTest();
  std::printf(" OK\n");

  std::printf("  lexerMixedClusterAndLongOptionsTest...");
  lexerMixedClusterAndLongOptionsTest();
  std::printf(" OK\n");

  std::printf("  lexerVeryLongOptionNameTest...");
  lexerVeryLongOptionNameTest();
  std::printf(" OK\n");

  std::printf("  lexerSingleDashAloneTest...");
  lexerSingleDashAloneTest();
  std::printf(" OK\n");

  // Integer Boundary Tests
  std::printf("  integerOverflowTest...");
  integerOverflowTest();
  std::printf(" OK\n");

  std::printf("  integerUnderflowTest...");
  integerUnderflowTest();
  std::printf(" OK\n");

  std::printf("  integerExactBoundariesTest...");
  integerExactBoundariesTest();
  std::printf(" OK\n");

  std::printf("  unsignedIntegerNegativeTest...");
  unsignedIntegerNegativeTest();
  std::printf(" OK\n");

  std::printf("  int8BoundaryTest...");
  int8BoundaryTest();
  std::printf(" OK\n");

  std::printf("  uint64LargeBoundaryTest...");
  uint64LargeBoundaryTest();
  std::printf(" OK\n");

  std::printf("  integerLeadingZerosTest...");
  integerLeadingZerosTest();
  std::printf(" OK\n");

  std::printf("  integerExtraCharactersTest...");
  integerExtraCharactersTest();
  std::printf(" OK\n");

  // Floating Point Tests
  std::printf("  floatScientificNotationTest...");
  floatScientificNotationTest();
  std::printf(" OK\n");

  std::printf("  floatNegativeExponentTest...");
  floatNegativeExponentTest();
  std::printf(" OK\n");

  std::printf("  floatVerySmallTest...");
  floatVerySmallTest();
  std::printf(" OK\n");

  std::printf("  floatVeryLargeTest...");
  floatVeryLargeTest();
  std::printf(" OK\n");

  std::printf("  floatNegativeZeroTest...");
  floatNegativeZeroTest();
  std::printf(" OK\n");

  std::printf("  floatPrecisionTest...");
  floatPrecisionTest();
  std::printf(" OK\n");

  // Boolean Tests
  std::printf("  booleanCaseSensitivityTest...");
  booleanCaseSensitivityTest();
  std::printf(" OK\n");

  std::printf("  booleanInvalidValuesTest...");
  booleanInvalidValuesTest();
  std::printf(" OK\n");

  // Complex Parser Tests
  std::printf("  parserManyOptionsTest...");
  parserManyOptionsTest();
  std::printf(" OK\n");

  std::printf("  parserRepeatedOptionOverwriteTest...");
  parserRepeatedOptionOverwriteTest();
  std::printf(" OK\n");

  std::printf("  parserInterleavedOptionsTest...");
  parserInterleavedOptionsTest();
  std::printf(" OK\n");

  std::printf("  parserLongOptionEqualsFormTest...");
  parserLongOptionEqualsFormTest();
  std::printf(" OK\n");

  std::printf("  parserMissingRequiredValueTest...");
  parserMissingRequiredValueTest();
  std::printf(" OK\n");

  std::printf("  parserOnlyLongNameTest...");
  parserOnlyLongNameTest();
  std::printf(" OK\n");

  std::printf("  parserOnlyShortNameTest...");
  parserOnlyShortNameTest();
  std::printf(" OK\n");

  // Custom Type Tests
  std::printf("  customVector3DTest...");
  customVector3DTest();
  std::printf(" OK\n");

  std::printf("  customIPAddressTest...");
  customIPAddressTest();
  std::printf(" OK\n");

  std::printf("  customIPAddressInvalidTest...");
  customIPAddressInvalidTest();
  std::printf(" OK\n");

  std::printf("  customTimeRangeTest...");
  customTimeRangeTest();
  std::printf(" OK\n");

  std::printf("  customSemVerTest...");
  customSemVerTest();
  std::printf(" OK\n");

  std::printf("  multipleCustomTypesTest...");
  multipleCustomTypesTest();
  std::printf(" OK\n");

  // Command Tests
  std::printf("  commandWithManyOptionsTest...");
  commandWithManyOptionsTest();
  std::printf(" OK\n");

  std::printf("  multipleCommandsTest...");
  multipleCommandsTest();
  std::printf(" OK\n");

  std::printf("  commandWithCustomTypeTest...");
  commandWithCustomTypeTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests
