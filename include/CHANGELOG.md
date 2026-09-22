# Changelog

## 1.0.1 — multiline expressions and HTTP completion

- Allowed newlines inside calls, arrays, maps, parenthesized expressions, lambda parameter lists, and function parameter lists.
- Added multiline examples to the language documentation.
- Completed the public HTTP methods for GET, POST, PUT, PATCH, DELETE, HEAD, and arbitrary requests.
- Added HTTP request options for timeout, connection timeout, proxy, user-agent, redirects, and TLS verification.
- Added reusable HTTP client configuration through `headers`, `timeout`, `connect_timeout`, `proxy`, `user_agent`, `follow_redirects`, and `insecure`.
- Updated HTTP response and dependency documentation.

## 1.0.1 
### Native standard library consolidation

- Consolidated overlapping standard-library modules into domain-oriented `app`, `crypto`, `data`, `fs`, `http`, `runtime`, `system`, `text`, and `time` modules.
- Kept the existing operations while giving each domain one canonical home.
- Expanded `examples/` to 24 multi-step examples covering every canonical native library.
- 
### REPL and library identity

- Moved the editable REPL help/presentation layer into `stdlib/repl.lucy`.
  
- Added Lucy-native `flow`, `data`, and `result` modules.
  
- Added exhaustive `docs/API.md` documentation for core, native bridges, and stdlib declarations.
  
- Audited the expanded standard library and restored compatibility aliases for Set predicates.
  
- Added `Set.subset?`, `Set.superset?`, and boolean `Set.intersect?`.
  
- Added Set operators `|`, `&`, `^`, and `-`.
  
- Kept `File` and `FileUtils` APIs distinct while preserving both low-level and utility-level file operations.
  
- Reduced accidental global function collisions between imported modules; free functions remain namespaced unless explicitly imported with `from ... import ...`.
  
- Added `Time` and `Date` arithmetic with `+`, `-`, and `<=>`.
  
- Added `switch/case/default`, `do ... while`, expression lambdas, and semicolon statement separators.
  
- Updated language, reference, standard-library, and release documentation for 1.0.1.
## 1.0.0

- Promoted Lucy to its first stable release.
- Replaced the basic line-based REPL with a cross-platform interactive line editor.
- Added persistent REPL history with Up/Down navigation.
- Added cursor editing with Left/Right, Home/End, Backspace, and Delete.
- Added Tab completion for keywords, builtins, globals, standard-library modules, module members, and object members.
- Added REPL commands for help, history, clear, version, and quit.
- Expanded the standard library with higher-level filesystem, path, string, collection, math, process, JSON, CSV, datetime, random, regex, and system helpers.
- Added additional array and string methods including insert, remove_at, slice, and char_at.

## 0.11.2

- Fixed the Phase 2 SQLite fallback builtins so Lucy compiles correctly when SQLite support is unavailable.
- Added explicit `Value` return types to exception-only native lambdas for compatibility with GCC 9.2.0 and other strict C++17 compilers.

## 0.11.1 — CLI Arguments and Documentation

- Added script command-line arguments through the global `ARGV` array.
- Added the lowercase `argv` alias for convenient access to script arguments.
- Added `sys.argv()` as the standard-library accessor for command-line arguments.
- Script arguments are preserved as strings in their original order.
- Added `os.home()`, `os.temp_dir()`, and `os.command_exists()` to the public OS module.
- Expanded the language reference to document syntax, values, operators, functions, classes, exceptions, modules, scoping, runtime behavior, and the CLI.
- Expanded the standard-library reference to document every public module, class, function, method, return value, and important limitation.
- Added a beginner-to-intermediate learning guide.
- Added a complete version history document covering additions from Lucy 1.0 to the current release.
- Added command-line argument regression coverage.

## 0.11.0 — Standard Library

- Added the Phase 2 standard library layer.
- Added `regex` with native C++ regular-expression primitives.
- Added a real JSON parser and serializer.
- Added `datetime` with Unix-millisecond timestamps, date parts, and formatting.
- Added `process` for command execution and output capture.
- Added `encoding` with Base64, hexadecimal, and URL encoding.
- Added CSV parsing and serialization with quoted-field support.
- Added an HTTP client using the host `curl` implementation.
- Added SQLite support when CMake finds SQLite3.
- Added the Lucy language logo to the distribution assets.
- Added a complete Phase 2 integration test suite.
- Kept the native boundary small: high-level standard-library APIs remain readable Lucy code.

## 0.10.0 — Language Core

- Added lexical block scoping for control-flow and nested blocks.
- Added default function arguments.
- Added named function arguments.
- Added variadic parameters with `*args`.
- Added argument validation for missing, duplicate, unknown, and out-of-order arguments.
- Improved bound `self` handling.
- Improved relocatable standard-library discovery across Windows, Linux, and macOS.
- Improved Windows installation fallback and user PATH setup.
