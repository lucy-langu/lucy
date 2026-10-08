# Lucy

**Version 2.0.0**

Lucy is a general-purpose programming language implemented in C++17 with runtime type contracts for annotated values and functions. It focuses on readable brace-delimited source, a compact expression system, a practical standard library, and an interactive REPL.

## Example

```lucy
let name = "Lucy"
var count = 0

func greet(value: String) -> String {
    return "Hello $value"
}

for i in [1, 2, 3] {
    count += i
}

unless name == "" {
    print(greet(name))
}

print(count ?? 0)
```

## Highlights

- brace-delimited blocks
- `let` and `var` bindings
- case-sensitive identifiers
- functions with defaults, variadics, named arguments, and type annotations
- arrays and maps with multiline literals
- multiline function calls
- lambdas and closures
- `switch` and `repeat ... while`
- `unless` negative conditionals
- `??` nil coalescing
- classes and structs
- exceptions with `try/catch/finally`
- modules and selective imports
- HTTP, TCP, DNS, filesystem, persistent SQLite connections and prepared statements, crypto, time, text, data, and process APIs
- interactive REPL and editor definitions
- shared `lucy_runtime` for third-party C++ extensions
- dynamic native module loading
- project-local Pure-Lucy packages under `packages/`

## Standard library

Lucy 2.0.0 loads its standard modules automatically. The main modules (`app`, `crypto`, `data`, `flow`, `fs`, `http`, `math`, `random`, `repl`, `result`, `runtime`, `set`, `sqlite`, `system`, `text`, and `time`) are available without a user-written `import`. Explicit `import` remains supported for compatibility and for user modules/packages.

## Extending built-in types

Built-in values can be extended with normal class syntax:

```lucy
class String {
    func shout() -> String {
        return self.upper() + "!"
    }
}

println "hello".shout()
```

The same mechanism applies to `Int`, `Double`, `Bool`, `Array`, `Map`, and the other built-in types.

## Documentation

The documentation is intentionally consolidated into a small set of deep references:

- `docs/README.md` — documentation map and source-of-truth policy
- `docs/GETTING_STARTED.md` — installation, tutorial, and first programs
- `docs/LANGUAGE_REFERENCE.md` — complete language syntax and semantics
- `docs/STANDARD_LIBRARY.md` — complete standard-library reference and API inventory
- `docs/REPL_AND_TOOLING.md` — CLI, REPL, completion, packages, testing, and editor workflow
- `docs/DEVELOPING_LUCY.md` — implementation architecture, extensions, testing, and releases
- `docs/HISTORY_AND_MIGRATION.md` — historical migration context
- `docs/CHANGELOG.md` — release history
