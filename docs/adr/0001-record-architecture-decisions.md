# 0001. Record architecture decisions

- Date: 2026-10-08

## Context

Version 3 redefines the command-line syntax and the option model. Several
decisions depend on non-obvious language constraints (constant evaluation,
structural types). Without a record, the reasoning behind them is lost.

## Decision

Record architecture decisions as ADRs in `docs/adr/`, one file per decision,
numbered sequentially (`NNNN-short-title.md`).

ADRs complement `docs/dev/`, they do not replace it:

- `docs/dev/` holds living specifications that describe the current behavior.
- `docs/adr/` holds immutable decisions. A changed decision gets a new ADR that
  supersedes the previous one.

## Consequences

- Specifications can link to the ADR that justifies each rule.
- Reverting a decision requires writing down why, not only changing the code.
