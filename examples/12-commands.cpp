#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {  // NOLINT
  using namespace etched;

  static constexpr int defaultPort = 8080;
  static constexpr int defaultJobs = 4;

  auto parser = ArgumentParser(
      "ExampleApp", "ExampleDescription",
      cmd<"init">("Initialize a new project",
                  optString<"name">('n', "name", "Project name", "my_project"),
                  optBool<"git">('g', "git", "Initialize git repository"),
                  // Commands generates their own namespace for options
                  optHelp('h', "help")),
      cmd<"build">("Build the current project",
                   optBool<"release">('r', "release", "Build in release mode"),
                   optInt<"jobs">('j', "jobs", "Number of parallel jobs",
                                  some(defaultJobs)),
                   optHelp('h', "help")),
      cmd<"serve">(
          "Start a development server",
          optInt<"port">('p', "port", "Port to listen on", some(defaultPort)),
          optString<"host">('H', "host", "Host interface", "127.0.0.1"),
          optHelp('h', "help")),
      optBool<"verbose">('v', "verbose", "Enable verbose logging"),
      optHelp('h', "help"));

  auto result = parser.parse(argc, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  if (result.unwrap().shouldExit) {
    return 0;
  }

  // Use the has() method to check for commands or flags
  if (parser.has<"verbose">()) {
    std::cout << "[VERBOSE] Logging enabled\n";
  }

  // Check for commands using has<"tag">()
  if (parser.has<"init">()) {
    // get<"tag">() for a command returns the command object (by reference)
    const auto& initCmd = parser.get<"init">();
    std::cout << "Initializing project: " << initCmd.get<"name">().value()
              << "\n";
    if (initCmd.has<"git">()) {
      std::cout << "  - Initializing git repository...\n";
    }
  } else if (parser.has<"build">()) {
    const auto& buildCmd = parser.get<"build">();
    bool isRelease = buildCmd.has<"release">();
    int jobs = buildCmd.get<"jobs">().value();
    std::cout << "Building project (mode: " << (isRelease ? "release" : "debug")
              << ")\n";
    std::cout << "  - Parallel jobs: " << jobs << "\n";
  } else if (parser.has<"serve">()) {
    const auto& serveCmd = parser.get<"serve">();
    std::string_view host = serveCmd.get<"host">().value();
    int port = serveCmd.get<"port">().value();
    std::cout << "Starting server on " << host << ":" << port << "\n";
  } else {
    std::cout << "No command provided. Use --help to see available commands.\n";
  }
  return 0;
}
