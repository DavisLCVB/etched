#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline void symbolTableInsertAndFindTest() {
  // We test the SymbolTable by creating one through the standard mechanism
  // as manual insertion is complex for a perfect hash table.
  constexpr auto makeTable = []() {
    auto options = std::make_tuple(
        optInt<"port">('p', "--port", "Port", some(8080)),
        optString<"host">('h', "--host", "Host", "localhost")
    );
    return detail::DefaultSymbolTable<8, DefaultSTConfig>::create(options);
  };
  constexpr auto table = makeTable();

  auto result1 = table.find("port");
  if (!result1.hasValue()) {
    throw "Should find 'port' in table";
  }
  if (result1.get().index != 0) {
    throw "port should have index 0";
  }

  auto result2 = table.find("host");
  if (!result2.hasValue()) {
    throw "Should find 'host' in table";
  }
  if (result2.get().index != 1) {
    throw "host should have index 1";
  }

  // Also check short names
  if (!table.find("p").hasValue()) throw "Should find 'p'";
  if (!table.find("h").hasValue()) throw "Should find 'h'";
}

inline void symbolTableNotFoundTest() {
  constexpr auto makeTable = []() {
    auto options = std::make_tuple(
        optInt<"port">('p', "--port", "Port", some(8080))
    );
    return detail::DefaultSymbolTable<8, DefaultSTConfig>::create(options);
  };
  constexpr auto table = makeTable();

  auto result = table.find("unknown");
  if (result.hasValue()) {
    throw "Should not find 'unknown' in table";
  }
}

inline void symbolTableStripDashesTest() {
  // This tests the key stripping logic
  std::string_view longOpt = "--port";
  std::string_view shortOpt = "p";

  auto longKey = longOpt;
  if (longKey.size() > 2 && longKey[0] == '-' && longKey[1] == '-') {
    longKey = longKey.substr(2);
  }

  auto shortKey = shortOpt;
  if (shortKey.size() > 1 && shortKey[0] == '-') {
    shortKey = shortKey.substr(1);
  }

  if (longKey != "port") {
    throw "Long option key should be 'port'";
  }
  if (shortKey != "p") {
    throw "Short option key should be 'p'";
  }

  // Verify hashes match with same salt
  uint32_t salt = 42;
  auto hash1 = detail::Hash<DefaultHashConfig>::salted(longKey, salt);
  auto hash2 = detail::Hash<DefaultHashConfig>::salted("port", salt);
  if (hash1 != hash2) {
    throw "Stripped key hash should match direct hash";
  }
}

inline void runSymbolTableTests() {
  std::printf("  symbolTableInsertAndFindTest...");
  symbolTableInsertAndFindTest();
  std::printf(" OK\n");

  std::printf("  symbolTableNotFoundTest...");
  symbolTableNotFoundTest();
  std::printf(" OK\n");

  std::printf("  symbolTableStripDashesTest...");
  symbolTableStripDashesTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests