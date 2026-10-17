# `repl` standard library — Lucy 2.0.1

Styled REPL banner/prompt, command/topic metadata, and topic help.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import repl
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println repl.version()
println repl.commands()
println repl.topics()
```

## Module-level functions

| Function | Signature |
|---|---|
| `banner` | `banner()` |
| `version` | `version()` |
| `prompt` | `prompt(depth = 0)` |
| `commands` | `commands()` |
| `topics` | `topics()` |
| `help` | `help(topic = nil)` |

## Classes and methods

No public class declarations were found in this module's pure-Lucy source.

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/repl.lucy`](../../stdlib/repl.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
