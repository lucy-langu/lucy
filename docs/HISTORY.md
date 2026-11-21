# Lucy Development History

## 1.0.1 — Language and Standard Library Consolidation

### Added

- Expression lambdas with `lambda(x) => expression` syntax and closure capture.
- `switch`, `case`, and `default` control flow with `break` support.
- `do ... while` loops that always execute their body at least once.
- Semicolon statement separators for multiple statements on one line.
- Time and Date operator support for `+`, `-`, and `<=>`.
- Set operator support for `|`, `&`, `^`, and `-`.
- Set predicate aliases `subset?`, `superset?`, and `intersect?`.

### Changed

- Ordinary imports expose a flat module API; use `module.function()` or `from module import function`. Implementation classes are available only through explicit selective imports.
- Standard-library documentation now distinguishes canonical APIs from compatibility aliases instead of treating every same-named method as a separate global function.

This document records additions and improvements that are part of the preserved Lucy release history. It focuses on capabilities added to the language and runtime rather than internal experiments.

## 1.0.0 — CLI and Documentation

### Added

- Script command-line arguments through the global `ARGV` array.
- Lowercase `argv` as an alias for script arguments.
- `sys.argv()` as a standard-library accessor for command-line arguments.
- Public `os.home()`.
- Public `os.temp_dir()`.
- Public `os.command_exists()`.
- Complete language and standard-library reference documentation.
- A dedicated command-line reference.
- A beginner-to-intermediate learning tutorial.
- A version history document.
- Regression coverage for command-line arguments.

## 0.11.0 — Standard Library

### Added

- The Phase 2 standard-library layer.
- Regular expressions through the `text` module.
- A JSON parser and serializer through `data`.
- Unix-millisecond date/time support through `time`.
- Process execution and output capture through `system`.
- Base64, hexadecimal, and URL encoding through `text`.
- CSV parsing and serialization through `data`.
- HTTP GET and POST support through `http`.
- Optional SQLite support through `sqlite` when SQLite3 is found during the build.
- The Lucy language logo in the distribution assets.
- Phase 2 integration tests.
- A small native boundary so high-level standard-library APIs remain readable Lucy source.

## 0.10.0 — Language Core

### Added

- Lexical block scoping for control-flow and nested blocks.
- Default function arguments.
- Named function arguments.
- Variadic parameters with `*args`.
- Argument validation for missing, duplicate, unknown, and out-of-order arguments.
- Improved bound `self` handling.
- Relocatable standard-library discovery across Windows, Linux, and macOS.
- Improved Windows installation fallback and user PATH setup.

## 0.9.1 — Filesystem and Standard Library

### Added

- Single-quoted strings with escaping and interpolation.
- A substantially expanded `fs` standard-library module.
- A substantially expanded `fs` standard-library module.
- The `fs` standard-library module.
- Expanded `system` APIs for environment, process, platform, CPU, home-directory, temporary-directory, and command-detection operations.
- A focused `fs` console module.
- Filesystem regression tests.
- Updated editor syntax definitions for single-quoted strings.
- Improved global installation layout and documentation.
- Constructor regression coverage for `Class.new(...)`.

## 0.9.0 — General-Purpose Core

### Added

- Direct `.lucy` source-file execution.
- An immediate interactive REPL.
- Functions and reusable source modules.
- Loops and conditionals.
- Arrays with persistent mutation methods.
- Maps/hashes with property access.
- Classes, constructors, instance fields, and inheritance.
- Typed `try` / `catch` / `finally` handling and `throw`.
- Module imports.
- Shell expressions using backticks with captured stdout.
- Method-oriented APIs for strings, numbers, arrays, and maps.
- Editor syntax packages and cross-platform installation scripts.

## 0.8.0 — Language Expansion

### Added

- A broader general-purpose language surface built around `.lucy` files and an interactive REPL.
- Arrays, indexing, functions, loops, classes, imports, constants, and global declarations.
- String interpolation.
- Arithmetic, comparison, logical, bitwise, assignment, range, and increment/decrement operators.
- Backtick shell expressions.
- An expanded standard library distributed as Lucy source.
- Source-location-aware diagnostics and typed runtime errors.

## 0.7.0 — General-Purpose Scripting

### Added

- Conventional `.lucy` source files.
- Interactive REPL.
- `if`, `else if`, `else`, and `end`.
- `while`, `break`, and `continue`.
- Functions, parameters, recursion, and `return`.
- Local and global variables.
- Constants with `const`.
- Arrays and indexing.
- String interpolation.
- Arithmetic, comparison, logical, and bitwise operators.
- Compound assignment and increment/decrement.
- Module imports.
- Built-in functions and standard libraries written in Lucy.
- Helpful syntax and runtime errors.

## 0.6.0 — Core Language

### Added

- Variables.
- Numbers, strings, booleans, and `nil`.
- String interpolation.
- Arrays.
- Arithmetic and comparisons.
- `if` / `else`.
- `while`.
- Functions and `return`.
- `break` / `continue`.
- Core built-ins including `print`, `input`, `len`, `str`, `int`, `type`, and `range`.
- Single-line comments.
- Interactive REPL.

## 0.5.0 — Early Language Features

### Added

- Variables.
- Strings and numbers.
- Conditional execution.
- `while` loops.
- User-defined functions.
- Function calls and `return`.
- Arithmetic operators including `+`, `-`, `*`, `/`, and `%`.
- Comparison operators.
- Shell command execution.
- Comments with `#`.
- Command-line options and diagnostics.

## 0.4.0 — Earliest Preserved Release

### Added

- The first preserved public C++ implementation of the language.
- Basic source parsing and syntax diagnostics.
- A small command-line interface.
- Comments and foundational scripting syntax.
- Portable C++ implementation without third-party dependencies.

> The repository's earliest preserved release is 0.4.0. Earlier release notes are not present in the current project history, so no unsupported chronology is claimed here.


## 1.0.1 documentation and identity refinement

- Moved REPL help content from the C++ runtime into `stdlib/repl.lucy`.
- Made REPL banner, version presentation, prompts, command metadata, topics, and help renderer Lucy-editable.
- Added `docs/API.md` as an exhaustive implementation-facing API inventory.
- Added Lucy-native `flow`, `data`, and `result` modules without removing compatibility-oriented APIs.
- Kept version 1.0.1 unchanged.
