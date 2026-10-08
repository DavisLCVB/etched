# 0002. Schema-free lexer

- Status: Accepted
- Date: 2026-10-08

## Context

Some tokens are ambiguous without the option schema:

- `-fxz` is `-f xz` when `-f` takes a value, and `-f -x -z` when it is a flag.
- `-5` is a value for a numeric option, or a short option `-5` (see
  [0003](0003-negative-numbers-as-values.md)).

Making the lexer schema-aware would spread these rules across two components.

## Decision

Classification happens in two stages:

1. The **lexer** classifies each token by shape only: `Terminator` (`--`),
   `LongWithValue` (`--name=value`), `Long` (`--name`), `ShortGroup`
   (`-chars`), and `Word` (anything else, including `-` alone and every token
   after `--`).
2. The **parser** interprets each lexical kind with the schema: flag groups,
   attached short values, negative numbers, option values, positionals, and
   subcommands.

## Consequences

- The lexer can be tested without a schema.
- Every syntactic ambiguity is resolved in one place, the parser.
- See `docs/dev/option_types.md`, section "Token classification".
