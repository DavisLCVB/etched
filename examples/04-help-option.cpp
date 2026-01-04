#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"port">("-p", "--port", "Server port", 8080),
      optString<"config">("-c", "--config", "Config file path"),
      optBool<"verbose">("-v", "--verbose", "Verbose output"),
      optHelp("-h", "--help"));

  parser.parse(argc, argv);

  std::cout << "Program started successfully!\n";

  return 0;
}
