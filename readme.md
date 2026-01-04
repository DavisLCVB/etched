# Etched

A modern, compile-time argument parsing library for C++20. Etched provides zero-overhead command-line argument parsing with type safety and compile-time validation.

## Features

- **Compile-time instantiation**: All options are instantiated at compile time for maximum performance
- **Type-safe**: Strong compile-time type checking with C++20 concepts
- **Tag-based access**: Access options using compile-time string tags
- **Custom type support**: Easy integration with user-defined types
- **Header-only**: No compilation required, just include and use
- **Custom Behavior**: Customize parsing and sanitization via pluggable ParserStrategy and SanitizerStrategy templates
- **Zero dependencies**: Only requires a C++20 compliant compiler

### Compile-time Instantiation

When you define an argument, it is instantiated at compile-time. Every option you define will be "etched" into the final binary of your program, ensuring parsing begins as quickly as possible at runtime with minimal overhead.

### Tag-based Access

Access your parsed arguments using compile-time string tags instead of runtime lookups. This provides type safety and enables the compiler to optimize away unnecessary code paths.

### Type Safety

Full compile-time type checking ensures you can't accidentally access an option with the wrong type. The compiler catches errors before your code runs.

### Custom Type Support

Extend Etched to parse any custom type by implementing a simple `fromStr<T>()` specialization. Perfect for configuration objects, mathematical types, or domain-specific values.

## Requirements

- C++20 compliant compiler
  - GCC 10+
  - Clang 12+
  - MSVC 19.29+ (Visual Studio 2019 16.10+)
- CMake 3.14+ (for building/installing)

## Installation

### Using FetchContent

Add this to your CMakeLists.txt:

```cmake
include(FetchContent)

FetchContent_Declare(
  etched
  GIT_REPOSITORY https://github.com/DavisLCVB/etched.git
  GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(etched)

target_link_libraries(your_target PRIVATE etched::etched)
```

### Using find_package (after installation)

Install the library:

```bash
cmake -B build
cmake --install build --prefix /usr/local
```

Use in your project:

```cmake
find_package(etched 1.0 REQUIRED)

target_link_libraries(your_target PRIVATE etched::etched)
```

### Header-only

Since Etched is header-only, you can also just copy the `include/etched` directory into your project and add it to your include path:

```cmake
target_include_directories(your_target PRIVATE path/to/etched/include)
```

## Quick Start

```cpp
#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
    using namespace etched;

    auto parser = ArgumentParser(
        optInt<"port">("-p", "--port", "Server port", 8080),
        optString<"host">("-h", "--host", "Server host", "localhost")
    );

    parser.parse(argc, argv);

    std::cout << "Host: " << parser.getOption<"host">().value.value() << "\n";
    std::cout << "Port: " << parser.getOption<"port">().value.value() << "\n";

    return 0;
}
```

Usage:
```bash
./myapp --host 127.0.0.1 --port 3000
# Output:
# Host: 127.0.0.1
# Port: 3000
```

## Usage

### Defining Options

Etched provides convenient helper functions for common types:

```cpp
// Integer types
optInt<"count">("-c", "--count", "Item count", 10)
optInt<"size", uint64_t>("-s", "--size", "Size in bytes", 1024)

// Floating point
optFloat<"rate">("-r", "--rate", "Rate multiplier", 1.5)
optFloat<"pi", double>("-p", "--pi", "Pi value", 3.14159)

// Strings
optString<"name">("-n", "--name", "User name", "guest")

// Booleans
optBool<"verbose">("-v", "--verbose", "Enable verbose output")

// Special options
optHelp("-h", "--help")
optVersion("1.0.0", "-V", "--version")
```

For custom types, use the generic `opt<T, "tag">()` function:

```cpp
opt<MyType, "custom">("-m", "--my-option", "Description", defaultValue)
```

### Parsing Arguments

Simply call `parse()` on your ArgumentParser:

```cpp
auto parser = ArgumentParser(/* options */);
parser.parse(argc, argv);
```

For better error handling:

```cpp
try {
    parser.parse(argc, argv);
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
```

### Accessing Values

Use the tag you defined to access option values:

