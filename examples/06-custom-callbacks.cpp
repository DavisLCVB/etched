#include <etched/etched.hpp>
#include <iostream>

void showCredits() {
  std::cout << "Created by: The Etched Team\n";
  std::cout << "License: MIT\n";
}

void showStats() {
  std::cout << "Statistics:\n";
  std::cout << "  - Total users: 1337\n";
  std::cout << "  - Uptime: 99.9%\n";
}

int main(int argc, const char* argv[]) {
  using namespace etched;

  auto parser = ArgumentParser(
      optInt<"port">("-p", "--port", "Server port", 8080),
      optCallback<"credits">("-c", "--credits", "Show credits", showCredits),
      optCallback<"stats">("-s", "--stats", "Show statistics", showStats),
      optHelp("-h", "--help"));

  parser.parse(argc, argv);

  std::cout << "Application continues after callbacks...\n";

  return 0;
}
