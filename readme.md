# Etched

A high-performance, **compile-time** argument parsing library for C++20. Etched uses a Hash Table and Jump Table to provide near-instant parsing with zero heap allocations during the core process.

## Key Improvements v2.0
- **Commands and nested commands support**: Full support for hierarchical CLI structures `(git remote add ...)` where each subcommand acts as a isolated "mini-parser".

- **Perfect Hashing**: Option lookups are $O(1)$ at runtime, powered by a compile-time salt search that eliminates hash collisions.

- **Advanced Lexer**: Handles short-flag clustering `(-xvf)`, long-option values `(--key=val)`, and the `--` delimiter natively.

- **Result-based Error Handling**: Moves away from pure exceptions to a more modern Result<T> pattern, inspired by Rust/C++23 expected.

## Basic Usage

Etched is designed to be intuitive and safe. You define your parser, run the execution, and access your data using compile-time tags.

```c++
#include <etched/etched.hpp>
#include <iostream>

auto main(int argc, const char* argv[]) noexcept -> int {  // NOLINT
    using namespace etched;

    static constexpr int defaultPort = 8080;

    auto parser = ArgumentParser(
                      "ExampleApp", "ExampleDescription",
                      optInt<"port">('p', "port", "Server port", some(defaultPort)),
                      optString<"host">(noShort, "host", "Server host", "localhost"));

    auto result = parser.parse(argc, argv);

    if (!result.isOk()) {
      result.unwrapErr().print();
      return 1;
    }

    std::cout << "Host: " << parser.get<"host">().value() << "\n";
    std::cout << "Port: " << parser.get<"port">().value() << "\n";
  return 0;
}
```

**Terminal execution:**

```shell
# Using default values:
./myapp
# Output: 
# Host: localhost
# Port: 8080

# Using defined flags
./myapp -p3000 --host 0.0.0.0
# Output:
# Host: 0.0.0.0
# Port: 3000
```

## Technical Architecture

Etched isn't just a wrapper around argv. It's a statically-baked state machine:

1. **Symbol Table**: A consteval process finds a perfect salt for your flags. At runtime, looking up --verbose is a single salted hash and an array access.

2. **Jump Table**: Instead of if/else chains, Etched uses an array of function pointers. The index from the Symbol Table "jumps" directly to the code that handles your specific option type.

3. **Zero-Copy Lexer**: Uses std::string_view to point directly to argv memory. No strings are copied or allocated during parsing.

## Custom Type support

To support your own types, simply specialize the deserialize function. 

```c++
template <>
auto deserialize(std::string_view str, MyType*) -> Result<MyType> {
    // Your parsing logic here
    if (success) return ok(MyType{...});
    return err<MyType>("Invalid format");
}
```

## Limitations

- **C++20 Required**: Uses FixedString templates and concepts.
- **Homogeneous Positionals**: All positional arguments must share the same type (or use std::string_view and parse manually).
- **Flags cannot have value**: The lexer cannot parse flags with values like `-p true`, `-p 1`, `-pON`, `--prefix=ON`. 

