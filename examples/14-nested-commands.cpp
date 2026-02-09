#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) -> int {
  using namespace etched;

  // Git-style nested commands: git remote add <name> <url>
  auto parser = ArgumentParser(
      "mygit", "Mini Git clone",
      cmd<"remote">("Manage set of tracked repositories",
                    cmd<"add">("Add a new remote",
                               optString<"name">('n', "name", "Remote name"),
                               optString<"url">('u', "url", "Remote URL"),
                               optHelp('h', "help")),
                    cmd<"list">("List all remotes", optHelp('h', "help")),
                    optHelp('h', "help")),
      optHelp('h', "help"));

  auto result = parser.parse(argc, argv);

  if (result.isOk() && result.unwrap().shouldExit) {
    return 0;
  }

  if (!result.isOk()) {
    result.unwrapErr().print();
    return 1;
  }

  if (parser.has<"remote">()) {
    const auto& remote = parser.get<"remote">();
    if (remote.has<"add">()) {
      const auto& add = remote.get<"add">();
      auto name = add.get<"name">().valueOr("origin");
      auto url = add.get<"url">().valueOr("unknown");
      std::cout << "Adding remote '" << name << "' with URL: " << url << "\n";
    } else if (remote.has<"list">()) {
      std::cout << "Listing remotes...\n";
      std::cout << "  origin  https://github.com/user/repo (fetch)\n";
      std::cout << "  origin  https://github.com/user/repo (push)\n";
    } else {
      std::cout << remote.help() << "\n";
    }
  } else {
    std::cout << parser.help() << "\n";
  }

  return 0;
}
