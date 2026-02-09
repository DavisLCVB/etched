#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline void nestedCommandsGitStyleTest() {
  auto parser = ArgumentParser("git", "Git version control",
      cmd<"remote">("Manage set of tracked repositories",
          cmd<"add">("Add a remote",
              optString<"name">('n', "--name", "Remote name"),
              optString<"url">('u', "--url", "Remote URL")
          )
      )
  );

  const char* argv[] = {"git", "remote", "add", "--name", "origin", "-u", "https://github.com/user/repo.git"};
  auto result = parser.parse(7, argv);

  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Nested command parsing failed";
  }

  if (!parser.has<"remote">()) throw "Should have 'remote' command";
  
  const auto& remoteCmd = parser.get<"remote">();
  if (!remoteCmd.has<"add">()) throw "Should have 'add' sub-command";

  const auto& addCmd = remoteCmd.get<"add">();
  if (addCmd.get<"name">().value() != "origin") throw "Wrong sub-option value for name";
  if (addCmd.get<"url">().value() != "https://github.com/user/repo.git") throw "Wrong sub-option value for url";
}

inline void nestedCommandsScopeTest() {
  // Test variable shadowing / scoping in nested commands
  auto parser = ArgumentParser("App", "Desc",
      optInt<"id">('i', "--id", "Global ID", some(1)),
      cmd<"user">("User command",
          optInt<"id">('i', "--id", "User ID", some(10)),
          cmd<"profile">("Profile command",
              optInt<"id">('i', "--id", "Profile ID", some(20))
          )
      )
  );

  const char* argv[] = {"app", "user", "profile", "-i", "21"};
  // Here -i 21 should apply to profile command.
  // user command should take default 10.
  // global should take default 1.
  
  auto result = parser.parse(5, argv);
  if (!result.isOk()) {
    result.unwrapErr().print();
    throw "Scope test parsing failed";
  }

  if (parser.get<"id">().value() != 1) throw "Global ID scope error";
  
  const auto& user = parser.get<"user">();
  if (user.get<"id">().value() != 10) throw "Level 1 ID scope error"; // Should be default
  // Wait, does -i apply to the deepest command only?
  // In this design, options are context sensitive.
  // "user profile -i 21" -> -i belongs to profile because profile is the active command being parsed.

  const auto& profile = user.get<"profile">();
  if (profile.get<"id">().value() != 21) throw "Level 2 ID scope error";
}

inline void runNestedCommandTests() {
  std::printf("  nestedCommandsGitStyleTest...");
  nestedCommandsGitStyleTest();
  std::printf(" OK\n");

  std::printf("  nestedCommandsScopeTest...");
  nestedCommandsScopeTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests
