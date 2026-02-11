# Etched Architecture

This document describes the internal design and architectural components of the Etched library.

## Overview

Etched is built around the idea of moving as much work as possible to **compile-time**. By doing so, the runtime overhead of argument parsing is reduced to a minimum ($O(1)$ flag lookups and zero heap allocations).

The library follows a modular design where different components are tied together by the `Orchestrator`.

## Core Components

### 1. The Orchestrator (`DefaultOrchestrator`)
The Orchestrator is the central hub. It manages the lifecycle of the parsing process and holds the instances of the other components. It is configured via a policy-based struct (the `Config` template parameter).

### 2. Symbol Table & Perfect Hashing (`PerfectSymbolTable`)
The Symbol Table is responsible for mapping string flags (like `-v` or `--port`) to internal indices.

- **Compile-time Search**: During compilation, Etched searches for a "salt" value that, when combined with its hashing algorithm, produces no collisions for all defined flags within a fixed-size table.
- **Constant Time Lookup**: At runtime, finding a flag's metadata is a single hash operation and an array access ($O(1)$).
- **Metadata**: Each entry in the table stores the index of the option in the internal storage and its type (Flag, Option, Command, etc.).

### 3. Jump Table (`DefaultJumpTable`)
The Jump Table handles the dispatch of actions once a flag is identified.

- **Static Dispatch**: Instead of using virtual functions or complex `switch` statements, Etched uses an array of function pointers.
- **Type-Specific Handlers**: Each handler is a template function specialized for the specific type of the option (e.g., `int`, `bool`, `std::string_view`).
- **Zero Overhead**: This allows the compiler to inline handlers easily and avoids the cost of dynamic dispatch.

### 4. Lexer (`DefaultLexer`)
The Lexer turns the raw `argv` array into a stream of tokens.

- **Zero-Copy**: It uses `std::string_view` to point directly to the original memory in `argv`. No strings are copied or allocated.
- **Clustering Support**: It can handle clustered short flags (e.g., `-xvf`) by yielding multiple tokens from a single `argv` element.
- **Context Awareness**: It can be told by the parser to treat the next token as a literal value even if it looks like a flag (e.g., for values starting with `-`).

### 5. Parser (`DefaultParser`)
The Parser implements the grammar of the CLI. It orchestrates the Lexer and Jump Table to process tokens in the correct order.

- **State Machine**: It follows a recursive descent-like approach (or a simplified state machine) to handle options, values, subcommands, and positional arguments.
- **Error Propagation**: It uses a `Result<T>` pattern to propagate errors without using exceptions at runtime.

## Data Flow

1. **Definition**: The user defines the `ArgumentParser` with a list of options.
2. **Compile-time**:
    - Options are verified for uniqueness.
    - The `SymbolTable` searches for a perfect hash salt.
    - The `JumpTable` is populated with specialized handlers.
    - Help text is pre-generated and stored in a static buffer.
3. **Runtime**:
    - `parser.parse(argc, argv)` is called.
    - The `Lexer` is initialized with the arguments.
    - The `Parser` loop begins:
        - `Lexer` yields a `Token`.
        - `SymbolTable` looks up the `Token` to find its `EntryMetadata`.
        - `JumpTable` dispatches to the handler for that metadata.
        - The handler updates the option's value or executes a callback.
    - The process continues until `END_OF_INPUT` or an error occurs.
4. **Access**: The user calls `parser.get<"tag">()` to retrieve the parsed data.

## Design Decisions

- **Policy-Based Design**: Using template parameters for configuration allows users to swap any component (Lexer, Parser, etc.) with a custom implementation while keeping the rest of the library intact.
- **Result Type**: Inspired by modern languages like Rust, the `Result<T>` class provides a safe way to handle errors without the overhead or control-flow complexity of exceptions.
- **Static Buffers**: Components like the generated help text use `BoundedString` (a fixed-capacity string) to avoid any dynamic memory allocation, ensuring suitability for embedded systems or high-performance applications.

## Limitations

To maintain its high performance and zero-allocation guarantees, Etched has several design-driven limitations:

- **No Assignments for Boolean Flags**: Boolean flags (defined with `optBool`) cannot have values assigned to them via any syntax. For example, `--flag=true`, `--flag true`, or `-f true` will simply enable the flag, and the string "true" will be treated as a separate positional argument.
- **Short Flag Joined Values**: Joined values are supported **only** for options that take a value (e.g., `-p8080` for a port). For boolean flags, any characters following the flag are treated as a cluster of additional short flags (e.g., `-vabc` triggers flags `v`, `a`, `b`, and `c`).
- **Cluster Execution Flow**: When short flags are clustered (e.g., `-hc`), if one flag (like `h` for help) triggers an exit, the rest of the cluster (`c`) is ignored. No error is raised for these skipped flags.
- **Homogeneous Positionals**: All positional arguments captured by `optPositional` must share the same type.
- **Callback Constraints**: While callbacks support lambdas, they must be stateless (no captures) or only capture values available at compile-time to maintain the static nature of the jump table.
- **Static Help Layout**: The help message layout, including column widths, is calculated at compile-time based on configuration. It does not dynamically adjust to the terminal's runtime width.
- **Perfect Hash Potential Collisions**: Although rare, having many options with extremely similar names might make it impossible for the `SymbolTable` to find a perfect hash salt within the configured attempt limit. Increasing the table's `factor` or `maxSaltAttempts` usually resolves this.

