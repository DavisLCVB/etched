# User guide for Etched

## Basic features

### 1. Define Options

Etched provides many ways to define options.

| helper | description | usage |
|---|---|---|
| `opt<T, Tag>` | Defines a generic option of type T. | `opt<int, "port">('p', "port", "desc", some(8080))` |
| `optInt<Tag>` | Defines an integer-based option. | `optInt<"port">('p', "port", "desc", some(8080))` |
| `optString<Tag>` | Defines a string-based option. | `optString<"name">('n', "name", "desc", "Guest")` |
| `optBool<Tag>` | Defines a boolean flag (true if present). | `optBool<"v">('v', "verbose", "desc")` |
| `optFloat<Tag>` | Defines a floating-point option. | `optFloat<"threshold">('t', "threshold", "desc", some(0.5))` |
| `optCallback<Tag>` | Defines an option that triggers a callback. | `optCallback<"help">('h', "help", "desc", [](){...})` |
| `optHelp` | Defines a standard help option. | `optHelp('h', "help")` |
| `optVersion` | Defines a standard version option. | `optVersion("1.0.0", 'v', "version")` |
| `optPositional<T>` | Defines a collector for positional arguments. | `optPositional<std::string_view>("Files")` |
| `cmd<Tag>` | Defines a subcommand with its own options. | `cmd<"config">("desc", optString<"k">(...))` |

### 2. Help and Version

Etched makes it easy to add standard help and version flags.

```cpp
auto parser = ArgumentParser("App", "Description",
    optHelp('h', "help"),
    optVersion("1.0.0", 'v', "version")
);
```

- `optHelp` automatically generates a help message based on your defined options and prints it when the flag is encountered.
- `optVersion` prints the provided version string.

> [!NOTE]
> Version is not present in the help, this will be added in the future

### 3. Accessing with tags

Values are accessed using the unique string tags defined during option creation.

```cpp
// Check if an option was provided
if (parser.has<"name">()) {
    // Get the value (returns an Optional<T>)
    auto name = parser.get<"name">().value();
}

// Boolean flags return bool directly via get
if (parser.get<"verbose">()) {
    // ...
}
```

### 4. Work with subcommands

Subcommands allow you to group related options.

```cpp
auto parser = ArgumentParser("tool", "desc",
    cmd<"build">("Build the project",
        optBool<"release">('r', "release", "Build in release mode")
    )
);

if (parser.has<"build">()) {
    auto& buildCmd = parser.get<"build">();
    if (buildCmd.get<"release">()) {
        // ...
    }
}
```

### 5. Handling results

The `parse` method returns a `Result<Output>`. You should always check if the parsing was successful.

```cpp
auto result = parser.parse(argc, argv);
if (!result.isOk()) {
    result.unwrapErr().print(); // Prints error message to stderr
    return 1;
}

if (result.unwrap().shouldExit) {
    return 0; // e.g., after printing help or version
}
```

### 6. Positional arguments

Positional arguments are captured using `optPositional`.

```cpp
auto parser = ArgumentParser("app", "desc",
    optPositional<std::string_view>("Files to process")
);

// Access as a collection
for (auto file : parser.get<"positional">()) {
    std::cout << "Processing: " << file << "\n";
}
```

## Advanced features 

### 1. Define your own types

To support custom types, you need to specialize the `etched::deserialize` function in the `etched` namespace.

```cpp
struct Point { int x, y; };

namespace etched {
template <>
auto deserialize(std::string_view str, Point*) -> Result<Point> {
    // Logic to parse "x,y" string...
    if (success) return ok(Point{x, y});
    return err<Point>("Invalid point format");
}
}

// Usage
auto parser = ArgumentParser("app", "desc",
    opt<Point, "origin">('o', "origin", "Coordinates")
);
```

### 2. Create custom callbacks

Callbacks allow you to execute logic as soon as an option is matched during the parsing process.

```cpp
auto parser = ArgumentParser("app", "desc",
    optCallback<"ping">('p', "ping", "Execute ping", []() {
        std::cout << "Pong!\n";
        // Optionally return Result<Output> to control flow
    })
);
```

### 3. Hierarchical subcommands

Etched supports nesting commands as deep as you need.

```cpp
auto parser = ArgumentParser("git", "desc",
    cmd<"remote">("Manage remotes",
        cmd<"add">("Add a remote",
            optString<"name">('n', "name", "Remote name"),
            optString<"url">('u', "url", "Remote URL")
        )
    )
);

if (parser.has<"remote">()) {
    auto& remote = parser.get<"remote">();
    if (remote.has<"add">()) {
        auto& add = remote.get<"add">();
        // ...
    }
}
```

## Configuration

Etched is highly configurable through C++ policy-based design. The `ArgumentParser` class accepts an optional template parameter that defines the behavior of the lexer, parser, and internal tables.

### 1. Creating a Custom Config

The easiest way to customize Etched is to inherit from `etched::DefaultConfig` and override only the parts you need.

```cpp
#include <etched/etched.hpp>

// 1. Define your custom parameters
struct MyCustomConfig : etched::DefaultConfig {
    // Override the max number of arguments allowed
    static constexpr size_t maxArgs = 50;

    // Customize the Lexer behavior
    struct MyLexerConfig : etched::DefaultLexerConfig {
        static constexpr bool allowClusters = false; // Disable -abc -> -a -b -c
        static constexpr bool prefferValues = true;  // Treat flags as values if expected
    };
    
    // Re-bind the Lexer with your new config
    using Lexer = etched::detail::DefaultLexer<MyLexerConfig>;

    // Customize Help generation
    struct MyHelpConfig : etched::DefaultHelpConfig {
        static constexpr size_t maxHelpSize = 2048;
        static constexpr size_t colWidth = 30;
    };
    using helpConfig = MyHelpConfig;
};

// 2. Apply it to the ArgumentParser
int main(int argc, const char* argv[]) {
    using namespace etched;
    
    auto parser = ArgumentParser(WithConfig<MyCustomConfig>{}, "MyApp", "Desc",
        optBool<"v">('v', "verbose", "Enable logs")
    );
    
    // ...
}
```

### 2. Configurable Components

When you create a custom config, you can control these key areas:

| Component | Description |
|---|---|
| `Lexer` | Controls how the input `argv` is tokenized (clustering, `=` handling, etc.). |
| `Parser` | Defines the grammar and state machine logic for parsing tokens. |
| `SymbolTable` | Configuration for the perfect hash search (salt attempts, table factor). |
| `helpConfig` | Controls the layout and static buffer size for the help message. |
| `maxArgs` | A safety limit for the number of tokens processed to prevent resource exhaustion. |

### 3. Why use custom configuration?

- **Binary Size**: Reduce `maxHelpSize` if you are on an embedded system with very limited memory.
- **Strictness**: Disable `allowClusters` if you want to force users to type `-a -b -c` instead of `-abc`.
- **Performance**: Adjust `SymbolTable` parameters if you have hundreds of options and the compile-time salt search is taking too long.


## Prev
[Etched](./etched.md)

## Next
[Advanced guide](./advanced_guide.md)
 
