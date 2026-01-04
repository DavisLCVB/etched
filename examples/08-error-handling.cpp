#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"port">("-p", "--port", "Server port", 8080),
      optString<"host">("-h", "--host", "Server host", "localhost"));

  try {
    parser.parse(argc, argv);

    std::cout << "Success! Configuration:\n";
    std::cout << "  Host: " << parser.getOption<"host">().value.value() << "\n";
    std::cout << "  Port: " << parser.getOption<"port">().value.value() << "\n";

  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << "\n";
    std::cerr << "\nUsage: " << argv[0] << " [OPTIONS]\n";
    std::cerr << "Options:\n";
    std::cerr << "  -p, --port <number>   Server port (default: 8080)\n";
    std::cerr << "  -h, --host <string>   Server host (default: localhost)\n";
    return 1;
  } catch (const std::out_of_range& e) {
    std::cerr << "Value out of range: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
