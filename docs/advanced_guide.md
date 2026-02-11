# Etched Advanced Guide

This guide dives deeper into the internal mechanics and advanced usage patterns of Etched.

## 1. How Perfect Hashing Works

Etched ensures $O(1)$ option lookup by finding a "perfect salt" at compile-time.

1. **Collection**: All option flags (short and long) are collected into a static array at compile-time.
2. **Salt Search**: A `consteval` function iterates through possible salt values. For each salt, it hashes all keys and checks for collisions in a table of size $N \times \text{factor}$.
3. **Verification**: Once a collision-free salt is found, it's baked into the `SymbolTable`.
4. **Runtime**: At runtime, the input string is hashed once with the stored salt, directly providing the index in the `JumpTable`.

If the search fails to find a salt within the `maxSaltAttempts` limit, a compile-time error is raised. You can fix this by increasing the `factor` or `maxSaltAttempts` in your custom configuration.

## 2. The Jump Table Mechanism

The `JumpTable` is a static array of function pointers. Each function is a template instantiation specialized for a specific option's type and logic.

- **Zero overhead**: Unlike virtual functions or `std::function`, these are plain function pointers, easily inlined by the compiler.
- **Static Dispatch**: The index from the `SymbolTable` is used to call the appropriate handler directly.

## 3. Zero-Copy Lexing

Etched's lexer never copies strings from `argv`. It uses `std::string_view` to point to the original memory.

- **Tokenization**: It identifies `-`, `--`, and `=` separators to split tokens without allocation.
- **Clustering**: When clustering is enabled (`-xvf`), the lexer yields each character as a separate `SHORT_OPTION` token during sequential `nextToken()` calls, maintaining the zero-copy guarantee.

## 4. Custom Lexers and Parsers

Etched is designed with a decoupled architecture that allows you to swap the core tokenization and parsing logic. This is done by satisfying specific C++ concepts.

### Implementing a Custom Lexer

A Lexer is responsible for turning the raw `char** argv` into a stream of `Token` objects. To implement your own, your class must satisfy the `etched::detail::IsLexer` concept:

| Method | Description |
|---|---|
| `setTokens(int, const char**)` | Initializes the lexer with the command line arguments. |
| `nextToken(ParsingContext)` | Returns the next `Result<Token>`. The context hints if we are looking for a value. |
| `currentToken()` | Returns the last successfully retrieved `Token`. |

**Example:**
```cpp
struct MyLexer {
    void setTokens(int argc, const char** argv) { /* ... */ }
    auto nextToken(etched::detail::ParsingContext ctx) -> etched::Result<etched::detail::Token> {
        // Your custom tokenization logic
    }
    auto currentToken() -> etched::detail::Token { /* ... */ }
};
```

### Implementing a Custom Parser

The Parser defines the grammar of your CLI. It consumes tokens from the Lexer and dispatches actions via the Jump Table. It must satisfy the `etched::detail::IsParser` concept, which requires a static `parse` method:

| Requirement | Description |
|---|---|
| `static parse(...)` | Takes the Lexer, Symbol Table, Jump Table, and Options tuple to execute the parsing logic. |

**Example:**
```cpp
struct MyParser {
    template <typename Lexer, typename SymbolTable, typename JumpTable, typename Options>
    static auto parse(Lexer& l, const SymbolTable& st, const JumpTable& jt, Options& opts) 
        -> etched::Result<etched::Output> {
        // Custom state machine or recursive descent logic
    }
};
```

### Hooking them up in Configuration

Once implemented, bind them in a custom configuration struct:

```cpp
struct AdvancedConfig : etched::DefaultConfig {
    using Lexer = MyLexer;
    using Parser = MyParser;
};

auto parser = etched::ArgumentParser(etched::WithConfig<AdvancedConfig>{}, "App", "Desc", ...);
```

This flexibility allows you to support alternative CLI syntaxes (like Windows-style `/f` flags) or different parsing strategies entirely.

## 5. Performance Tips

- **Minimize Tags**: Each tag increases the search space for perfect hashing.
- **Short Long Names**: Keep long option names reasonable to stay within `maxStringSize`.
- **Pre-calculate Results**: Since many values are available at compile-time, leverage `constexpr` where possible when interacting with the parser's configuration.

## 6. Integration with build systems

Beyond CMake, since Etched is header-only, you can simply add the `include` path to your build:

- **GCC/Clang**: `-I/path/to/etched/include -std=c++20`
- **MSVC**: `/I"C:\path\to\etched\include" /std:c++20`

## Prev
[User guide](./user_guide.md)

## Next 
[Architecture](./architecture.md)
