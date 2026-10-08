# 0004. Interleaved options and positionals

- Date: 2026-10-08

## Context

GNU-style parsers let options and positionals appear in any order
(`app in.txt -v out.txt`). POSIX-style parsers stop option parsing at the
first positional, which suits pass-through tools such as `env` or `ssh`.

## Decision

Options and positionals may always be interleaved. Version 3 has no strict
POSIX mode. Pass-through use cases rely on the `--` terminator.

## Alternatives considered

- **Configurable strict mode.** Rejected for now: it duplicates parser logic
  and tests for a case that `--` already covers. It can be added later as a
  per-command setting.

## Consequences

- One parsing algorithm and one set of tests.
- Programs that forward arguments to another program must document `--`.
