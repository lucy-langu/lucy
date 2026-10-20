## 2.0.5 — Windows/MSVC build fix

- Defined `NOMINMAX` for the `lucy_runtime` target on Windows so Windows SDK `min`/`max` macros cannot corrupt calls to `std::min` and `std::max`. This addresses the MSVC compilation cascade in `runtime.cpp` and `stdlib_native.cpp`.
- Updated the project, runtime, CLI, installer message, standard-library manifest, and core API contract version metadata to `2.0.5`.
- This fix is based on the reported Windows GitHub Actions log. A native Windows/MSVC run must still confirm the workflow is green.

## 2.0.4 — Round 2 audit follow-up

- Snapshot-based array callback and `for` iteration prevents iterator invalidation when callbacks mutate the source array.
- Added bounded recursive rendering/equality and checked numeric behavior for `int(double)`, integer `sum`, `abs`, and exponentiation.
- Clamped date month shifts to the last valid day of the target month.
- Added validation for `Math.clamp`, `Math.average`, and `random.sample`.
- See `docs/AUDIT_FOLLOWUP_2_0_4.md` for scope and unresolved audit items.

# Lucy Changelog

## 2.0.3 — Core object-oriented APIs and diagnostics

- Added import-free `Dir`, `File`, and `Path` global objects, delegating to the existing `fs` and `system` APIs for consistent behavior.
- Added useful `NameError` suggestions for likely misspellings of visible variables and global names.
- Added regression tests for core filesystem facades and typo diagnostics.
- Updated version constants, CLI output, core API reference, error guide, examples, and release verification record.
- Full structured runtime stack traces and precise source spans for every runtime error remain future work; this release does not claim Ruby-level diagnostic parity.

## 2.0.2 — REPL usability, introspection, and documentation synchronization

- Removed ANSI color styling from the REPL banner and prompts to avoid Windows color-rendering problems; retained only terminal control sequences required for editing.
- Changed Tab completion to expand a unique/common prefix first, show candidates on the next Tab, and cycle through candidates on subsequent Tabs.
- Added REPL commands `:vars`, `:functions`, `:modules`, `:where <name>`, and `:methods <Type>` for live runtime inspection.
- Made completion include user-added methods on built-in types such as `String` and `Int`, while hiding internal `__*` builtins from top-level completion.
- Added runnable examples and regression tests for built-in type extensions and module path introspection; corrected REPL and language reference docs to match implementation.
- Prevented module discovery from treating arbitrary executable parent directories as Lucy library roots.
- Fixed `len(string)` to count UTF-8 code points, made `float(string)` reject trailing junk/non-finite values, and implemented the documented `Map.length()`/`Map.size()` methods.


## 2.0.1 — Reliability, documentation, and REPL polish

- Fixed UTF-8 code-point behavior for core string length/index/slice/reverse methods.
- Prevented empty-pattern replacement hangs and made numeric conversions reject trailing junk.
- Corrected CSV record/quoting behavior and JSON surrogate-pair decoding.
- Made `Benchmark` measure Lucy callbacks rather than executing shell commands.
- Hardened `OptionParser`, added `--name=value`, and improved REPL styling, topic help, Ctrl-C reset, UTF-8 editing, Ctrl-L, and Ctrl-W.
- Added regression tests, source-indexed per-module API pages, and runnable CSV/JSON/SQLite projects.

## 2.0.0 Changelog

## 2.0.0 — Runtime, language, library, and tooling consolidation

### Language

- Brace-delimited blocks are the current block syntax.
- The language is case-sensitive.
- Runtime type contracts are enforced for variables, parameters, returns, and typed struct fields.
- Hexadecimal integer literals (`0x` / `0X`) are supported.
- Binary integer literals (`0b` / `0B`) are supported.
- Numeric digit separators are supported in valid positions.
- Quoted map keys are supported.
- Expression lambdas are supported.
- Empty blocks are supported.
- `??` supports nil/missing-value fallback behavior.
- `println` provides newline-terminated output.
- `exit(code)` provides explicit process termination.
- Structs provide typed aggregate values with methods.
- Range expressions support inclusive `..` and exclusive `...` forms.
- Array range indexing supports negative endpoints.
- Indexed `for`/`foreach` bindings expose value and zero-based index.
- `range(start, stop, step)` provides numeric iteration values.
- Numeric conversion helpers provide binary, hexadecimal, and octal formatting, including optional zero-padding.
- Number methods provide `to_binary`, `to_hex`, and `to_octal`.

### Runtime and object model

- Built-in types can be reopened through ordinary `class Type { ... }` declarations.
- Built-in extension methods receive the actual value as `self`.
- Standard modules shipped with Lucy are loaded automatically at interpreter startup.
- The runtime exposes a reusable shared library.
- Native extension loading works through the public C++ extension boundary.
- Native resource-backed objects participate in member dispatch.

### Standard library

