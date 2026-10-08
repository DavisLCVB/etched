# 0010. Repetition policies

- Status: Accepted
- Date: 2026-10-08

## Context

An option or flag can appear more than once on the command line. Libraries
differ: clap, CLI11, and Boost.program_options reject repeated single-value
options by default, while Python's argparse keeps the last value.

Some policies only make sense for one kind of option: counting (`-vvv`)
applies to flags, and accumulating values (`-I a -I b`) applies to options
whose value type is a container.

## Decision

- A repeated occurrence is an **error by default**. The policy is configurable
  per option.
- Policies are split by option kind, so invalid combinations cannot be
  written:
  - `OptionRepetition`: `THROW_ERROR`, `TAKE_FIRST`, `TAKE_LAST`, `ACCUMULATE`.
  - `FlagRepetition`: `THROW_ERROR`, `TAKE_FIRST`, `TAKE_LAST`, `COUNT`.
  - Positionals have no repetition policy; their arity is defined by
    `PositionalPolicy`.
- `ACCUMULATE` is only valid when the value type is a container. Each
  occurrence is deserialized as one element and appended. Using it with a
  non-container type is a compile-time error, checked where the value type and
  the traits are both known.
- A negated occurrence (`--no-NAME`) counts as an occurrence of the same flag
  and follows its policy.

## Alternatives considered

- **One shared enum validated with `static_assert`.** Rejected: the error is
  reported away from where the trait is written, and the enum offers values
  that cannot be used.
- **Accumulating automatically when the value type is a container.** Rejected:
  `std::string` satisfies a container concept, and a custom deserializer may
  build a container from a single token.
- **Negation always overrides earlier occurrences.** Rejected: it adds an
  exception to the policy rule.

## Consequences

- The shared values (`THROW_ERROR`, `TAKE_FIRST`, `TAKE_LAST`) are declared in
  both enums. Parser code that must handle both is written as a template over
  the policy type.
- With the default policy, `--verbose --no-verbose` fails. Overriding a flag
  from a wrapper or alias requires `TAKE_LAST`.
