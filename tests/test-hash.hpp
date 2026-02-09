#pragma once
#include <etched/etched.hpp>
#include <cstdio>

namespace etched::tests {

inline void hashBasicTest() {
  auto hash1 = detail::Hash<DefaultHashConfig>::salted("port", 0);
  auto hash2 = detail::Hash<DefaultHashConfig>::salted("port", 0);
  if (hash1 != hash2) {
    throw "Same string should produce same hash";
  }
}

inline void hashDifferentStringsTest() {
  auto hash1 = detail::Hash<DefaultHashConfig>::salted("port", 0);
  auto hash2 = detail::Hash<DefaultHashConfig>::salted("host", 0);
  if (hash1 == hash2) {
    throw "Different strings should produce different hashes";
  }
}

inline void hashSubstrTest() {
  std::string_view full = "--port";
  std::string_view stripped = full.substr(2);
  std::string_view direct = "port";

  auto hash1 = detail::Hash<DefaultHashConfig>::salted(stripped, 0);
  auto hash2 = detail::Hash<DefaultHashConfig>::salted(direct, 0);
  if (hash1 != hash2) {
    std::fprintf(stderr, "hash(substr): %u, hash(direct): %u\n", hash1, hash2);
    throw "substr and direct string should have same hash";
  }
}

inline void hashEmptyStringTest() {
  auto hash = detail::Hash<DefaultHashConfig>::salted("", 0);
  if (hash != 2872998923U) {
    throw "Empty string should return expected salted hash";
  }
}

inline void hashKnownValuesTest() {
  // FNV-1a known test vectors (now with avalanche)
  auto hash_empty = detail::Hash<DefaultHashConfig>::salted("", 0);
  if (hash_empty != 2872998923U) {
    throw "Salted empty string should be 2872998923";
  }
}

inline void runHashTests() {
  std::printf("  hashBasicTest...");
  hashBasicTest();
  std::printf(" OK\n");

  std::printf("  hashDifferentStringsTest...");
  hashDifferentStringsTest();
  std::printf(" OK\n");

  std::printf("  hashSubstrTest...");
  hashSubstrTest();
  std::printf(" OK\n");

  std::printf("  hashEmptyStringTest...");
  hashEmptyStringTest();
  std::printf(" OK\n");

  std::printf("  hashKnownValuesTest...");
  hashKnownValuesTest();
  std::printf(" OK\n");
}

}  // namespace etched::tests