# 0006. Option identity and traits in the type, values in the object

- Status: Accepted
- Date: 2026-10-08

## Context

The parser aims to be fully compiled: symbol tables, perfect hashing, and
collision checks are computed at compile time, and only the parsing
algorithms run at runtime.

Version 2 builds options through `consteval` functions. A `consteval` call
must produce a constant expression, so **the whole option object** must be a
constant. Any data member that owns memory (`std::string`, `std::vector`,
user types that allocate) cannot be stored, which blocks arbitrary default
values ([0007](0007-runtime-default-values.md)).

## Decision

Split each option into four parts:

| Part | Lives in | Contents |
| ---- | -------- | -------- |
| Identity | type | `tag`, `shortName`, `longName`, option kind (option, flag, positional) |
| Traits | type | value type `T`, `isRequired`, `hasDefault`, `repetitionPolicy`, `hasNegativeForm`, `positionalPolicy`, `isGlobal` |
| Metadata | object | `description`, `metavar`, help group, `hidden`, environment variable name |
| Values | object | default value or factory, parsed value slot and its source |

Classification rule: a property goes in the **type** if it changes a type,
the generated code, or a compile-time check. It goes in the **object** if it
is only read to display help or to resolve a value at runtime.

- The identity is grouped in a single non-type template parameter, a
  structural `Id` struct deduced from braced initialization:
  `opt<std::string, {"host", 'H', "host"}>("Server host", "localhost")`.
- Option objects are built with `constexpr` constructors, not `consteval`.
  They run at runtime and only move values into the object.
- Compile-time checks (duplicate names, global collisions) are `static_assert`
  over the option types.
- Traits are aggregates. Properties fully determined by the option kind, such
  as `requiresValue`, are `static constexpr` and cannot be set per instance.

## Alternatives considered

- **Everything as constexpr data built by `consteval` (version 2).** Rejected:
  blocks defaults of types that own memory.
- **Description in the type.** Rejected: every description length creates a
  distinct type, which inflates symbol names and compile time.
- **Metadata through template specialization keyed by tag.** Rejected: the
  metadata is declared away from the option, tags become global across
  subcommands, and missing specializations cause ODR problems.
- **Full specification inside a lambda type.** Rejected: noisy syntax, and a
  lambda in a header creates a different type per translation unit.

## Consequences

- Option objects contain only runtime data; structural information has no
  per-object cost.
- Help text is generated at runtime from the metadata.
- Names in template arguments increase template instantiations and symbol
  length moderately.
