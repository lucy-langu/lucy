# ADR 0001: Runtime subsystem boundaries

- **Status:** Accepted
- **Date:** 2026-11-07
- **Applies to:** Lucy 2.0.7 maintenance and subsequent 2.x work

## Context

The runtime translation unit had accumulated environment lookup, runtime type contracts, execution, object dispatch, imports, process interaction, and REPL integration. That concentration increases compile-time coupling and makes isolated testing and review harder.

## Decision

Keep the existing public `Interpreter` API and runtime semantics, but separate stable runtime responsibilities into focused translation units:

- `environment.cpp`: lexical environment storage, lookup, assignment, and name suggestions.
- `type_system.cpp`: runtime type-name matching and contract enforcement.
- `runtime.cpp`: interpreter execution, evaluation, object dispatch, module integration, and runtime orchestration.
- `value.cpp`: representation and operations of `Value`.
- `lexer.cpp` / `parser.cpp`: frontend stages.

Headers under `include/lucy/` define the shared C++ interfaces. Internal helpers must not be duplicated across translation units.

## Consequences

- The runtime build has clearer subsystem boundaries without changing the language-visible API.
- Type-contract behavior can be tested independently from parser implementation.
- Further decomposition should be incremental and behavior-preserving; do not split files merely to reach an arbitrary line count.
