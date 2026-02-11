# Etched Documentation

A high-performance, **compile-time** argument parsing library for C++20. Etched uses a Perfect Hash Table and Jump Table to provide near-instant parsing with zero heap allocations during the core process.

## Key Features

- **Compile-time Perfect Hashing**: Option lookups are $O(1)$ at runtime, eliminating hash collisions via a compile-time salt search.
- **Zero Heap Allocation**: The core parsing process operates without dynamic memory allocation.
- **Type Safety**: Access parsed values using compile-time tags, ensuring type correctness.
- **Modern C++**: Built with C++20 features like concepts, `std::string_view`, and fixed-string templates.
- **Subcommands and Nested Commands**: Full support for hierarchical CLI structures (e.g., `git remote add ...`).
- **Advanced Lexer**: Handles short-flag clustering (`-xvf`), long-option values (`--key=val`), and the `--` delimiter natively.
- **Result-based Error Handling**: Modern error management using a `Result<T>` pattern.

## Installation

Etched is a header-only library. You only need to include the `include` directory in your project.

### Integration via CMake

#### 1. FetchContent
```cmake
include(FetchContent)
FetchContent_Declare(
    etched
    GIT_REPOSITORY https://github.com/DavisLCVB/etched.git
    GIT_TAG main
)
FetchContent_MakeAvailable(etched)
target_link_libraries(your_target PRIVATE etched::etched)
```

#### 2. System-wide
```bash
cmake -B build
sudo cmake --install build
```
Then use `find_package(etched REQUIRED)`.

## Basic Usage

Define your parser using the `ArgumentParser` class and helper functions.

```c++
#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
    using namespace etched;

    auto parser = ArgumentParser("MyApp", "Description",
        optString<"name">('n', "name", "User name", "Guest"),
        optInt<"port">('p', "port", "Port number", some(8080)),
        optBool<"v">('v', "verbose", "Verbose mode")
    );

    auto result = parser.parse(argc, argv);
    if (!result.isOk()) {
        result.unwrapErr().print();
        return 1;
    }

    if (parser.get<"v">()) {
        std::cout << "Port: " << parser.get<"port">().value() << "\n";
    }
    std::cout << "Hello, " << parser.get<"name">().value() << "!\n";
    return 0;
}
```

## Advanced Features

### Subcommands
Commands act as isolated "mini-parsers" with their own options.

```c++
auto parser = ArgumentParser("App", "Desc",
    cmd<"config">("Configure settings",
        optString<"key">('k', "key", "Config key"),
        optString<"val">('v', "value", "Config value")
    )
);

// Accessing subcommand data
if (parser.has<"config">()) {
    auto& config = parser.get<"config">();
    std::cout << "Key: " << config.get<"key">().value() << "\n";
}
```

### Positional Arguments
Capture arguments that are not associated with a specific flag.

```c++
auto parser = ArgumentParser("App", "Desc",
    optPositional<std::string_view>("Input files")
);

auto result = parser.parse(argc, argv);
for (const auto& file : parser.get<"positional">()) {
    std::cout << "File: " << file << "\n";
}
```

### Custom Callbacks
Execute logic immediately when an option is encountered.

```c++
auto parser = ArgumentParser("App", "Desc",
    optCallback<"print">('p', "print", "Print something", []() {
        std::cout << "Action triggered!\n";
    })
);
```

### Custom Types
To support custom types, specialize the `etched::deserialize` function.

```c++
struct Color { int r, g, b; };

namespace etched {
template <>
auto deserialize(std::string_view str, Color*) -> Result<Color> {
    // Parsing logic...
    if (success) return ok(Color{...});
    return err<Color>("Invalid color format");
}
}
```

## Technical Architecture

### Symbol Table & Perfect Hashing
Etched uses a `consteval` process to find a perfect hash salt for your option flags. At runtime, looking up a flag is a single salted hash and an array access, providing $O(1)$ performance.

### Jump Table
Instead of complex `if/else` or `switch` chains, Etched employs a jump table (an array of function pointers). The index retrieved from the Symbol Table allows the parser to "jump" directly to the handler for that specific option.

### Zero-Copy Lexer
The lexer uses `std::string_view` to point directly to the memory provided in `argv`. No strings are copied or allocated during the core parsing loop.

## Limitations

- **Boolean Flag Assignments**: Boolean flags do not support any form of assignment (e.g., `--flag=true`, `-f true`).
- **Short Flag Joined Values**: Joined values are supported for options that take a value (e.g., `-p8080`). For boolean flags, attached characters are treated as a cluster of other flags.
- **Clustered execution**: If a cluster of flags (e.g., `-hc`) contains an option that triggers an exit (like `help`), subsequent flags in that same cluster are ignored.
- **Callback restrictions**: Callbacks must be stateless or capture only compile-time known values.
- **Fixed Help Layout**: Help columns are preconfigured at compile-time and do not adapt to runtime terminal width.

## Next
[User guide](./user_guide.md)
