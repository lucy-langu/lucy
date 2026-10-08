# Lucy 2.0.0 Documentation

Lucy is a general-purpose programming language designed around a small, readable syntax, a dynamic runtime with optional type contracts, a practical standard library, object-oriented programming, and a native C++ extension boundary.

This documentation is deliberately organized as a small set of deep references instead of many fragmented pages. If you are new to Lucy, read in order; if you already know Lucy, use the reference documents as the canonical index.

## Documentation map

| Document | Purpose |
|---|---|
| [Getting Started](GETTING_STARTED.md) | Install, run, learn the language through complete programs, and understand the recommended workflow. |
| [Language Reference](LANGUAGE_REFERENCE.md) | Complete language syntax and runtime semantics: values, types, expressions, functions, control flow, OOP, modules, errors, and limits. |
| [Standard Library](STANDARD_LIBRARY.md) | The user-facing API for every shipped module, including signatures, behavior, examples, errors, and a complete API inventory. |
| [REPL and Tooling](REPL_AND_TOOLING.md) | REPL, completion, CLI, history, packages, tests, editor integration, and troubleshooting on Windows/Linux/macOS. |
| [Developing Lucy](DEVELOPING_LUCY.md) | Architecture, source tree, native extensions, build system, testing, release/versioning, and how to add language/library features safely. |
| [Changelog](CHANGELOG.md) | Version history and user-visible changes. |

## The shortest path to your first useful program

```lucy
func greet(name: String) -> String {
    return "Hello, $name!"
}

let names = ["Ada", "Grace", "Nima"]

for name, index in names {
    println "#$index: " + greet(name)
}
```

Then try the REPL:

```text
$ lucy
Lucy$ let values = [1, 2, 3]
Lucy$ values.ma<Tab>
```

Completion is live: it uses the current runtime environment and object members rather than a separate documentation-only list.

## Canonicality rule

The implementation is the final authority for Lucy 2.0.0 behavior. These documents are written from the lexer, parser, runtime, shipped `stdlib/*.lucy`, tests, and public headers. A feature is not documented as supported merely because it would be desirable.

When a feature is not implemented, the reference says so explicitly. This is intentional: Lucy documentation must never teach syntax that the interpreter cannot execute.

## Design principles

- **Readable source:** braces delimit blocks; `end` is not part of normal 2.0 syntax.
- **Flat APIs:** standard-library modules expose practical one-level calls such as `fs.read(...)`, not unnecessary namespace chains.
- **Objects everywhere:** classes can extend built-in types such as `String` and `Array`.
- **Runtime contracts:** type annotations are checked by the interpreter at runtime.
- **Useful REPL:** completion, history, multiline input, and discoverable help are first-class development tools.
- **Portable core:** the interpreter and REPL are designed for Windows, Linux, and macOS.
- **Extensible runtime:** pure Lucy libraries are preferred when possible; C++ extensions are available when native functionality is necessary.
