#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"port">("-p", "--port", "Server port", 8080),
      optString<"host">("-h", "--host", "Server host", "localhost"));

  parser.parse(argc, argv);

  auto port = parser.getOption<"port">();
  auto host = parser.getOption<"host">();

  std::cout << "Server configuration:\n";
  std::cout << "  Host: " << host.value.value() << "\n";
  std::cout << "  Port: " << port.value.value() << "\n";

  return 0;
}
