# `runtime` standard library — Lucy 2.0.1

Kernel/collection facades, formatting, guarded callback helpers, and reflection/dynamic invocation.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import runtime
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println runtime.inspect([1, 2, 3])
println runtime.methods("Lucy")
println runtime.responds("Lucy", "upper")
```

## Exported module-level objects

`collections`, `kernel`

## Module-level functions

| Function | Signature |
|---|---|
| `printf` | `printf(format_string, values = [])` |
| `format` | `format(format_string, values = [])` |
| `catch` | `catch(callback)` |
| `rescue` | `rescue(callback, handler)` |
| `ensure` | `ensure(callback, cleanup)` |
| `methods` | `methods(value)` |
| `responds` | `responds(value, name)` |
| `call` | `call(value, name, args = [])` |
| `inspect` | `inspect(value)` |
| `variables` | `variables()` |
| `globals` | `globals()` |
| `ancestors` | `ancestors(value)` |
| `superclass` | `superclass(value)` |

## Classes and methods

### `Kernel`

| Method | Signature |
|---|---|
| `printf` | `printf(format_string, values = [])` |
| `p` | `p(value)` |
| `pp` | `pp(value)` |
| `format` | `format(format_string, values = [])` |
| `sprintf` | `sprintf(format_string, values = [])` |
| `catch` | `catch(callback)` |
| `rescue` | `rescue(callback, handler)` |
| `ensure` | `ensure(callback, cleanup)` |
| `system` | `system(command)` |
| `spawn` | `spawn(command)` |
| `trap` | `trap(signal_number, callback)` |
| `global_variables` | `global_variables()` |
| `local_variables` | `local_variables()` |
| `methods` | `methods(value)` |
| `respond_to?` | `respond_to?(value, name)` |
| `send` | `send(value, name, args = [])` |
| `inspect` | `inspect(value)` |
| `to_a` | `to_a(value)` |
| `to_h` | `to_h(value)` |
| `to_sym` | `to_sym(value)` |
| `ancestors` | `ancestors(value)` |
| `superclass` | `superclass(value)` |
| `singleton_class` | `singleton_class(value)` |
| `proc` | `proc(callback)` |
| `lambda` | `lambda(callback)` |

### `Collections`

| Method | Signature |
|---|---|
| `first` | `first(items)` |
| `last` | `last(items)` |
| `reverse` | `reverse(items)` |
| `contains` | `contains(items, value)` |
| `count` | `count(items, value)` |
| `index` | `index(items, value)` |
| `compact` | `compact(items)` |
| `unique` | `unique(items)` |
| `flatten` | `flatten(items)` |
| `sum` | `sum(items)` |
| `min` | `min(items)` |
| `max` | `max(items)` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/runtime.lucy`](../../stdlib/runtime.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