```cpp
auto option = parser.getOption<"port">();

// Access the parsed value (returns std::optional)
int port = option.value.value();

// With default fallback
int port = option.value.value_or(8080);

// Check if value was provided
if (option.value.has_value()) {
    // Value was provided by user
}

// Access other properties (all are std::optional)
const char* shortName = option.shortName.value();         // "-p"
const char* longName = option.longName.value();           // "--port"
const char* desc = option.description.value();            // "Server port"
int defaultVal = option.defaultValue.value();             // 8080

// Or with fallbacks
const char* shortName = option.shortName.value_or("");
bool hasDesc = option.description.has_value();
```

### Default Values

All options (except boolean flags) can have default values:

```cpp
optInt<"timeout">("-t", "--timeout", "Timeout in seconds", 30)  // default: 30
optString<"config">("-c", "--config", "Config file", "app.conf")  // default: "app.conf"
```

Boolean flags default to `false` and become `true` when present.

### Required vs Optional Arguments

By default, all options are optional (they use their default values if not provided). You can check if a value was explicitly provided:

```cpp
auto opt = parser.getOption<"config">();
if (opt.value.has_value()) {
    // User explicitly provided this option
} else {
    // Using default value
}
```

For required options, check after parsing:

```cpp
parser.parse(argc, argv);

if (!parser.getOption<"input">().value.has_value()) {
    std::cerr << "Error: --input is required\n";
    return 1;
}
```

### Custom Types

To use custom types, specialize the `fromStr` template in the `etched` namespace:

```cpp
struct Point2D {
    double x;
    double y;
};

namespace etched {
    template <>
    auto fromStr<Point2D>(const char* str) -> Point2D {
        Point2D result;
        if (sscanf(str, "%lf,%lf", &result.x, &result.y) != 2) {
            throw std::invalid_argument("Point2D format: x,y");
        }
        return result;
    }
}

// Now use it:
int main(int argc, const char* argv[]) {
    using namespace etched;

    auto parser = ArgumentParser(
        opt<Point2D, "position">("-p", "--position", "2D Position", Point2D{0.0, 0.0})
    );

    parser.parse(argc, argv);

    auto pos = parser.getOption<"position">().value.value();
    std::cout << "Position: (" << pos.x << ", " << pos.y << ")\n";

    return 0;
}
```

Usage:
```bash
./myapp --position 3.14,2.71
# Output: Position: (3.14, 2.71)
```

### Error Handling

Etched throws exceptions for parsing errors:

```cpp
try {
    parser.parse(argc, argv);
} catch (const std::invalid_argument& e) {
    // Invalid option value or format
    std::cerr << "Invalid argument: " << e.what() << "\n";
    return 1;
} catch (const std::exception& e) {
    // Other errors
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
}
```

## Examples

See the `examples/` directory for complete working examples:

### Basic Example

```cpp
#include <etched/etched.hpp>
#include <iostream>

int main(int argc, const char* argv[]) {
    using namespace etched;

    auto parser = ArgumentParser(
        optInt<"port">("-p", "--port", "Server port", 8080),
        optString<"host">("-h", "--host", "Server host", "localhost")
    );

    parser.parse(argc, argv);

    std::cout << "Host: " << parser.getOption<"host">().value.value() << "\n";
    std::cout << "Port: " << parser.getOption<"port">().value.value() << "\n";

    return 0;
}
```

### Advanced Example

```cpp
using namespace etched;

auto parser = ArgumentParser(
    optInt<"count", int32_t>("-c", "--count", "Item count", 10),
    optFloat<"rate", double>("-r", "--rate", "Rate multiplier", 1.5),
    optString<"name">("-n", "--name", "User name", "guest"),
    optInt<"size", uint64_t>("-s", "--size", "File size", 1024),
    optBool<"verbose">("-v", "--verbose", "Verbose output"),
    optHelp("-h", "--help")
);

parser.parse(argc, argv);

// Access all values
auto count = parser.getOption<"count">().value.value();
auto rate = parser.getOption<"rate">().value.value();
auto name = parser.getOption<"name">().value.value();
auto size = parser.getOption<"size">().value.value();
auto verbose = parser.getOption<"verbose">().value.value_or(false);
```

### Custom Type Example

See `examples/11-custom-types.cpp` for a complete example with Point2D and Color types.

## API Reference

### ArgumentParser

Main parser class that accepts a variadic list of options:

```cpp
auto parser = ArgumentParser(option1, option2, ...);
void parse(int argc, const char* argv[]);
template<FixedString Tag> auto getOption();
```

### Option Helper Functions

