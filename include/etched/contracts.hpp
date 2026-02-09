#ifndef ETCHED_CONTRACTS_HPP
#define ETCHED_CONTRACTS_HPP

#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "types.hpp"

namespace etched::detail {
// Forward declaration to prevent circular dependency between
// contracts and deserializer.
template <typename T>
struct PositionalContainer;
}  // namespace etched::detail

namespace etched {

// Template functions declaration. The dummy ptr is used to ensure
// function overloading over template specialization.
template <typename T>
auto deserialize(std::string_view str, T*) -> Result<T>;

// Specialization for positional containers to prevent deserialization of them.
template <typename T>
inline auto deserialize(std::string_view /*unused*/,
                        detail::PositionalContainer<T>* /*unused*/)
    -> Result<detail::PositionalContainer<T>> {
  return err<detail::PositionalContainer<T>>("Cannot deserialize container");
}
}  // namespace etched

// CONCEPTS FOR OPTIONS CHECKIN

namespace etched::detail {
// Concept to check if a type is deserializable using the deserialize function.
template <typename T>
concept IsDeserializable = requires(std::string_view str) {
  {
    deserialize(str, static_cast<T*>(nullptr))
  } -> std::convertible_to<Result<T>>;
};

// Concept to check if a type is an integer (excluding bool).
template <typename T>
concept Integer = std::integral<T> && !std::same_as<T, bool>;

// Concept to check if a type is a valid action callback.
template <typename T>
concept IsAction =
    std::same_as<T, NoCallbackType> ||
    (std::is_invocable_v<T> &&
     (std::same_as<std::invoke_result_t<T>, void> ||
      std::convertible_to<std::invoke_result_t<T>, Result<Output>>));

// Concept to check if a type is a valid Positional Options.[Option/Positional]
template <typename T>
concept IsPositionalOption = requires(const T t) {
  typename T::ValueType;
  requires T::tag == "positional";
  { t.value } -> std::same_as<const typename T::ValueType&>;
};

// Concept to check if a type is a valid Option (any type) [Option]
template <typename T>
concept IsOption =
    (requires(const T t) {
      typename T::ValueType;
      { t.shortName } -> std::convertible_to<char>;
      { t.longName } -> std::convertible_to<std::string_view>;
      {
        t.value
      } -> std::convertible_to<const Optional<typename T::ValueType>&>;
    } || IsPositionalOption<T>) && requires(T t) {
      { t.trigger() } -> std::convertible_to<Result<Output>>;
    };

// Concept to check if a type is a valid Option that takes a value (not a flag) [Option/Value]
template <typename T>
concept IsOptionValue = IsOption<T> && !IsPositionalOption<T> && requires {
  typename T::ValueType;
  requires !std::is_same_v<typename T::ValueType, bool>;
};

// Concept to check if a type is a valid Option that is a flag (boolean) [Option/Flag]
template <typename T>
concept IsOptionFlag = IsOption<T> && !IsPositionalOption<T> && requires {
  typename T::ValueType;
  requires std::is_same_v<typename T::ValueType, bool>;
};

// Concept to check if a type is a valid Command.
template <typename T>
concept IsCommand =
    requires { typename T::Config::Lexer; } && requires(const T t) {
      { T::tag };
      { t.name } -> std::convertible_to<std::string_view>;
    } && requires(T t, typename T::Config::Lexer& l) {
      { t.parse(l) } -> std::convertible_to<Result<Output>>;
    };

// Concept to check if a type is a valid Argument, which can be either an Option or a Command.
template <typename T>
concept IsArgument = IsOption<T> || IsCommand<T>;

// BASE STRUCT FOR PARSING

// Enum to represent Types of tokens
enum class TokenType : uint8_t {
  SHORT_OPTION,
  LONG_OPTION,
  POSITIONAL,
  SEPARATOR,  // e.g. "--"
  END_OF_INPUT,
};

// Struct to represent a token with its value and type
struct Token {
  std::string_view value;
  TokenType type;
};

// Enum to represent the behavior of the handle function in the jump table
enum class EntryType : uint8_t { OPTION, FLAG, COMMAND, POSITIONAL, UNKNOWN };

// Struct to represent metadata for an entry in the symbolTable.
// The type field indicates how the handle function of the jumpTable should process the entry.
struct EntryMetadata {
  size_t index;
  EntryType type;
};

// Enum to represent the context of parsing, which can influence how tokens are interpreted.
enum class ParsingContext : uint8_t {
  // Default context, no special rules
  DEFAULT,
  // When the previous token was an option that can take value
  WAINTING_FOR_VALUE
};

// CONCEPTS FOR PARSING COMPONENTS

// Concept to check if a type is a valid Lexer with the required interface.
template <typename T>
concept IsLexer =
    requires(T t, int argc, const char** argv, ParsingContext ctx) {
      { t.nextToken(ctx) } -> std::same_as<Result<Token>>;
      { t.setTokens(argc, argv) } -> std::same_as<void>;
      { t.currentToken() } -> std::same_as<Token>;
    } && std::is_default_constructible_v<T>;

// Concept to check if a type is a valid Symbol Table with the required interface.
template <typename T, typename... Args>
concept IsSymbolTable =
    requires(const T t, std::string_view key, const std::tuple<Args...>& args) {
      { t.find(key) } -> std::same_as<Optional<EntryMetadata>>;
      { t.positionalIndex() } -> std::same_as<Optional<size_t>>;
      { T::create(args) } -> std::same_as<T>;
    };

// Concept to check if a type is a valid Jump Table with the required interface.
template <typename T, typename Lexer, typename Options>
concept IsJumpTable = requires(T t, size_t index, Options& opts, Lexer& l) {
  { t.dispatch(index, opts, l) } -> std::same_as<Result<Output>>;
  { T::create() } -> std::same_as<T>;
} && IsLexer<Lexer>;

// Concept to check if a type is a valid Parser with the required interface.
template <typename P, typename Lexer, typename SymbolTable, typename JumpTable,
          typename Options>
concept IsParser = IsLexer<Lexer> && IsSymbolTable<SymbolTable> &&
                   IsJumpTable<JumpTable, Lexer, Options> &&
                   requires(Lexer& l, const SymbolTable& st,
                            const JumpTable& jt, Options& opts) {
                     {
                       P::parse(l, st, jt, opts)
                     } -> std::convertible_to<Result<Output>>;
                   };

// CONCEPTS FOR CONFIG CHECKING

// Concept for Lexer configuration.
template <typename T>
concept IsLexerConfig = requires {
  // Flag to allow or disallow clusters (groups of characters treated as a single token e.g., "-abc" or "-hl")
  { T::allowClusters } -> std::convertible_to<bool>;
  // Flag to allow if and value-option can take the next token as their value. Even if the next token is another flag
  { T::prefferValues } -> std::convertible_to<bool>;
};

// Concept for Hash configuration used in the symbol table.
template <typename T>
concept IsHashConfig = requires {
  // Initial hash value used in the perfect hashing algorithm.
  { T::initHash } -> std::convertible_to<uint32_t>;
  // Prime number used in the perfect hashing algorithm to reduce collisions.
  { T::prime } -> std::convertible_to<uint32_t>;
  // Multiplier used in the perfect hashing algorithm to compute the hash value.
  { T::saltMultiplier } -> std::convertible_to<uint32_t>;
};

// Concept for Symbol Table configuration.
template <typename T>
concept IsSymbolTableConfig = requires {
  // Factor to determine the size of the symbol table based on the number of keys.
  { T::factor } -> std::convertible_to<double>;
  // Maximum number of keys that can be stored in the symbol table.
  { T::maxKeysLimit } -> std::convertible_to<size_t>;
  // Maximum number of attempts to find a perfect hash salt before giving up.
  { T::maxSaltAttempts } -> std::convertible_to<uint32_t>;
  // Maximum size of strings stored in the symbol table.
  { T::maxStringSize } -> std::convertible_to<size_t>;
  // Hash configuration used in the symbol table.
  typename T::hashConfig;
  requires IsHashConfig<typename T::hashConfig>;
};

// Concept for Help configuration used in generating help messages.
template <typename T>
concept IsHelpConfig = requires {
  // Maximum size of the generated help message. (This is stored in the final binary)
  { T::maxHelpSize } -> std::convertible_to<size_t>;
};

// Concept for the overall Orchestrator configuration, which includes all
// components and their configurations.
template <typename T>
concept IsOrchestratorConfig = requires {
  typename T::Lexer;       // The lexer
  typename T::Parser;      // The parser
  typename T::helpConfig;  // The help configuration type
  requires IsHelpConfig<typename T::helpConfig>;
  { T::maxArgs } -> std::convertible_to<size_t>;  // Maximum number of arguments
};

}  // namespace etched::detail

#endif  // ETCHED_CONTRACTS_HPP
