# Lucy 2.0.0 Changelog

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


## 2.0.0 semantic clarification and documentation update

- Resolved `repeat`/`do` as aliases for the post-test loop grammar.
- Documented exact `*args` variadic syntax and restrictions.
- Documented first-match/no-fallthrough `switch` semantics.
- Documented `global`, lexical environments, shadowing, and closure capture.
- Added complete keyword, operator, type, truthiness, exception, CLI, REPL, extension, testing, limits, migration, and tooling references.
- Added exponent numeric literals (`1e10`, `1.5e-3`).
- Added `\0`, `\xNN`, and `\uXXXX` string escapes.
- Added explicit backslash-newline line continuation.
- Added lexical and semantic regression tests.


## Documentation and REPL consolidation

- Consolidated the fragmented 2.0 documentation into deep canonical references for getting started, language semantics, the complete standard library, REPL/tooling, and development/extension work.
- Removed duplicated topic files whose contents had become stale or too small to be useful as independent references.
- Reworked REPL completion to use the live runtime environment and actual module/object members instead of a separately maintained standard-library member list.
- Improved Tab completion so unique candidates complete immediately, multiple candidates complete their common prefix, and repeated Tab presses cycle through candidates.
- Expanded REPL help topics to cover the language and every shipped standard-library module.
- Added regression coverage for module, builtin-type, chained-member, and numeric-method completion.
