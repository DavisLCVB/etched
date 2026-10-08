# 0008. Value sources and precedence

- Status: Proposed
- Date: 2026-10-08

## Context

Option values may come from sources other than the command line, such as a
configuration file. The schema stays compile-time; only values are injected at
runtime. Defining which options exist from a file at runtime is out of scope
([0006](0006-option-identity-in-type.md)).

## Decision

- Each option has a runtime value slot that records the value and its source.
- Sources, from highest to lowest precedence:
  `command line > environment > configuration file > default`.
- Parsing text into `T` requires a `deserialize` operation from
  `std::string_view`. It is the only requirement for a type to be used in an
  option.
- A `serialize` operation from `T` to text is optional and runtime-only. When
  available, it enables showing the effective value in help and writing the
  resulting configuration to a file.

## Consequences

- File and environment values follow the same conversion path as the command
  line.
- Types without `serialize` cannot show their effective value in help or be
  written back to a file.
