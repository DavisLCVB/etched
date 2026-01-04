#pragma once
#ifndef ETCHED_LIB_ETCHED_PARSER_TESTS_HPP
#define ETCHED_LIB_ETCHED_PARSER_TESTS_HPP

#include <etched/etched.hpp>

namespace etched::tests {

auto fromStrSignedIntTest() -> void {
  {
    auto val = fromStr<int>("42");
    if (val != 42) {
      throw "fromStr<int> failed for positive value";
    }
  }
  {
    auto val = fromStr<int>("-42");
    if (val != -42) {
      throw "fromStr<int> failed for negative value";
    }
  }
  {
    auto val = fromStr<int8_t>("127");
    if (val != 127) {
      throw "fromStr<int8_t> failed for max value";
    }
  }
  {
    auto val = fromStr<int8_t>("-128");
    if (val != -128) {
      throw "fromStr<int8_t> failed for min value";
    }
  }
  {
    auto val = fromStr<int16_t>("32767");
    if (val != 32767) {
      throw "fromStr<int16_t> failed for max value";
    }
  }
  {
    auto val = fromStr<int32_t>("-2147483648");
    if (val != -2147483648) {
      throw "fromStr<int32_t> failed for min value";
    }
  }
  {
    auto val = fromStr<int64_t>("9223372036854775807");
    if (val != 9223372036854775807LL) {
      throw "fromStr<int64_t> failed for max value";
    }
  }
}

auto fromStrUnsignedIntTest() -> void {
  {
    auto val = fromStr<unsigned int>("42");
    if (val != 42u) {
      throw "fromStr<unsigned int> failed for positive value";
    }
  }
  {
    auto val = fromStr<uint8_t>("255");
    if (val != 255u) {
      throw "fromStr<uint8_t> failed for max value";
    }
  }
  {
    auto val = fromStr<uint16_t>("65535");
    if (val != 65535u) {
      throw "fromStr<uint16_t> failed for max value";
    }
  }
  {
    auto val = fromStr<uint32_t>("4294967295");
    if (val != 4294967295u) {
      throw "fromStr<uint32_t> failed for max value";
    }
  }
  {
    auto val = fromStr<uint64_t>("18446744073709551615");
    if (val != 18446744073709551615ULL) {
      throw "fromStr<uint64_t> failed for max value";
    }
  }
}

auto fromStrFloatTest() -> void {
  {
    auto val = fromStr<float>("3.14");
    if (val < 3.13f || val > 3.15f) {
      throw "fromStr<float> failed for positive value";
    }
  }
  {
    auto val = fromStr<float>("-2.5");
    if (val > -2.4f || val < -2.6f) {
      throw "fromStr<float> failed for negative value";
    }
  }
  {
    auto val = fromStr<float>("0.0");
    if (val != 0.0f) {
      throw "fromStr<float> failed for zero";
    }
  }
}

auto fromStrDoubleTest() -> void {
  {
    auto val = fromStr<double>("3.14159");
    if (val < 3.14158 || val > 3.14160) {
      throw "fromStr<double> failed for positive value";
    }
  }
  {
    auto val = fromStr<double>("-2.71828");
    if (val > -2.71827 || val < -2.71829) {
      throw "fromStr<double> failed for negative value";
    }
  }
  {
    auto val = fromStr<double>("0.0");
    if (val != 0.0) {
      throw "fromStr<double> failed for zero";
    }
  }
}

auto fromStrCStringTest() -> void {
  {
    auto val = fromStr<const char*>("hello");
    if (val[0] != 'h' || val[1] != 'e' || val[2] != 'l' || val[3] != 'l' ||
        val[4] != 'o') {
      throw "fromStr<const char*> failed";
    }
  }
}

auto stringTest() -> void {
  {
    detail::String str1("hello");
    detail::String str2("hello");
    if (!(str1 == str2)) {
      throw "String equality failed for same strings";
    }
  }
  {
    detail::String str1("hello");
    detail::String str2("world");
    if (str1 == str2) {
      throw "String equality failed for different strings with same length";
    }
  }
  {
    detail::String str1("hello");
    detail::String str2("hi");
    if (str1 == str2) {
      throw "String equality failed for different length strings";
    }
  }
  {
    detail::String str("test");
    if (str.data[0] != 't' || str.data[1] != 'e' || str.data[2] != 's' ||
        str.data[3] != 't' || str.data[4] != '\0') {
      throw "String data storage failed";
    }
  }
  {
    detail::String str("hello");
    if (!(str == "hello")) {
      throw "String comparison with literal failed";
    }
  }
  {
    if (!("hello" == detail::String("hello"))) {
      throw "Literal comparison with String failed";
    }
  }
}

auto optionTest() -> void {
  {
    auto option = opt<int, "port">("-p", "--port", "Port number", 8080);
    const char* shortName = option.shortName.value();
    if (shortName[0] != '-' || shortName[1] != 'p') {
      throw "Option shortName failed";
    }
    const char* longName = option.longName.value();
    if (longName[0] != '-' || longName[1] != '-' || longName[2] != 'p') {
      throw "Option longName failed";
    }
    if (!option.defaultValue.has_value() ||
        option.defaultValue.value() != 8080) {
      throw "Option defaultValue failed";
    }
  }
  {
    auto option = opt<const char*, "name">("-n", "--name", "User name");
    if (!option.shortName || !option.longName || !option.description) {
      throw "Option initialization failed";
    }
    if (option.defaultValue.has_value()) {
      throw "Option without default value should not have value";
    }
  }
}

auto argumentParserBasicTest() -> void {
  constexpr auto parser = ArgumentParser(
      opt<int, "port">("-p", "--port", "Port number", 8080),
      opt<const char*, "host">("-h", "--host", "Host address", "localhost"));

  const char* argv[] = {"program", "-p", "3000", "--host", "127.0.0.1"};
  auto mutableParser = parser;
  mutableParser.parse(5, argv);

  auto& portOpt = mutableParser.getOption<"port">();
  if (portOpt.value != 3000) {
    throw "ArgumentParser failed to parse -p flag";
  }

  auto& hostOpt = mutableParser.getOption<"host">();
  const char* expectedHost = "127.0.0.1";
  const char* actualHost = hostOpt.value.value();
  bool hostsMatch = true;
  for (int i = 0; expectedHost[i] != '\0'; ++i) {
    if (actualHost[i] != expectedHost[i]) {
      hostsMatch = false;
      break;
    }
  }
  if (!hostsMatch) {
    throw "ArgumentParser failed to parse --host flag";
  }
}

auto argumentParserDefaultValuesTest() -> void {
  constexpr auto parser =
      ArgumentParser(opt<int, "port">("-p", "--port", "Port number", 8080),
                     opt<int, "timeout">("-t", "--timeout", "Timeout", 30));

  const char* argv[] = {"program"};
  auto mutableParser = parser;
  mutableParser.parse(1, argv);

  auto& portOpt = mutableParser.getOption<"port">();
  if (portOpt.value != 8080) {
    throw "ArgumentParser failed to use default value for port";
  }

  auto& timeoutOpt = mutableParser.getOption<"timeout">();
  if (timeoutOpt.value != 30) {
    throw "ArgumentParser failed to use default value for timeout";
  }
}

auto argumentParserLongFlagsTest() -> void {
  constexpr auto parser = ArgumentParser(
      opt<int, "verbose">("-v", "--verbose", "Verbosity level", 0),
      opt<const char*, "output">("-o", "--output", "Output file"));

  const char* argv[] = {"program", "--verbose", "2", "--output", "result.txt"};
  auto mutableParser = parser;
  mutableParser.parse(5, argv);

  auto& verboseOpt = mutableParser.getOption<"verbose">();
  if (verboseOpt.value != 2) {
    throw "ArgumentParser failed to parse --verbose long flag";
  }

  auto& outputOpt = mutableParser.getOption<"output">();
  const char* expectedOutput = "result.txt";
  bool outputsMatch = true;
  for (int i = 0; expectedOutput[i] != '\0'; ++i) {
    if (outputOpt.value.value()[i] != expectedOutput[i]) {
      outputsMatch = false;
      break;
    }
  }
  if (!outputsMatch) {
    throw "ArgumentParser failed to parse --output long flag";
  }
}

auto argumentParserShortFlagsTest() -> void {
  constexpr auto parser =
      ArgumentParser(opt<int, "count">("-c", "--count", "Count value"),
                     opt<const char*, "file">("-f", "--file", "File path"));

  const char* argv[] = {"program", "-c", "42", "-f", "data.txt"};
  auto mutableParser = parser;
  mutableParser.parse(5, argv);

  auto& countOpt = mutableParser.getOption<"count">();
  if (countOpt.value != 42) {
    throw "ArgumentParser failed to parse -c short flag";
  }

  auto& fileOpt = mutableParser.getOption<"file">();
  const char* expectedFile = "data.txt";
  bool filesMatch = true;
  for (int i = 0; expectedFile[i] != '\0'; ++i) {
    if (fileOpt.value.value()[i] != expectedFile[i]) {
      filesMatch = false;
      break;
    }
  }
  if (!filesMatch) {
    throw "ArgumentParser failed to parse -f short flag";
  }
}

auto argumentParserMixedFlagsTest() -> void {
  constexpr auto parser = ArgumentParser(
      opt<int, "port">("-p", "--port", "Port number", 8080),
      opt<const char*, "host">("-h", "--host", "Host address", "localhost"),
      opt<int, "workers">("-w", "--workers", "Worker count", 4));

  const char* argv[] = {"program",     "-p", "3000", "--host",
                        "192.168.1.1", "-w", "8"};
  auto mutableParser = parser;
  mutableParser.parse(7, argv);

  auto& portOpt = mutableParser.getOption<"port">();
  if (portOpt.value != 3000) {
    throw "ArgumentParser failed with mixed flags for port";
  }

  auto& hostOpt = mutableParser.getOption<"host">();
  const char* expectedHost = "192.168.1.1";
  bool hostsMatch = true;
  for (int i = 0; expectedHost[i] != '\0'; ++i) {
    if (hostOpt.value.value()[i] != expectedHost[i]) {
      hostsMatch = false;
      break;
    }
  }
  if (!hostsMatch) {
    throw "ArgumentParser failed with mixed flags for host";
  }

  auto& workersOpt = mutableParser.getOption<"workers">();
  if (workersOpt.value != 8) {
    throw "ArgumentParser failed with mixed flags for workers";
  }
}

auto argumentParserPartialArgumentsTest() -> void {
  constexpr auto parser = ArgumentParser(
      opt<int, "port">("-p", "--port", "Port number", 8080),
      opt<const char*, "host">("-h", "--host", "Host address", "localhost"),
      opt<int, "timeout">("-t", "--timeout", "Timeout", 30));

  const char* argv[] = {"program", "-p", "5000"};
  auto mutableParser = parser;
  mutableParser.parse(3, argv);

  auto& portOpt = mutableParser.getOption<"port">();
  if (portOpt.value != 5000) {
    throw "ArgumentParser failed to parse provided argument";
  }

  auto& hostOpt = mutableParser.getOption<"host">();
  const char* expectedHost = "localhost";
  bool hostsMatch = true;
  for (int i = 0; expectedHost[i] != '\0'; ++i) {
    if (hostOpt.value.value()[i] != expectedHost[i]) {
      hostsMatch = false;
      break;
    }
  }
  if (!hostsMatch) {
    throw "ArgumentParser failed to keep default for unprovided argument";
  }
}

auto argumentParserMultipleTypesTest() -> void {
  constexpr auto parser = ArgumentParser(
      opt<int8_t, "byte">("-b", "--byte", "Byte value"),
      opt<uint16_t, "ushort">("-u", "--ushort", "Unsigned short"),
      opt<int64_t, "long">("-l", "--long", "Long value"),
      opt<float, "ratio">("-r", "--ratio", "Ratio value"),
      opt<double, "pi">("-d", "--pi", "Pi value"));

  const char* argv[] = {
      "program", "-b",  "127", "-u",     "65535", "-l", "9223372036854775807",
      "-r",      "0.5", "-d",  "3.14159"};
  auto mutableParser = parser;
  mutableParser.parse(11, argv);

  auto& byteOpt = mutableParser.getOption<"byte">();
  if (byteOpt.value != 127) {
    throw "ArgumentParser failed for int8_t";
  }

  auto& ushortOpt = mutableParser.getOption<"ushort">();
  if (ushortOpt.value != 65535) {
    throw "ArgumentParser failed for uint16_t";
  }

  auto& longOpt = mutableParser.getOption<"long">();
  if (longOpt.value != 9223372036854775807LL) {
    throw "ArgumentParser failed for int64_t";
  }

  auto& ratioOpt = mutableParser.getOption<"ratio">();
  if (ratioOpt.value < 0.49f || ratioOpt.value > 0.51f) {
    throw "ArgumentParser failed for float";
  }

  auto& piOpt = mutableParser.getOption<"pi">();
  if (piOpt.value < 3.14 || piOpt.value > 3.15) {
    throw "ArgumentParser failed for double";
  }
}

auto errorHandlingIntTest() -> void {
  {
    bool caught = false;
    try {
      fromStr<int8_t>("128");
    } catch (const std::out_of_range&) {
      caught = true;
    }
    if (!caught) {
      throw "int8_t overflow not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<int8_t>("-129");
    } catch (const std::out_of_range&) {
      caught = true;
    }
    if (!caught) {
      throw "int8_t underflow not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<uint8_t>("256");
    } catch (const std::out_of_range&) {
      caught = true;
    }
    if (!caught) {
      throw "uint8_t overflow not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<int>("abc");
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Invalid int string not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<int>(nullptr);
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "nullptr int conversion not detected";
    }
  }
}

auto errorHandlingFloatTest() -> void {
  {
    bool caught = false;
    try {
      fromStr<double>("invalid");
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Invalid double string not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<double>(nullptr);
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "nullptr double conversion not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<float>(nullptr);
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "nullptr float conversion not detected";
    }
  }
}

auto errorHandlingCharTest() -> void {
  {
    bool caught = false;
    try {
      fromStr<char>("ab");
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Multi-char string not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<char>("");
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Empty char string not detected";
    }
  }
}

auto sanitizerTest() -> void {
  {
    if (!detail::BasicSanitizer::isValid("test")) {
      throw "Valid arg rejected";
    }
  }
  {
    if (!detail::BasicSanitizer::isValid("-p")) {
      throw "Valid short flag rejected";
    }
  }
  {
    if (!detail::BasicSanitizer::isValid("--port")) {
      throw "Valid long flag rejected";
    }
  }
  {
    if (!detail::BasicSanitizer::isValid("hello world")) {
      throw "String with spaces rejected";
    }
  }
  {
    if (!detail::BasicSanitizer::isValid("path\twith\ttabs")) {
      throw "String with tabs rejected";
    }
  }
  {
    if (detail::BasicSanitizer::isValid(nullptr)) {
      throw "nullptr not rejected";
    }
  }
  {
    if (detail::BasicSanitizer::isValid("")) {
      throw "Empty string not rejected";
    }
  }
  {
    if (detail::BasicSanitizer::isValid("\x01invalid")) {
      throw "Control character not rejected";
    }
  }
}

auto boolFlagTest() -> void {
  {
    constexpr auto parser = ArgumentParser(
        optBool<"verbose">("-v", "--verbose", "Verbose output"));
    const char* argv[] = {"program", "-v"};
    auto mutableParser = parser;
    mutableParser.parse(2, argv);
    auto& opt = mutableParser.getOption<"verbose">();
    if (!opt.value.value()) {
      throw "Boolean flag not set";
    }
  }
  {
    constexpr auto parser = ArgumentParser(
        optBool<"debug">("-d", "--debug", "Debug mode", false));
    const char* argv[] = {"program"};
    auto mutableParser = parser;
    mutableParser.parse(1, argv);
    auto& opt = mutableParser.getOption<"debug">();
    if (opt.value.value()) {
      throw "Boolean default value incorrect";
    }
  }
}

auto unknownOptionTest() -> void {
  bool caught = false;
  try {
    constexpr auto parser =
        ArgumentParser(optInt<"port">("-p", "--port", "Port"));
    const char* argv[] = {"program", "-x"};
    auto mutableParser = parser;
    mutableParser.parse(2, argv);
  } catch (const std::invalid_argument&) {
    caught = true;
  }
  if (!caught) {
    throw "Unknown option not detected";
  }
}

auto missingValueTest() -> void {
  bool caught = false;
  try {
    constexpr auto parser =
        ArgumentParser(optInt<"port">("-p", "--port", "Port"));
    const char* argv[] = {"program", "-p"};
    auto mutableParser = parser;
    mutableParser.parse(2, argv);
  } catch (const std::invalid_argument&) {
    caught = true;
  }
  if (!caught) {
    throw "Missing value not detected";
  }
}

auto positionalArgTest() -> void {
  bool caught = false;
  try {
    constexpr auto parser =
        ArgumentParser(optInt<"port">("-p", "--port", "Port"));
    const char* argv[] = {"program", "positional"};
    auto mutableParser = parser;
    mutableParser.parse(2, argv);
  } catch (const std::invalid_argument&) {
    caught = true;
  }
  if (!caught) {
    throw "Positional arg not detected";
  }
}

auto maxArgumentsTest() -> void {
  bool caught = false;
  try {
    constexpr auto parser =
        ArgumentParser(optInt<"port">("-p", "--port", "Port"));
    std::array<const char*, 300> argv;
    argv[0] = "program";
    for (int i = 1; i < 300; i++) {
      argv[i] = "-p";
    }
    auto mutableParser = parser;
    mutableParser.parse(300, argv.data());
  } catch (const std::invalid_argument&) {
    caught = true;
  }
  if (!caught) {
    throw "Too many args not detected";
  }
}

int globalCallbackCount = 0;
void testCallback() { globalCallbackCount++; }

auto callbackTest() -> void {
  {
    globalCallbackCount = 0;
    auto parser = ArgumentParser(
        optCallback<"test">("-t", "--test", "Test", testCallback),
        optInt<"port">("-p", "--port", "Port", 8080));
    const char* argv[] = {"program", "-t", "-p", "3000"};
    parser.parse(4, argv);
    if (globalCallbackCount != 1) {
      throw "Callback not executed";
    }
    if (parser.getOption<"port">().value.value() != 3000) {
      throw "Parsing continued incorrectly after callback";
    }
  }
}

auto stringWithSpacesTest() -> void {
  {
    constexpr auto parser =
        ArgumentParser(optString<"path">("-p", "--path", "Path"));
    const char* argv[] = {"program", "-p", "/path/with spaces/file.txt"};
    auto mutableParser = parser;
    mutableParser.parse(3, argv);
    auto& opt = mutableParser.getOption<"path">();
    const char* expected = "/path/with spaces/file.txt";
    bool match = true;
    for (int i = 0; expected[i] != '\0'; ++i) {
      if (opt.value.value()[i] != expected[i]) {
        match = false;
        break;
      }
    }
    if (!match) {
      throw "String with spaces not parsed correctly";
    }
  }
}

auto terminalOptionTest() -> void {
  bool caught = false;
  try {
    constexpr auto parser = ArgumentParser(
        optInt<"port">("-p", "--port", "Port", 8080), optHelp("-h", "--help"));
    const char* argv[] = {"program", "--help", "-p", "3000"};
    auto mutableParser = parser;
    mutableParser.parse(4, argv);
  } catch (const std::invalid_argument&) {
    caught = true;
  }
  if (!caught) {
    throw "Arguments after terminal option not detected";
  }
}

struct TestPoint {
  double x;
  double y;
};

struct TestColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

}  // namespace etched::tests

