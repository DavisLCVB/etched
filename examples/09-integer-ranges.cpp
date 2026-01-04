#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"tiny", int8_t>("-t", "--tiny", "Tiny number (-128 to 127)", 0),
      optInt<"small", int16_t>("-s", "--small", "Small number", 0),
      optInt<"normal", int32_t>("-n", "--normal", "Normal number", 0),
      optInt<"big", int64_t>("-b", "--big", "Big number", 0),
      optInt<"ubyte", uint8_t>("-u", "--ubyte", "Unsigned byte (0-255)", 0));

  try {
    parser.parse(argc, argv);

    std::cout << "Integer values:\n";
    std::cout << "  int8_t:  " << static_cast<int>(parser.getOption<"tiny">().value.value()) << "\n";
    std::cout << "  int16_t: " << parser.getOption<"small">().value.value() << "\n";
    std::cout << "  int32_t: " << parser.getOption<"normal">().value.value() << "\n";
    std::cout << "  int64_t: " << parser.getOption<"big">().value.value() << "\n";
    std::cout << "  uint8_t: " << static_cast<unsigned>(parser.getOption<"ubyte">().value.value()) << "\n";

  } catch (const std::out_of_range& e) {
    std::cerr << "Error: " << e.what() << "\n";
    std::cerr << "Make sure values are within the type's range!\n";
    return 1;
  }

  return 0;
}