- `optInt<"tag">(short, long, desc, default)` - Integer option
- `optInt<"tag", T>(short, long, desc, default)` - Typed integer (int32_t, uint64_t, etc.)
- `optFloat<"tag">(short, long, desc, default)` - Float option
- `optFloat<"tag", T>(short, long, desc, default)` - Typed float (float, double)
- `optString<"tag">(short, long, desc, default)` - String option
- `optBool<"tag">(short, long, desc)` - Boolean flag
- `opt<T, "tag">(short, long, desc, default)` - Generic option for custom types
- `optHelp(short, long)` - Help option
- `optVersion(version, short, long, description)` - Version option (version is required)
- `optCallback<"tag", CallbackType>(short, long, desc, callback)` - Option with custom callback

### Accessing Parsed Values

```cpp
auto option = parser.getOption<"tag">();
// Properties:
// - option.value          // std::optional<T>
// - option.defaultValue   // std::optional<T>
// - option.shortName      // std::optional<const char*>
// - option.longName       // std::optional<const char*>
// - option.description    // std::optional<const char*>
// - option.tag            // compile-time string (static constexpr)
```

### Custom Type Conversion

Specialize in the `etched` namespace:

```cpp
namespace etched {
    template <>
    auto fromStr<YourType>(const char* str) -> YourType {
        // Parse str and return YourType
        // Throw std::invalid_argument on error
    }
}
```

### ArgumentParser Customization

The `ArgumentParser` accepts template parameters for customization:

```cpp
template <ParserStrategy Strategy = detail::DefaultParserStrategy,
          SanitizerStrategy Sanitizer = detail::BasicSanitizer,
          IsOption... Options>
class ArgumentParser;
```

#### DefaultParserStrategy

The default parser provides:

- **Automatic help generation**: When `optHelp()` is used, it automatically generates and displays help at runtime by iterating through all options and printing their flags and descriptions
- **Terminal options**: Special handling for help and version options
- **Unix-style parsing**: Supports `--long` and `-short` option formats
- **Boolean flags**: Automatic detection of boolean options (no value required)
- **Callbacks**: Support for options with custom callbacks via `optCallback()`

Example with automatic help:

```cpp
auto parser = ArgumentParser(
    optInt<"port">("-p", "--port", "Server port", 8080),
    optString<"host">("-h", "--host", "Server host", "localhost"),
    optBool<"verbose">("-v", "--verbose", "Enable verbose output"),
    optHelp(std::nullopt, "--help")
);

parser.parse(argc, argv);
```

Running with `--help` automatically prints:
```
-p, --port <value>    Server port
-h, --host <value>    Server host
-v, --verbose    Enable verbose output
--help
```

#### BasicSanitizer

The default sanitizer validates arguments at runtime:

- Ensures all arguments are valid printable ASCII (32-126)
- Allows tabs, newlines, and carriage returns
- Rejects control characters and null/empty arguments
- Throws `std::invalid_argument` for invalid input

#### Custom Strategies

You can implement custom `ParserStrategy` or `SanitizerStrategy` by satisfying their respective concepts:

```cpp
// Custom parser for different argument styles
struct CustomParser {
    template <std::size_t N, IsOption... Options>
    static auto parse(int argc, std::array<const char*, N> argv, Options&... opts) -> void {
        // Your custom parsing logic
    }
};

// Custom sanitizer for stricter validation
struct StrictSanitizer {
    template <std::size_t N>
    static constexpr auto sanitizeArgs(int argc, const char* argv[])
        -> std::pair<std::array<const char*, N>, int> {
        // Your custom sanitization logic
    }
};

// Use custom strategies
auto parser = ArgumentParser<CustomParser, StrictSanitizer>(/* options */);
```

## Performance

Etched is designed for zero-overhead parsing:

- All option metadata is instantiated at compile-time
- Tag-based access allows complete inlining and optimization
- No dynamic allocations during parser creation
- Minimal runtime overhead for parsing

## Limitations

- Requires C++20 (for template non-type parameters with class types)
- Tags must be compile-time constants
- No built-in support for subcommands (can be implemented via custom ParserStrategy)
- Default help message is basic (customize via custom ParserStrategy for advanced formatting)

## Building Examples and Tests

```bash
# Build with examples
cmake -B build -DETCHED_BUILD_EXAMPLES=ON
cmake --build build

# Build with tests
cmake -B build -DETCHED_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