- The standard library is organized into flat modules: `app`, `crypto`, `data`, `flow`, `fs`, `http`, `math`, `random`, `repl`, `result`, `runtime`, `set`, `sqlite`, `system`, `text`, and `time`.
- Internal `__*`/private helpers remain implementation details rather than public application API.
- SQLite remains optional at build time.

### REPL and tooling

- Interactive history and line editing are supported.
- Member completion supports dotted paths and cycles through multiple candidates with repeated Tab presses.
- Completion inventories include current built-ins and standard modules.

### Packaging and extensions

- Pure-Lucy packages use `packages/<name>/src/init.lucy` as their entry point.
- `LUCY_PATH` is the main configurable resource root.
- The public C++ extension API is exposed through `include/lucy/extension.hpp`.

### Removed from the current public surface

- `echo` is not a 2.0 output builtin.
- The old `end` block terminator is not the current block syntax.
- The earlier hardware simulator concept is not part of the 2.0 core.
- Old deep standard-library namespace examples are not current API.

Historical details and migration guidance are in `HISTORY_AND_MIGRATION.md`.

## 2.0.3 — core class APIs and stdlib release metadata

- Made `File`, `Dir`, and `Path` actual core class objects whose class methods delegate to the canonical `fs` and `system` module functions.
- Enabled class-body extensions such as `class File { func helper() { ... } }` to modify the same global core class instead of creating an unrelated facade.
- Preserved explicit imports of `fs`, `system`, `text`, `time`, and all other standard-library modules.
- Added `stdlib/manifest.json` with independently versioned module metadata and `tools/stdlib_manager.py` to list, verify, and update module versions.
- Expanded documentation for the core class API, module versioning policy, and honest limits of current runtime diagnostics.
- Added a regression test for core class extension, filesystem API behavior, and lowercase module imports.
- Fixed the CMake environment for the name-error suggestion test so it can locate the standard library in an out-of-tree build.

### Known limitations

- Per-module versions are metadata and release tooling; there is not yet a remote registry, automatic downloader, dependency solver, or transactional updater.
- Runtime exceptions do not yet all retain exact AST source spans, so full line/column/caret diagnostics and complete call stacks remain unfinished.
- Native Windows and macOS verification has not been performed as part of this change.

## 2.0.3 — object model, source-aware runtime errors, and ecosystem hardening

- Added `super.method(...)` parent dispatch for instance methods, preserving the current receiver and starting lookup at the defining class's parent.
- Added `static`, `public`, `protected`, and `private` method modifiers; runtime access checks distinguish `AccessError` from missing-method failures.
- Added source locations to parsed statement/expression roots and runtime diagnostics, including a source excerpt/caret and function frames with declaration locations where available.
- Added an object-model regression suite and a diagnostic regression that checks runtime line/column output.
- Added atomic `stdlib_manager.py install-source` updates with SemVer validation, downgrade protection, rollback on manifest verification failure, and dependency constraint checks.
- Added a GitHub Actions matrix for Debug/Release builds and tests on Linux, Windows, and macOS. The matrix is CI configuration; native platform results are only confirmed after those jobs actually run.
- Added `docs/OBJECT_MODEL_AND_DIAGNOSTICS.md` and a machine-readable core API contract at `docs/contracts/core_api.json`.


## 2.0.3 — audit-driven runtime hardening

- Added a configurable interpreter call-depth limit (`LUCY_MAX_CALL_DEPTH`) and a `RecursionError` instead of allowing uncontrolled recursive calls to exhaust the native stack.
- Made array/map stringification and equality cycle-safe; arrays/maps compare structurally, while instances and callable/class values use identity semantics.
- Added a 512-level nesting limit to JSON parsing, expression parsing, and unary operators.
- Added checked int64 addition/subtraction/multiplication, checked floating-point-to-integer conversion, safe shift-count validation, and integer-literal range diagnostics.
- Replaced unsafe string argument extraction across native API arguments with language-level `TypeError` diagnostics.
- Changed `Dir.rmdir` to be non-recursive by default; recursive deletion now requires an explicit boolean opt-in.
- Removed implicit current-working-directory and source-directory native extension lookup; configured installation roots and `LUCY_PATH` remain supported.
- Replaced the strong environment registry with weak references and periodic pruning, preserving shutdown cycle breaking without retaining every temporary environment for the interpreter lifetime.
- Added audit regression coverage for cycles, recursion limits, integer overflow, shifts, random ranges, JSON nesting and safe directory removal.

### Audit items still open

This is a targeted hardening pass, not closure of the entire attached audit. Remaining high-priority work includes complete removal of all unchecked variant access (especially non-string map-key and non-callable initialization paths), closure-cycle ownership without shutdown-time cleanup, full checked numeric semantics including all casts and unary/power edge cases, iterator/range safety, `finally` correctness, typed exception objects and catch semantics, complete token spans, binary-safe file APIs, HTTP hardening/native TLS, module caching, and Windows/macOS execution evidence. See `docs/AUDIT_FOLLOWUP_2_0_3.md` for status.
