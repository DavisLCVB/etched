#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"port">("-p", "--port", "Server port", 8080),
      optVersion("MyApp v2.5.1", "-v", "--version", "Show version"),
      optHelp("-h", "--help"));

  parser.parse(argc, argv);

  std::cout << "Application running...\n";

  return 0;
}
