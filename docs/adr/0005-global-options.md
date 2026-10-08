# 0005. Global options

- Status: Accepted
- Date: 2026-10-08

## Context

Some options, such as `--verbose`, are meaningful for every subcommand. The
parsing context is passed down to each subcommand, so a global option parsed
inside a subcommand must reach the command that declares it.

## Decision

- An option can be declared global (`isGlobal` trait). Global options are
  visible in every descendant subcommand.
- The value is stored in a **single slot owned by the declaring command**.
  Subcommands do not store a copy.
- The symbol table of each subcommand is generated at compile time including
  the inherited global options. Each inherited entry records which ancestor
  owns the slot.
- The parsing context carries a reference to the ancestors' storage, not a copy
  of their options.
- A subcommand cannot declare an option whose short or long name collides with
  an inherited global option. This is a compile-time error.
- Repeated occurrences across levels follow the option's repetition policy, as
  any other repetition.

## Alternatives considered

- **Recursive lookup on read.** Rejected: every read costs O(depth) and needs a
  rule to choose between occurrences at different levels.
- **Copying globals into the parsing context.** Rejected: it adds storage and
  copies without any benefit over a reference.

## Consequences

- Reading a global option is O(1) from its owner.
- Subcommand symbol tables grow with the inherited entries.
- `isGlobal` must be a type-level trait because it changes the generated
  tables ([0006](0006-option-identity-in-type.md)).
- Whether a global option may appear after the subcommand by default is
  controlled by `OptionsConfig::allowUnorderedGlobalOption`. Its final default
  is not decided yet.
