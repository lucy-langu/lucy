# ADR 0002: Runtime memory ownership baseline

- **Status:** Accepted as a documented limitation; redesign deferred
- **Date:** 2026-11-07
- **Applies to:** Lucy 2.0.7

## Context

`Value`, `Function`, `Class`, `Instance`, and `Environment` use `std::shared_ptr` for shared lifetime. Functions retain lexical closures through `Function::closure`; environments retain bound values and functions. This permits closures to outlive their defining call, but it also permits reference cycles (for example, an environment binding a function that captures the same environment).

## Decision

Do not replace the ownership model with a tracing collector or weak-closure scheme as part of a maintenance refactor. Either change could invalidate live closures or native extension values without a complete object-graph model and conformance suite.

The current interpreter tracks environments weakly and clears surviving tracked environments during interpreter shutdown to break environment/function cycles. This is shutdown cleanup, not a garbage collector and not a guarantee of prompt reclamation during a long-running REPL session.

## Required safeguards

- Closure behavior and lexical capture remain covered by language regression tests.
- New runtime object types must document ownership edges and lifetime expectations.
- Native extensions must not retain interpreter-owned values beyond the interpreter lifetime unless a future explicit lifetime contract permits it.
- A future collector proposal must define roots, traversal of arrays/maps/classes/instances/native objects, finalization ordering, extension interaction, and measurable pause/space behavior before implementation.

## Consequences

Lucy 2.0.7 preserves closure correctness and avoids claiming memory reclamation guarantees it does not provide. Long-lived allocation and reference-cycle behavior remain explicit engineering work rather than hidden assumptions.
