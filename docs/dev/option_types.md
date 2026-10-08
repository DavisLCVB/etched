# Option Types

> Status: **draft for v3**. This document defines the command-line syntax the
> parser accepts. It intentionally ignores the current (v2) implementation.

## Conventions

- `argv[0]` is the program name and is never parsed.
- A **short name** is a single ASCII alphanumeric character: `[A-Za-z0-9]`.
- A **long name** matches `[a-z0-9][a-z0-9-]*` and cannot end in `-`.
- Names are case-sensitive. Long-name abbreviations (`--verb` for `--verbose`)
  are **not** accepted.
- Single-dash long names (`-verbose`) are **not** accepted. `-verbose` is read
  as a group of short options (see [Flag Option](#flag-option)).
- The token `--` ends option parsing. Every later token is positional.
- The token `-` alone is a regular value (commonly stdin/stdout), never an
  option.

### Token classification

Classification happens in two stages:

1. **Lexer (schema-free).** Each token is classified by its shape only, in
   this order:

   | Token shape    | Lexical kind  |
   | -------------- | ------------- |
   | `--`           | `Terminator`  |
   | `--name=value` | `LongWithValue` |
   | `--name`       | `Long`        |
   | `-` (alone)    | `Word`        |
   | `-chars`       | `ShortGroup`  |
   | anything else  | `Word`        |

   After `--`, every token is a `Word` regardless of its shape.

2. **Parser (schema-aware).** The parser interprets each lexical kind using
   the declared options: it decides whether a `ShortGroup` is a group of flags,
   a short option with an attached value, or a negative number, and whether a
   `Word` is an option value, a positional, or a subcommand.

The lexer never needs the schema. Every ambiguity is resolved in the parser.

## Basic Option

An option that takes exactly one value per occurrence.

Accepted forms:

```text
-o VALUE        short, separate value
-oVALUE         short, attached value
--output VALUE  long, separate value
--output=VALUE  long, attached value
```

Rules:

- A missing value is an error: `app --output` (end of input) or
  `app --output --verbose`.
- A separate value is **not** consumed if it looks like an option (starts with
  `-` and is not `-` alone). Values that start with `-` must use the attached
  form: `--output=-weird-name` or `-o-weird-name`.
- Exception, negative numbers: when the option's value type is numeric, a
  separate token that is a valid negative number (`-5`, `-0.25`) is consumed
  as the value, unless it matches a declared short option of the current
  scope. Example: `--offset -5` is valid for an integer `--offset`. Enabled
  by default; it can be disabled with `OptionsConfig::allowNegativeNumbers`.
- `--output=` sets an explicit empty value.
- `-o=VALUE` gives the value `=VALUE` (attached-form rule); it is not a
  special case.
- Repeated occurrences follow the option's repetition policy. The default is
  an error: `--port 80 --port 8080` fails. The policy can be changed to keep
  the first occurrence, keep the last one, or accumulate every occurrence.
- Accumulation is only valid when the value type is a container, such as
  `std::vector<std::string>`. Each occurrence is deserialized as one element
  and appended: `-I a -I b` gives `{"a", "b"}`. Declaring accumulation with a
  non-container value type is a compile-time error.
- The syntax is always one value per occurrence. Comma-separated lists are not
  split by the parser; a custom deserializer may parse one token into a
  container instead.

## Flag Option

An option that takes no value. Its presence is the information.

Accepted forms:

```text
-v
--verbose
-abc            group: same as -a -b -c
-vvv            repetition inside a group (only for counting flags)
--no-verbose    negation (only for negatable flags)
```

Rules:

- `--verbose=VALUE` is an error.
- In a group, every character must be a short flag, except the **last** one,
  which may be a basic option that consumes the rest of the token or the next
  token: `-xzf archive.tar` and `-xzfarchive.tar` are both valid.
- A basic option in the middle of a group consumes the rest of the group as
  its value: `-fxz` means `-f xz` when `-f` takes a value.
- Repeating a flag follows its repetition policy. The default is an error:
  `-v -v` fails. The policy can be changed to accept repetitions, or to count
  them (`-vvv`).
- `--no-NAME` exists only when the flag is declared negatable. A negated
  occurrence counts as an occurrence of the same flag and follows its
  repetition policy: with the default, `--verbose --no-verbose` fails; when
  the policy keeps the last occurrence, it results in `false`.

## Positional Option

A value identified by its position instead of a name.

Rules:

- Positionals are filled in declaration order with the tokens classified as
  values that are not consumed by an option.
- Options and positionals may be interleaved:
  `app in.txt -v out.txt` is the same as `app -v in.txt out.txt`.
- Only the **last** positional may be variadic (one or more, or zero or more).
- A required positional that is never filled is an error.
- Extra values with no positional left to receive them are an error.
- `--` forces the remaining tokens to be positional: `app -- -v` gives the
  value `-v` to the first positional.

## Subcommand

A named scope with its own options, positionals, and subcommands.

```text
app [app options] build [build options] [build positionals]
```

Rules:

- A subcommand is matched by the **first value token** at the current level.
  Its name follows the long-name rules.
- Tokens before the subcommand belong to the parent. Tokens after it belong to
  the subcommand.
- A parent option after the subcommand is an error: `app build --verbose`
  fails when `--verbose` is declared by `app`. Global options that are
  accepted after a subcommand are deferred
  ([ADR 0011](../adr/0011-tokens-after-subcommand.md)).
- A command with subcommands cannot declare positionals. This removes the
  ambiguity between a subcommand name and a positional value.
- An unknown value where a subcommand is expected is an error
  (`unknown command 'biuld'`). Suggestions may be offered, but subcommand
  abbreviations are not accepted.
- Subcommands nest without a depth limit: `app remote add origin URL`.

## Errors

| Situation                                   | Example                    |
| ------------------------------------------- | -------------------------- |
| Unknown option                              | `app --colour`             |
| Missing value                               | `app --output`             |
| Value given to a flag                       | `app --verbose=yes`        |
| Unknown character in a short group          | `app -vq` (no `-q`)        |
| Missing required positional                 | `app` with `<input>`       |
| Unexpected extra positional                 | `app a b` with one `<input>` |
| Unknown subcommand                          | `app biuld`                |
| Parent option after subcommand              | `app build --config x`     |

## Related Decisions

- [ADR 0002](../adr/0002-schema-free-lexer.md): schema-free lexer.
- [ADR 0003](../adr/0003-negative-numbers-as-values.md): negative numbers as
  separate values.
- [ADR 0004](../adr/0004-interleaved-positionals.md): interleaved options and
  positionals.
- [ADR 0005](../adr/0005-global-options.md): global options (deferred).
- [ADR 0011](../adr/0011-tokens-after-subcommand.md): tokens after a
  subcommand belong to the subcommand.
