#include <cmath>
#include <cstdint>
#include <etched/etched.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

struct Point2D {
  double x;
  double y;
};

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

namespace etched {

template <>
auto fromStr<Point2D>(const char* str) -> Point2D {
  if (!str) {
    throw std::invalid_argument("Null pointer for Point2D");
  }

  std::string input(str);
  size_t comma = input.find(',');

  if (comma == std::string::npos) {
    throw std::invalid_argument("Point2D format: x,y");
  }

  std::string xStr = input.substr(0, comma);
  std::string yStr = input.substr(comma + 1);

  return Point2D{std::stod(xStr), std::stod(yStr)};
}

template <>
auto fromStr<Color>(const char* str) -> Color {
  if (!str) {
    throw std::invalid_argument("Null pointer for Color");
  }

  std::string input(str);

  if (input[0] == '#') {
    input = input.substr(1);
  }

  if (input.length() != 6) {
    throw std::invalid_argument("Color format: #RRGGBB or RRGGBB");
  }

  auto parseHex = [](const std::string& hex) -> uint8_t {
    return static_cast<uint8_t>(std::stoi(hex, nullptr, 16));
  };

  return Color{parseHex(input.substr(0, 2)), parseHex(input.substr(2, 2)),
               parseHex(input.substr(4, 2))};
}

}  // namespace etched

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      opt<Point2D, "position">("-p", "--position", "Position (x,y)",
                               Point2D{0.0, 0.0}),
      opt<Color, "color">("-c", "--color", "Color (#RRGGBB)",
                          Color{255, 255, 255}),
      optBool<"verbose">("-v", "--verbose", "Verbose output"),
      optHelp("-h", "--help"));

  try {
    parser.parse(argc, argv);

    auto pos = parser.getOption<"position">().value.value();
    auto color = parser.getOption<"color">().value.value();
    auto verbose = parser.getOption<"verbose">().value.value_or(false);

    std::cout << "Configuration:\n";
    std::cout << "  Position: (" << pos.x << ", " << pos.y << ")\n";
    std::cout << "  Color: RGB(" << static_cast<int>(color.r) << ", "
              << static_cast<int>(color.g) << ", " << static_cast<int>(color.b)
              << ")\n";
    std::cout << "  Verbose: " << (verbose ? "ON" : "OFF") << "\n";

    if (verbose) {
      std::cout << "\nDetailed info:\n";
      std::cout << "  Point magnitude: "
                << std::sqrt(pos.x * pos.x + pos.y * pos.y) << "\n";
      std::cout << "  Color hex: #" << std::hex << static_cast<int>(color.r)
                << static_cast<int>(color.g) << static_cast<int>(color.b)
                << std::dec << "\n";
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    std::cerr << "\nUsage: " << argv[0] << " [OPTIONS]\n";
    std::cerr << "Options:\n";
    std::cerr << "  -p, --position <x,y>     Position coordinates (default: "
                 "0.0,0.0)\n";
    std::cerr << "  -c, --color <#RRGGBB>    Color in hex (default: #FFFFFF)\n";
    std::cerr << "  -v, --verbose            Verbose output\n";
    std::cerr << "  -h, --help               Show help\n";
    std::cerr << "\nExamples:\n";
    std::cerr << "  " << argv[0] << " -p 3.14,2.71 -c #FF5733\n";
    std::cerr << "  " << argv[0] << " --position 100,200 --color FF0000 -v\n";
    return 1;
  }

  return 0;
}
