# Lucy 2.0.7 — Phase 1 and Phase 2 implementation report

**Implementation pass:** 2026-11-07  
**Base archive:** user-supplied Lucy 2.0.7 source tree  
**Language/runtime version:** unchanged at 2.0.7

## Scope delivered

### Phase 1 — Language specification

- Added `LANGUAGE_SPECIFICATION.md` as the normative contract for 2.0.7.
- Added an explanatory EBNF grammar, operator precedence, runtime type contracts, lexical rules, function/scope rules, object model boundaries, exception behavior, extension boundary, and compatibility policy.
- Updated the detailed language reference's current-version claims and corrected stale statements that contradicted the implementation (expression interpolation, `super.method`, and source-aware textual call frames).
- Linked the specification and architecture decisions from the documentation index.

### Phase 2 — Runtime engineering foundation

- Extracted lexical environment operations from `runtime.cpp` into `environment.cpp`.
- Extracted runtime type matching and type-contract enforcement into private implementation files `src/type_system.hpp` and `src/type_system.cpp`.
- Kept the public `Interpreter` interface and language-visible semantics unchanged.
- Replaced aggregate initialization of runtime `Function` objects in the touched dispatch/struct-construction paths with explicit field initialization.
- Enabled `-Wall -Wextra -Wpedantic` (or `/W4` on MSVC) consistently for runtime and test targets. The final Linux build completed without compiler warnings.
- Added `runtime_core_test.cpp` to test lexical lookup, assignment propagation, immutable bindings, and type contracts independently of parsing.
- Added opt-in `LUCY_ENABLE_SANITIZERS` for GCC/Clang AddressSanitizer and UndefinedBehaviorSanitizer.
- Added ADRs for subsystem boundaries, current memory ownership, and compatibility.

## Verification performed

| Check | Result |
|---|---|
| Configure | Passed |
| Debug build | Passed |
| Compiler warnings in final build | None |
| Existing language/tool tests | Passed |
| New runtime core test | Passed |
| Total CTest tests | **45 / 45 passed** |
| CLI version | `2.0.7` |
| SQLite | Disabled, matching the default project configuration |

**Local environment:** Linux x86_64, GCC 14.2.0, CMake 3.31.6.

## Explicitly not claimed

- No native Windows/MSVC, Windows/MinGW, or macOS build was run in this pass.
- The sanitizer-instrumented build compiled, but the full sanitizer test run could not be validated in this execution environment: the host injects `jemalloc` through `LD_PRELOAD`, and after removing that preload the sandbox could not reserve AddressSanitizer shadow memory. This is an environment limitation, not a passing sanitizer result.
- The memory model was documented, not replaced. Runtime objects still use shared ownership; closure/environment reference cycles can remain until interpreter shutdown. Lucy 2.0.7 does not have a tracing garbage collector.
- The parser, evaluator, and native subsystem are not yet fully separated into independently replaceable components. This pass establishes the first runtime boundaries without a risky interpreter rewrite.
- No bytecode VM, JIT, static type checker, async runtime, or language syntax expansion was added.

## Next engineering gates

1. Run the same build and CTest suite on Windows/MSVC, Windows/MinGW, and macOS.
2. Add a proper ownership/GC design with extension lifetime rules before implementing cycle collection.
3. Introduce structured diagnostics and full AST source spans as a separate compatibility-reviewed change.
4. Continue runtime decomposition by interfaces and tests, not by mechanical file splitting.
