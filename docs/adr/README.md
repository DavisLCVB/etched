# Architecture Decision Records

Each record captures one decision, its context, and its consequences.
Records are immutable once accepted: to change a decision, write a new record
that supersedes the old one and update the old record's status.

Statuses: `Proposed`, `Accepted`, `Deferred by NNNN`, `Superseded by NNNN`,
`Deprecated`.

| ADR | Title | Status |
| --- | ----- | ------ |
| [0001](0001-record-architecture-decisions.md) | Record architecture decisions | Accepted |
| [0002](0002-schema-free-lexer.md) | Schema-free lexer | Accepted |
| [0003](0003-negative-numbers-as-values.md) | Negative numbers as separate values | Accepted |
| [0004](0004-interleaved-positionals.md) | Interleaved options and positionals | Accepted |
| [0005](0005-global-options.md) | Global options | Deferred by 0011 |
| [0006](0006-option-identity-in-type.md) | Option identity and traits in the type, values in the object | Accepted |
| [0007](0007-runtime-default-values.md) | Default values are runtime values | Accepted |
| [0008](0008-value-sources.md) | Value sources and precedence | Proposed |
| [0009](0009-ctstring-conversions.md) | CTString conversions | Accepted |
| [0010](0010-repetition-policies.md) | Repetition policies | Accepted |
| [0011](0011-tokens-after-subcommand.md) | Tokens after a subcommand belong to the subcommand | Accepted |
