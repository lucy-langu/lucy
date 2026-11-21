# ADR 0003: Language and release compatibility policy

- **Status:** Accepted
- **Date:** 2026-11-07
- **Applies to:** Lucy 2.x

## Context

The language reference, runtime implementation, standard library, tests, and release metadata have historically drifted. A mature language needs a stable contract and a repeatable process for changing it.

## Decision

`docs/LANGUAGE_SPECIFICATION.md` is the normative description of language-level behavior for the current release. The detailed language reference and API references provide examples and inventories. Executable tests provide conformance evidence, not permission to silently contradict the specification.

- Patch: compatible fixes and corrections; record behavior changes.
- Minor: additive compatible language/runtime/tooling changes.
- Major: breaking syntax, semantics, value representation, or extension contract changes.

Historical verification reports remain historical. Platform support is claimed only for toolchain/OS combinations that have actual build and test evidence.

## Consequences

Every new language feature requires tests and documentation. Every intentional breaking change requires a migration note and appropriate versioning.
