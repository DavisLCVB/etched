# 0011. Tokens after a subcommand belong to the subcommand

- Date: 2026-10-08

## Context

[ADR 0005](0005-global-options.md) designed global options that a subcommand
also accepts after its name (`app build --verbose`). That design requires
every subcommand symbol table to include the global options of all its
ancestors, and the parsing context to carry a reference per nesting level.

`OptionsConfig::allowUnorderedGlobalOption` was introduced to control that
behavior, defaulting to `false`.

## Decision

- Every token after a subcommand belongs to that subcommand. A parent option
  after the subcommand is an error: `app --verbose build` is valid,
  `app build --verbose` is not.
- Global options accepted after a subcommand are **deferred** and out of scope
  for the first v3 release. ADR 0005 keeps the design for when the feature is
  revisited.

## Alternatives considered

- **Accept global options after the subcommand by default.** Rejected: it
  breaks the rule that the position of a token defines its owner, and the
  inherited symbol tables grow with every nesting level.

## Consequences

- Symbol tables only contain the options of their own command.
- The parsing context does not need references to ancestor storage.
- Programs that want an option on every level must declare it in each
  subcommand.
- `allowUnorderedGlobalOption` and the `isGlobal` trait have no effect while
  the feature is deferred.
