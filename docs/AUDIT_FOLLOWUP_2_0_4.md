# Lucy 2.0.4 — Round 2 audit follow-up

This pass applies targeted fixes from `Lucy 2.0.3 fix pass - Round 2 Audit.pdf`. It is not a claim that every item in that audit is closed.

## Changes in this pass

- Array `each`, `map`, `filter`, `any`, and `all` iterate over a snapshot so callbacks can mutate the original array without invalidating the active iterator. `for` loops likewise use a snapshot.
- Value rendering stops descending after 512 nested containers and emits `...`, avoiding unbounded recursive rendering for deeply nested acyclic structures. Cyclic containers retain the existing `[...]` / `{...}` markers.
- Structural equality has a 512-level recursion guard and raises `ValueError` when exceeded.
- `int(double)` rejects non-finite and out-of-range values instead of relying on an unsafe floating-to-integer cast. Integer `sum`, `abs`, and non-negative integer exponentiation preserve integer values and report overflow.
- `Math.clamp` rejects `low > high`; `Math.average` rejects an empty array; `random.sample` rejects negative counts and counts larger than the input size.
- Date month shifting clamps the day to the target month length, including leap-year February.
- Version metadata is bumped to 2.0.4.

## Still open from Round 2

- Native stack exhaustion / evaluator nesting and sanitizer verification (N-1). The call counter is still a secondary guard, not a proof of native-stack safety.
- Closure/environment reference cycles (N-2).
- Deep nested-container destruction and JSON stringify depth (N-4). Rendering and equality now have guards, but this does not make destruction iterative.
- Regex catastrophic backtracking/stack exhaustion (N-5), month-end clamping (N-6), remaining numeric overflow cases (other casts and edge cases; N-8), all unchecked `std::get` paths and function-specific argument diagnostics (N-9), range consistency (N-10), and documented strictness for silent clamping (N-11).
- Full sanitizer and cross-platform verification must be run before describing the audit as closed.