namespace etched {

template <>
auto fromStr<tests::TestPoint>(const char* str) -> tests::TestPoint {
  if (!str) {
    throw std::invalid_argument("Null pointer for TestPoint");
  }
  std::string input(str);
  size_t comma = input.find(',');
  if (comma == std::string::npos) {
    throw std::invalid_argument("TestPoint format: x,y");
  }
  return tests::TestPoint{
    std::stod(input.substr(0, comma)),
    std::stod(input.substr(comma + 1))
  };
}

template <>
auto fromStr<tests::TestColor>(const char* str) -> tests::TestColor {
  if (!str) {
    throw std::invalid_argument("Null pointer for TestColor");
  }
  std::string input(str);
  if (input[0] == '#') input = input.substr(1);
  if (input.length() != 6) {
    throw std::invalid_argument("TestColor format: #RRGGBB");
  }
  auto parseHex = [](const std::string& hex) -> uint8_t {
    return static_cast<uint8_t>(std::stoi(hex, nullptr, 16));
  };
  return tests::TestColor{
    parseHex(input.substr(0, 2)),
    parseHex(input.substr(2, 2)),
    parseHex(input.substr(4, 2))
  };
}

}  // namespace etched

namespace etched::tests {

auto customTypePointTest() -> void {
  {
    auto point = fromStr<TestPoint>("3.14,2.71");
    if (point.x < 3.13 || point.x > 3.15) {
      throw "Point x parsing failed";
    }
    if (point.y < 2.70 || point.y > 2.72) {
      throw "Point y parsing failed";
    }
  }
  {
    auto point = fromStr<TestPoint>("0,0");
    if (point.x != 0.0 || point.y != 0.0) {
      throw "Point zero parsing failed";
    }
  }
  {
    auto point = fromStr<TestPoint>("-10.5,20.3");
    if (point.x > -10.4 || point.x < -10.6) {
      throw "Point negative x failed";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<TestPoint>("invalid");
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Invalid point format not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<TestPoint>(nullptr);
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Null point not detected";
    }
  }
}

auto customTypeColorTest() -> void {
  {
    auto color = fromStr<TestColor>("#FF0000");
    if (color.r != 255 || color.g != 0 || color.b != 0) {
      throw "Color red parsing failed";
    }
  }
  {
    auto color = fromStr<TestColor>("00FF00");
    if (color.r != 0 || color.g != 255 || color.b != 0) {
      throw "Color green without # failed";
    }
  }
  {
    auto color = fromStr<TestColor>("#0000FF");
    if (color.r != 0 || color.g != 0 || color.b != 255) {
      throw "Color blue parsing failed";
    }
  }
  {
    auto color = fromStr<TestColor>("#FFFFFF");
    if (color.r != 255 || color.g != 255 || color.b != 255) {
      throw "Color white parsing failed";
    }
  }
  {
    auto color = fromStr<TestColor>("#123456");
    if (color.r != 0x12 || color.g != 0x34 || color.b != 0x56) {
      throw "Color hex parsing failed";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<TestColor>("#FFF");
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Invalid color length not detected";
    }
  }
  {
    bool caught = false;
    try {
      fromStr<TestColor>(nullptr);
    } catch (const std::invalid_argument&) {
      caught = true;
    }
    if (!caught) {
      throw "Null color not detected";
    }
  }
}

auto customTypeParserTest() -> void {
  {
    constexpr auto parser = ArgumentParser(
        opt<TestPoint, "position">("-p", "--position", "Position", TestPoint{0, 0}),
        opt<TestColor, "color">("-c", "--color", "Color", TestColor{0, 0, 0}));

    const char* argv[] = {"program", "-p", "10.5,20.3", "-c", "#FF5733"};
    auto mutableParser = parser;
    mutableParser.parse(5, argv);

    auto pos = mutableParser.getOption<"position">().value.value();
    if (pos.x < 10.4 || pos.x > 10.6) {
      throw "Parser position x failed";
    }
    if (pos.y < 20.2 || pos.y > 20.4) {
      throw "Parser position y failed";
    }

    auto color = mutableParser.getOption<"color">().value.value();
    if (color.r != 0xFF || color.g != 0x57 || color.b != 0x33) {
      throw "Parser color failed";
    }
  }
  {
    constexpr auto parser = ArgumentParser(
        opt<TestPoint, "pos">("-p", "--pos", "Pos", TestPoint{1.0, 2.0}));

    const char* argv[] = {"program"};
    auto mutableParser = parser;
    mutableParser.parse(1, argv);

    auto pos = mutableParser.getOption<"pos">().value.value();
    if (pos.x != 1.0 || pos.y != 2.0) {
      throw "Custom type default value failed";
    }
  }
}

auto parserTests() -> void {
  fromStrSignedIntTest();
  fromStrUnsignedIntTest();
  fromStrFloatTest();
  fromStrDoubleTest();
  fromStrCStringTest();
  stringTest();
  optionTest();
  argumentParserBasicTest();
  argumentParserDefaultValuesTest();
  argumentParserLongFlagsTest();
  argumentParserShortFlagsTest();
  argumentParserMixedFlagsTest();
  argumentParserPartialArgumentsTest();
  argumentParserMultipleTypesTest();
  errorHandlingIntTest();
  errorHandlingFloatTest();
  errorHandlingCharTest();
  sanitizerTest();
  boolFlagTest();
  unknownOptionTest();
  missingValueTest();
  positionalArgTest();
  maxArgumentsTest();
  callbackTest();
  stringWithSpacesTest();
  terminalOptionTest();
  customTypePointTest();
  customTypeColorTest();
  customTypeParserTest();
}

}  // namespace etched::tests

#endif
