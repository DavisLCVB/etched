#include <etched/etched.hpp>
#include <iostream>
#include <string>

struct Point {
  double x, y;
};

namespace etched {
template <>
// Custom deserialization for Point type
auto deserialize(std::string_view str, [[maybe_unused]] Point* dummy)
    -> Expected<Point, RuntimeError> {
  using namespace etched;
  std::cout << "Deserializing Point from: " << str << "\n";
  auto comma = str.find(',');
  if (comma == std::string_view::npos) {
    return etched::err(RuntimeError{"Point must be in format x,y"});
  }

  try {
    double x = std::stod(std::string(str.substr(0, comma)));
    double y = std::stod(std::string(str.substr(comma + 1)));
    return Point{.x = x, .y = y};
  } catch (...) {
    return etched::err(RuntimeError{"Invalid coordinates"});
  }
}
}  // namespace etched

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      opt<Point, "point">('p', "point", "A 2D point (x,y)", noDefault<Point>));

  auto result = parser.parse(argc, argv);

  if (!result.has_value()) {
    result.error().print();
    return 1;
  }

  if (parser.has<"point">()) {
    Point p = parser.get<"point">().value();
    std::cout << "Point: (" << p.x << ", " << p.y << ")\n";
  } else {
    std::cout << "No point provided.\n";
  }
  return 0;
}
