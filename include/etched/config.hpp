#ifndef ETCHED_CONFIG_HPP
#define ETCHED_CONFIG_HPP

#include <cstddef>

#include "jump_table.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "symbol_table.hpp"

namespace etched {

/**
 * Default configuration for Helper generator
 */
struct DefaultHelpConfig {
  // Maximum size of the generated help message
  static constexpr size_t maxHelpSize = 4096;
  // Maximum width of each column in the help message
  // Needed because help is generated in compile-time
  static constexpr size_t colWidth = 22;
};

/*
 * Default configuration for Lexer
 */
struct DefaultLexerConfig {
  // Flag to allow or disallow clusters
  // (groups of characters treated as a single token e.g., "-abc" or "-hl")
  static constexpr bool allowClusters = true;
  // Flag to allow if and value-option can take the next
  // token as their value. Even if the next token is another flag
  // e.g., "--path --another" will treat "--another" as the value of "--path")
  // Disabled by default to prevent ambiguity and unintended consequences.
  static constexpr bool prefferValues = false;
};

struct DefaultHashConfig {
  // Initial hash value used in the perfect hashing algorithm.
  static constexpr uint32_t initHash = 2166136261U;
  // Prime number used in the perfect hashing algorithm to reduce collisions.
  static constexpr uint32_t prime = 16777619U;
  // Multiplier used in the perfect hashing algorithm to compute the hash value.
  static constexpr uint32_t saltMultiplier = 0x5bd1e995;
};

/*
 * Default configuration for Symbol Table
 */
struct DefaultSTConfig {
  // Factor to determine the size of the symbol table based on the number of keys.
  static constexpr size_t factor = 2;
  // Maximum number of keys that can be stored in the symbol table.
  static constexpr size_t maxKeysLimit = 128;
  // Maximum number of attempts to find a perfect hash salt before giving up.
  static constexpr uint32_t maxSaltAttempts = 10000;
  // Maximum size of strings stored in the symbol table.
  static constexpr size_t maxStringSize = 64;
  // Hash configuration used in the symbol table, which is set to the default hash configuration.
  using hashConfig = DefaultHashConfig;
};

/*
 * Default configuration for the entire library
 */
struct DefaultConfig {
  // The lexer type used in the library, which is set to the default lexer with
  // the default lexer configuration.
  using Lexer = detail::DefaultLexer<DefaultLexerConfig>;
  // The parser type used in the library, which is set to the default parser.
  using Parser = detail::DefaultParser;
  // The symbol table configuration used in the library, which is set to the
  // default symbol table configuration.
  template <size_t N>
  using SymbolTable = detail::DefaultSymbolTable<N, DefaultSTConfig>;
  // The jump table type used in the library, which is set to the default jump
  // table.
  template <typename... Args>
  using JumpTable = detail::DefaultJumpTable<Lexer, Args...>;
  // The help configuration used in the library, which is set to the default
  // help configuration.
  using helpConfig = DefaultHelpConfig;

  // The maximum number of arguments that can be processed by the library,
  // set to a default value of 1000. This limit is in place to prevent excessive
  // resource usage and potential performance issues.
  static constexpr size_t maxArgs = 1000;
};

}  // namespace etched

#endif  // ETCHED_CONFIG_HPP
