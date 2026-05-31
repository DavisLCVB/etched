#ifndef ETCHED_CONTRACTS_HPP
#define ETCHED_CONTRACTS_HPP

#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "types.hpp"
#include "utils/optional.hpp"

namespace etched::detail {
/**
 * @brief Forward declaration of PositionalContainer.
 */
template <typename T>
struct PositionalContainer;
}  // namespace etched::detail

namespace etched {

/**
 * @brief Deserialize a string into a type T.
 * 
 * @tparam T Target type.
 * @param str The string to deserialize.
 * @param ptr Dummy pointer for overload resolution.
 * @return Result containing the deserialized value or an error.
 */
template <typename T>
auto deserialize(std::string_view str, T*) -> Expected<T, RuntimeError>;

/**
 * @brief Specialization for positional containers to prevent deserialization.
 * 
 * @tparam T Element type.
 * @return Always returns an error.
 */
template <typename T>
inline auto deserialize(std::string_view /*unused*/,
                        detail::PositionalContainer<T>* /*unused*/)
    -> Expected<detail::PositionalContainer<T>, RuntimeError> {
  return etched::err(RuntimeError{"Cannot deserialize container"});
}
}  // namespace etched

namespace etched::detail {

/**
 * @brief Concept to check if a type is deserializable.
 */
template <typename T>
concept IsDeserializable = requires(std::string_view str) {
  {
    deserialize(str, static_cast<T*>(nullptr))
  } -> std::convertible_to<Expected<T, RuntimeError>>;
};

/**
 * @brief Concept to check if a type is an integer (excluding bool).
 */
template <typename T>
concept Integer = std::integral<T> && !std::same_as<T, bool>;

/**
 * @brief Concept to check if a type is a valid action callback.
 */
template <typename T>
concept IsAction =
    std::same_as<T, NoCallbackType> ||
    (std::is_invocable_v<T> &&
     (std::same_as<std::invoke_result_t<T>, void> ||
      std::convertible_to<std::invoke_result_t<T>, Expected<Output, RuntimeError>>));

/**
 * @brief Concept for positional options.
 */
template <typename T>
concept IsPositionalOption = requires(const T t) {
  typename T::ValueType;
  requires T::tag == "positional";
  { t.value } -> std::same_as<const typename T::ValueType&>;
};

/**
 * @brief Concept for general options.
 */
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
      { t.trigger() } -> std::convertible_to<Expected<Output, RuntimeError>>;
    };

/**
 * @brief Concept for options that take a value.
 */
template <typename T>
concept IsOptionValue = IsOption<T> && !IsPositionalOption<T> && requires {
  typename T::ValueType;
  requires !std::is_same_v<typename T::ValueType, bool>;
};

/**
 * @brief Concept for flag options (boolean).
 */
template <typename T>
concept IsOptionFlag = IsOption<T> && !IsPositionalOption<T> && requires {
  typename T::ValueType;
  requires std::is_same_v<typename T::ValueType, bool>;
};

/**
 * @brief Concept for commands.
 */
template <typename T>
concept IsCommand =
    requires { typename T::Config::Lexer; } && requires(const T t) {
      { T::tag };
      { t.name } -> std::convertible_to<std::string_view>;
    } && requires(T t, typename T::Config::Lexer& l) {
      { t.parse(l) } -> std::convertible_to<Expected<Output, RuntimeError>>;
    };

/**
 * @brief Concept for arguments (options or commands).
 */
template <typename T>
concept IsArgument = IsOption<T> || IsCommand<T>;

/**
 * @brief Types of tokens encountered during lexing.
 */
enum class TokenType : uint8_t {
  SHORT_OPTION,
  LONG_OPTION,
  POSITIONAL,
  SEPARATOR,
  END_OF_INPUT,
};

/**
 * @brief Represents a token from the input.
 */
struct Token {
  // The value of the token.
  std::string_view value;
  // The type of the token.
  TokenType type;
};

/**
 * @brief Types of entries in the symbol table.
 */
