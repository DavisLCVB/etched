# 0009. CTString conversions

- Date: 2026-10-08

## Context

`CTString` had implicit conversions to both `std::string_view` and
`const char*`. Verified consequences:

- Calls overloaded on `std::string_view` and `const char*` are ambiguous.
- A `CTString` converts silently to `bool` through the pointer.

## Decision

- Keep the implicit conversion to `std::string_view`, the natural type of the
  parser.
- Replace the implicit pointer conversion with an explicit `c_str()`.
- Defining `ETCHED_CTSTRING_IMPLICIT_CSTR` restores the implicit pointer
  conversion for code that depends on it, accepting the ambiguities above.
- Constructing a `CTString` from a `std::array` without a null terminator is a
  compile-time error.

## Consequences

- Code that relied on the implicit pointer conversion must call `c_str()` or
  define the compatibility macro.
