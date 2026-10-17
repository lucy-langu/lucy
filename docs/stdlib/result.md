# `result` standard library — Lucy 2.0.2

Small success/error value conventions and unwrap helpers.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import result
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
let outcome = result.ok(42)
if result.success(outcome) {
    println result.unwrap(outcome)
}
```

## Module-level functions

| Function | Signature |
|---|---|
| `ok` | `ok(value)` |
| `err` | `err(message)` |
| `success` | `success(result)` |
| `unwrap` | `unwrap(result, fallback = nil)` |
| `message` | `message(result)` |

## Classes and methods

No public class declarations were found in this module's pure-Lucy source.

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/result.lucy`](../../stdlib/result.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
