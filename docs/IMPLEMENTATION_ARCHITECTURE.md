# Lucy 2.0.0 Implementation Architecture

This document describes the current C++ implementation rather than the ideal future architecture.

## 1. Pipeline

The implementation can be understood as:

```text
source
  ↓
lexer
  ↓
tokens
  ↓
parser
  ↓
AST
  ↓
interpreter/runtime
  ├── environments and values
  ├── built-ins and object dispatch
  ├── standard-library bridge
  ├── module/package resolver
  ├── REPL
  └── native extension boundary
```

## 2. Lexer

`src/lexer.cpp` turns source text into token objects. It recognizes keywords, identifiers, strings, numbers, punctuation, operators, ranges, interpolation-related forms, backticks, comments, and statement separators.

The lexer performs important numeric validation. Hexadecimal and binary literals are not allowed to degrade into a prefix plus unrelated identifier when malformed.

## 3. Parser

`src/parser.cpp` constructs AST nodes for declarations, expressions, control flow, functions, classes, structs, imports, exceptions, loops, ranges, lambdas, and assignments.

The expression parser separates precedence levels including range, arithmetic, shifts, comparison, equality, bitwise operations, logical operations, coalescing, and ternary expressions.

## 4. AST

The public AST declarations live under `include/lucy/ast.hpp`. The tree contains nodes for statements and expressions, including `RangeExpr` and the function/class/struct forms required by 2.0.

## 5. Runtime values

`include/lucy/value.hpp` defines the runtime `Value` representation and the value categories used by the interpreter. `src/value.cpp` contains value operations and representation logic.

Runtime method dispatch supports:

- ordinary class instances;
- built-in values such as numbers, strings, arrays, and maps;
- built-in type classes that can be reopened by Lucy source;
- native resource-backed objects.

## 6. Built-in type extension

The runtime maintains built-in type classes and installs them for `Object`, `Nil`, `Bool`, `Int`, `Double`, `Number`, `String`, `Array`, `Map`, `Function`, `Class`, `Instance`, and `Native`.

When Lucy code declares:

```lucy
class String {
    func shout() -> String {
        return self.upper() + "!"
    }
}
```

the runtime extends the existing built-in class instead of replacing the primitive type. The method receiver is bound to the actual primitive value.

This is the public object model; application code does not need an internal type-extension function.

## 7. Standard-library loading

The interpreter loads the shipped standard modules at startup. This is why a 2.0 program can call `fs.read`, `http.get`, or `data.parse` without importing those shipped modules first.

User modules and packages still participate in normal import resolution.

## 8. Range and slicing implementation

`RangeExpr` represents `a..b` and `a...b`. Array indexing detects range expressions and constructs a new array. Negative endpoints are normalized against the collection length.

The built-in `range(start, stop, step)` creates an integer array with an exclusive stop and rejects a zero step.

## 9. Loop implementation

`for` and `foreach` evaluate an array/range value and bind each element. The optional second loop variable receives the zero-based item index:

```lucy
for value, index in values {
    # value and index are available here
}
```

## 10. REPL

`src/repl.cpp` implements the interactive editor and completion behavior. Completion candidates are derived from known keywords, standard modules, module members, and built-in value members. Dotted completion can cycle through multiple matches.

The completion implementation avoids executing arbitrary user expressions merely to produce a candidate list.

## 11. Native extension boundary

`include/lucy/extension.hpp` and `src/extension.cpp` implement the public native extension boundary. The runtime can load shared modules and expose functions, constants, and native objects.

The runtime library is also built as a reusable shared library (`lucy_runtime`), separating the runtime implementation from the CLI executable.

## 12. Native standard-library bridge

`src/stdlib_native.cpp` contains native implementations needed for operating-system and external-library operations. The Pure-Lucy modules in `stdlib/` present the user-facing API.

This division keeps the public API expressive while keeping platform-specific mechanisms behind the runtime boundary.

## 13. Optional SQLite

CMake controls SQLite through `LUCY_ENABLE_SQLITE`. When enabled, SQLite3 is found and linked to `lucy_runtime`. When disabled, the runtime remains buildable without that dependency.

## 14. Source map

| Path | Role |
|---|---|
| `src/lexer.cpp` | lexical analysis |
| `src/parser.cpp` | AST construction |
| `src/runtime.cpp` | interpreter, values, built-ins, dispatch |
| `src/value.cpp` | runtime value behavior |
| `src/phase2.cpp` | native/library support layer |
| `src/stdlib_native.cpp` | native standard-library facilities |
| `src/repl.cpp` | interactive editor and completion |
| `src/extension.cpp` | C++ extension loading/API |
| `src/main.cpp` | CLI entry point |
| `include/lucy/*.hpp` | public/runtime headers |
| `stdlib/*.lucy` | Pure-Lucy standard library |
| `tests/*.lucy` | language/runtime regression tests |

## 15. Architectural rule

When adding a feature, decide first whether it belongs in the language core, runtime, Pure-Lucy library, or native extension layer. Avoid adding a native wrapper for functionality that can be expressed cleanly in Lucy.