enum class EntryType : uint8_t { OPTION, FLAG, COMMAND, POSITIONAL, UNKNOWN };

/**
 * @brief Metadata for a symbol table entry.
 */
struct EntryMetadata {
  // Index of the entry in the storage.
  size_t index;
  // Type of the entry.
  EntryType type;
};

/**
 * @brief Parsing context to influence token interpretation.
 */
enum class ParsingContext : uint8_t { DEFAULT, WAITING_FOR_VALUE };

/**
 * @brief Concept for lexers.
 */
template <typename T>
concept IsLexer =
    requires(T t, int argc, const char** argv, ParsingContext ctx) {
      { t.nextToken(ctx) } -> std::same_as<Expected<Token, RuntimeError>>;
      { t.setTokens(argc, argv) } -> std::same_as<void>;
      { t.currentToken() } -> std::same_as<Token>;
    } && std::is_default_constructible_v<T>;

/**
 * @brief Concept for symbol tables.
 */
template <typename T, typename... Args>
concept IsSymbolTable =
    requires(const T t, std::string_view key, const std::tuple<Args...>& args) {
      { t.find(key) } -> std::same_as<Optional<EntryMetadata>>;
      { t.positionalIndex() } -> std::same_as<Optional<size_t>>;
      { T::create(args) } -> std::same_as<T>;
    };

/**
 * @brief Concept for jump tables.
 */
template <typename T, typename Lexer, typename Options>
concept IsJumpTable = requires(T t, size_t index, Options& opts, Lexer& l) {
  { t.dispatch(index, opts, l) } -> std::same_as<Expected<Output, RuntimeError>>;
  { T::create() } -> std::same_as<T>;
} && IsLexer<Lexer>;

/**
 * @brief Concept for parsers.
 */
template <typename P, typename Lexer, typename SymbolTable, typename JumpTable,
          typename Options>
concept IsParser = IsLexer<Lexer> && IsSymbolTable<SymbolTable> &&
                   IsJumpTable<JumpTable, Lexer, Options> &&
                   requires(Lexer& l, const SymbolTable& st,
                            const JumpTable& jt, Options& opts) {
                     {
                       P::parse(l, st, jt, opts)
                     } -> std::convertible_to<Expected<Output, RuntimeError>>;
                   };

/**
 * @brief Concept for lexer configuration.
 */
template <typename T>
concept IsLexerConfig = requires {
  { T::allowClusters } -> std::convertible_to<bool>;
  { T::prefferValues } -> std::convertible_to<bool>;
};

/**
 * @brief Concept for hash configuration.
 */
template <typename T>
concept IsHashConfig = requires {
  { T::initHash } -> std::convertible_to<uint32_t>;
  { T::prime } -> std::convertible_to<uint32_t>;
  { T::saltMultiplier } -> std::convertible_to<uint32_t>;
};

/**
 * @brief Concept for symbol table configuration.
 */
template <typename T>
concept IsSymbolTableConfig = requires {
  { T::factor } -> std::convertible_to<double>;
  { T::maxKeysLimit } -> std::convertible_to<size_t>;
  { T::maxSaltAttempts } -> std::convertible_to<uint32_t>;
  { T::maxStringSize } -> std::convertible_to<size_t>;
  typename T::hashConfig;
  requires IsHashConfig<typename T::hashConfig>;
};

/**
 * @brief Concept for help configuration.
 */
template <typename T>
concept IsHelpConfig = requires {
  { T::maxHelpSize } -> std::convertible_to<size_t>;
};

/**
 * @brief Concept for the overall orchestrator configuration.
 */
template <typename T>
concept IsOrchestratorConfig = requires {
  typename T::Lexer;
  typename T::Parser;
  typename T::helpConfig;
  requires IsHelpConfig<typename T::helpConfig>;
  { T::maxArgs } -> std::convertible_to<size_t>;
};

}  // namespace etched::detail

#endif  // ETCHED_CONTRACTS_HPP
