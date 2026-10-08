# 0003. Negative numbers as separate values

- Status: Accepted
- Date: 2026-10-08

## Context

By default, a separate token that starts with `-` is not consumed as an
option value, so `--offset -5` would fail. Requiring `--offset=-5` is correct
but surprising for numeric options.

## Decision

A separate token that is a valid negative number (`-5`, `-0.25`) is consumed as
the value of the preceding option when:

- the option's value type is numeric, and
- the token does not match a declared short option of the current scope.

For every other value type, values that start with `-` must use the attached
form (`--name=-x`, `-n-x`).

## Consequences

- The decision depends on the value type, so it is taken by the parser, not by
  the lexer ([0002](0002-schema-free-lexer.md)).
- A numeric short option such as `-5` shadows the negative number `-5` in its
  scope.
