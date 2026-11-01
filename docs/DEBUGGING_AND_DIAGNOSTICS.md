# Lucy 2.0.7 — Debugging and Diagnostics Handbook

## 1. Purpose

This document defines a repeatable debugging process for the interpreter, standard library, package tooling, and editor integration. It does not claim that Lucy ships a source-level debugger or an LSP server.

## 2. Bug report minimum

Every actionable report should include:

1. Lucy version (`lucy --version`).
2. Operating system, architecture, compiler, and build type.
3. Exact command and working directory.
4. Smallest source program that reproduces the issue.
5. Actual output and expected output.
6. Complete stderr/stack trace, without secrets.
7. Whether the issue reproduces in a clean build.
8. Relevant feature flags (for example, SQLite build support).

Do not report only “does not work”; preserve the first failing output.

## 3. Triage classification

| Class | Examples | First subsystem |
|---|---|---|
| Lexer | invalid token, string escape | `src/lexer.cpp` |
| Parser | precedence, malformed block | `src/parser.cpp` |
| Runtime | wrong value, scope, dispatch | `src/runtime.cpp`, `src/value.cpp` |
| Standard library | wrong file/network/data behavior | `stdlib/`, `src/stdlib_native.cpp` |
| REPL | history, multiline, completion | `src/repl.cpp` |
| Extension | load failure, ABI mismatch | `src/extension.cpp`, public headers |
| Package tooling | manifest/lock/import behavior | `tools/lucy_pkg.lucy`, module lookup |
| Build/release | compiler/linker/installer | `CMakeLists.txt`, CI and packaging |

## 4. Reproduction discipline

- Reduce the failure while preserving the behavior.
- Remove unrelated imports, files, and environment variables.
- Separate compile-time, runtime, and tooling failures.
- Re-run the smallest reproduction after each change.
- Add a regression test before or alongside the fix.
- Run the full suite after the focused test passes.
- Confirm the test fails against the pre-fix revision when practical.

## 5. Diagnostic quality contract

A user-facing diagnostic should identify the category, source location, offending token or construct when available, and a concise explanation. Suggestions must be conservative; a suggestion is not a silent correction.

For source positions, distinguish byte offsets, Unicode code points, and editor protocol character units. Never mix these units without an explicit conversion.

## 6. Runtime safety and observability

- Keep recursion/call-depth limits and malformed-input guards covered by tests.
- Bound expensive rendering, equality, parsing, and nested data operations.
- Avoid logging secrets, environment values, tokens, or full request headers.
- Ensure native exceptions are translated to Lucy errors at API boundaries.
- Preserve separate stdout/stderr behavior for child processes.
- Ensure LSP protocol output (if implemented later) is isolated from diagnostics/logging.

## 7. Debug build workflow

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
ctest --test-dir build-debug --output-on-failure
```

For a focused test:

```sh
ctest --test-dir build-debug -R test_name --output-on-failure
```

On Windows, use a fresh build directory for each generator/toolchain. Do not reuse a MinGW cache for MSVC or vice versa.

## 8. Regression test expectations

Every bug fix should state:

- the previously failing input;
- expected behavior;
- actual pre-fix behavior;
- test file and assertion;
- platforms/build configurations exercised;
- any known untested boundary.

A passing Linux run does not establish Windows or macOS correctness.

## 9. Open debugging gaps

This source archive does not provide a source-level step debugger, DAP adapter, production LSP server, or remote package diagnostics. Those should be tracked as separate deliverables rather than described as existing features.
