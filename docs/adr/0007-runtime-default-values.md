# 0007. Default values are runtime values

- Date: 2026-10-08

## Context

Requirements, in priority order:

1. Defaults computed at runtime, or a factory evaluated at runtime.
2. Defaults written inline with the option, preferably not as text.
3. No special converters for `std::string`, `std::vector`, or other library
   types, so the mechanism extends to any type.

The result of a constant evaluation cannot contain pointers to memory that is
not static. Verified with GCC and Clang:

- A stored `std::string` fails: long strings point to heap memory
  (`refers to a result of operator new`), and short strings point to their own
  buffer (`pointer to subobject of temporary`, rejected by Clang).
- Any mechanism that keeps the value as data (lambda captures, `std::bind`,
  `std::function`) fails for the same reason.

## Decision

Defaults are ordinary runtime values stored in the option object, enabled by
[0006](0006-option-identity-in-type.md):

- `opt<T, Id>(description, value)` accepts any argument `D` such that
  `std::constructible_from<T, D>`. Examples: `"localhost"`,
  `std::vector{80, 443}`, `std::thread::hardware_concurrency()`.
- A separate overload accepts an invocable that returns `T`, evaluated lazily
  at runtime. Captures are allowed.
- `hasDefault` is a trait selected by the overload, not by the value.
- `get<Tag>()` returns `T` when the option is required or has a default, and
  `Optional<T>` otherwise.
- Options without a default store nothing for it.

## Alternatives considered

- **Default inside the type as a structural `StaticOptional`.** Rejected:
  restricts defaults to structural types and inflates types.
- **Default as text, deserialized at runtime.** Viable and common (cxxopts,
  clap), but loses compile-time type checking and fails requirement 2.
- **Function pointer to a user-written lambda.** Viable, but it does not allow
  inline values.
- **Pointer to a user-declared static object.** Viable and allows computed
  defaults, but requires declaring a separate variable.

## Consequences

- Any constructible type works as a default without specializations.
- Returning `T` for required options is only sound after a successful parse.
  The result API must guarantee that `get` is only reachable after parsing
  succeeded.
