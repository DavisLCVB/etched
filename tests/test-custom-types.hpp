#pragma once
#include <etched/etched.hpp>
#include <cstdio>
#include <cstring>

namespace etched::tests {

struct Point {
  double x;
  double y;
};

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

}  // namespace etched::tests

namespace etched {

template <>
inline auto deserialize(std::string_view str, tests::Point*) -> Expected<tests::Point, RuntimeError> {
  auto comma = str.find(',');
  if (comma == std::string_view::npos) {
    return etched::err(RuntimeError{"Point format: x,y"});
  }
  auto xStr = str.substr(0, comma);
  auto yStr = str.substr(comma + 1);

  double x = 0, y = 0;
  auto xResult = deserialize(xStr, static_cast<double*>(nullptr));
  if (!xResult.has_value()) return etched::err(RuntimeError{"Invalid x coordinate"});
  x = xResult.value();

  auto yResult = deserialize(yStr, static_cast<double*>(nullptr));
  if (!yResult.has_value()) return etched::err(RuntimeError{"Invalid y coordinate"});
  y = yResult.value();

  return tests::Point{x, y};
}

template <>
inline auto deserialize(std::string_view str, tests::Color*) -> Expected<tests::Color, RuntimeError> {
  if (str.empty()) {
    return etched::err(RuntimeError{"Empty color string"});
  }
  if (str[0] == '#') {
    str = str.substr(1);
  }
  if (str.size() != 6) {
    return etched::err(RuntimeError{"Color format: #RRGGBB or RRGGBB"});
  }

  auto parseHex = [](std::string_view hex) -> uint8_t {
    uint8_t result = 0;
    for (char c : hex) {
      result *= 16;
      if (c >= '0' && c <= '9') result += static_cast<uint8_t>(c - '0');
      else if (c >= 'a' && c <= 'f') result += static_cast<uint8_t>(c - 'a' + 10);
      else if (c >= 'A' && c <= 'F') result += static_cast<uint8_t>(c - 'A' + 10);
    }
    return result;
  };

  return tests::Color{
      parseHex(str.substr(0, 2)),
      parseHex(str.substr(2, 2)),
      parseHex(str.substr(4, 2))};
}

}  // namespace etched

namespace etched::tests {

inline void customTypePointTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      opt<Point, "pos">('p', "--position", "Position", some(Point{0.0, 0.0})));

  const char* argv[] = {"prog", "-p", "10.5,20.3"};
  auto result = parser.parse(3, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Point parsing should succeed";
  }

  Point p = parser.get<"pos">().value();
  if (p.x < 10.4 || p.x > 10.6) {
    throw "Point x should be ~10.5";
  }
  if (p.y < 20.2 || p.y > 20.4) {
    throw "Point y should be ~20.3";
  }
}

inline void customTypeColorTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      opt<Color, "color">('c', "--color", "Color", some(Color{0, 0, 0})));

  const char* argv[] = {"prog", "-c", "#FF5733"};
  auto result = parser.parse(3, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Color parsing should succeed";
  }

  Color c = parser.get<"color">().value();
  if (c.r != 0xFF || c.g != 0x57 || c.b != 0x33) {
    std::fprintf(stderr, "Got: r=%d g=%d b=%d\n", c.r, c.g, c.b);
    throw "Color should be #FF5733";
  }
}

inline void customTypeDefaultValueTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      opt<Point, "origin">('o', "--origin", "Origin", some(Point{1.0, 2.0})));

  const char* argv[] = {"prog"};
  auto result = parser.parse(1, argv);

  if (!result.has_value()) {
    throw "Default value parsing should succeed";
  }

  Point p = parser.get<"origin">().value();
  if (p.x != 1.0 || p.y != 2.0) {
    throw "Default Point should be (1.0, 2.0)";
  }
}

inline void customTypeCombinedTest() {
  auto parser = ArgumentParser("TestApp", "TestDescription",
      opt<Point, "pos">('p', "--position", "Position", some(Point{0.0, 0.0})),
      opt<Color, "color">('c', "--color", "Color", some(Color{255, 255, 255})),
      optInt<"size">('s', "--size", "Size", some(10)));

  const char* argv[] = {"prog", "-p", "5,5", "-c", "00FF00", "-s", "20"};
  auto result = parser.parse(7, argv);

  if (!result.has_value()) {
    result.error().print();
    throw "Combined custom types should succeed";
  }

  Point p = parser.get<"pos">().value();
  Color c = parser.get<"color">().value();
  int s = parser.get<"size">().value();

  if (p.x != 5.0 || p.y != 5.0) throw "Position should be (5,5)";
  if (c.r != 0 || c.g != 255 || c.b != 0) throw "Color should be green";
  if (s != 20) throw "Size should be 20";
}

inline void runCustomTypeTests() {
  std::printf("  customTypePointTest...");
  customTypePointTest();
  std::printf(" OK\n");

  std::printf("  customTypeColorTest...");
  customTypeColorTest();
  std::printf(" OK\n");

  std::printf("  customTypeDefaultValueTest...");
  customTypeDefaultValueTest();
  std::printf(" OK\n");

  std::printf("  customTypeCombinedTest...");
  customTypeCombinedTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests