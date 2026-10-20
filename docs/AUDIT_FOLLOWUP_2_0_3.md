# Lucy 2.0.3 — audit follow-up

This document tracks the supplied “Lucy 2.0.3 - Prioritized Bug Report and Roadmap” against a targeted source hardening pass. Statuses describe this source tree and the Linux tests actually run; they are not a claim that every audit item is closed.

## Addressed in this pass

- **P0-1 (partial):** user-function call depth is bounded (default 1000; `LUCY_MAX_CALL_DEPTH` override) and reports `RecursionError`. Parser expression and unary nesting also have limits. Full parser statement/block nesting and all evaluator recursion paths still need a second pass.
- **P0-2:** cyclic array/map printing uses cycle markers; equality is cycle-safe, structural for arrays/maps, and identity-based for instances/functions/classes.
- **P0-3 (partial):** JSON nesting is limited to 512 levels and fails with `JSONDecodeError`; parser expression/unary recursion is bounded. Deep statement blocks and CSV nesting/size limits remain to be audited.
- **P0-4 (partial):** integer literals are range-checked; int64 add/subtract/multiply, shifts, floating-to-integer casts, and `random_int(min,max)` bounds are checked. Integer division/power semantics, all builtins/native casts, and edge cases such as minimum-int negation need further review.
- **P0-5 (partial):** the environment registry now stores weak references and prunes expired entries, avoiding strong retention of every block/call environment. Shutdown still clears live environments to break closure cycles; a true ownership/GC design is not implemented.
- **P0-6 (partial):** string arguments in native APIs now produce language-level `TypeError` rather than `std::bad_variant_access`. A complete audit of every `std::get` and all native argument contracts is still required.
- **P0-7:** floating-point numeric literals that overflow `std::stod` now produce `SyntaxError: number literal out of range`.
- **P0-8 (partial):** current-working-directory and importing-script-directory native extension search have been removed. Explicit installation roots and `LUCY_PATH` are retained. The trust model for `LUCY_PATH`, package loading and dynamic libraries still needs documentation and review.
- **P0-9:** directory removal is non-recursive by default; recursive deletion requires `true`.

## Regression verification

- Existing Debug/SQLite-enabled CTest suite passed after the changes: **37/37 tests**.
- New regression coverage checks recursive calls, cyclic values, arithmetic overflow, shift counts, reversed random ranges, deeply nested JSON and non-recursive directory removal.
- A sanitizer build was attempted but did not finish within the execution window; this pass therefore makes **no ASan/UBSan-clean claim**.
- Windows and macOS native runs have not been executed here. CI configuration alone is not proof of platform success.

## Remaining audit priorities

1. Finish P0-4 and P0-6 with exhaustive variant/cast audits and message-asserting tests for every failure path.
2. Fix `finally` behavior and exception typing/catch scoping (P1-18/P1-19).
3. Correct string indexing/ranges, map key/member precedence, ordered maps, lazy ranges and iterator semantics (P1-13/P1-15/P1-16/P1-17/P1-24).
4. Correct integer precision, unary/power result types, float modulo, precedence and type widening (P1-1 through P1-6).
5. Harden HTTP/process APIs, binary I/O, module cache/shadowing and deterministic filesystem results (P0-10/P2-3/P2-5/P2-6).
6. Add fuzzing, golden stdout/stderr tests, benchmarks, and native Windows/macOS evidence (P2-8/P3).
