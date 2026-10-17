# `set` standard library — Lucy 2.0.2

Set construction, membership, and set algebra operations.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import set
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
let left = set.from_values([1, 2, 3])
let right = set.from_values([3, 4])
println set.union(left, right)
```

## Exported module-level objects

`set`

## Module-level functions

| Function | Signature |
|---|---|
| `new` | `new(values = [])` |
| `from_values` | `from_values(values)` |
| `add` | `add(target, value)` |
| `delete` | `delete(target, value)` |
| `include?` | `include?(target, value)` |
| `union` | `union(left, right)` |
| `intersection` | `intersection(left, right)` |
| `difference` | `difference(left, right)` |
| `symmetric_difference` | `symmetric_difference(left, right)` |
| `subset?` | `subset?(left, right)` |
| `superset?` | `superset?(left, right)` |
| `intersect?` | `intersect?(left, right)` |

## Classes and methods

### `Set`

| Method | Signature |
|---|---|
| `initialize` | `initialize(values = [])` |
| `new` | `new(values = [])` |
| `add` | `add(value)` |
| `delete` | `delete(value)` |
| `include?` | `include?(value)` |
| `member?` | `member?(value)` |
| `each` | `each(callback)` |
| `size` | `size()` |
| `length` | `length()` |
| `empty?` | `empty?()` |
| `clear` | `clear()` |
| `map` | `map(callback)` |
| `select` | `select(callback)` |
| `reject` | `reject(callback)` |
| `merge` | `merge(other)` |
| `subset` | `subset(other)` |
| `superset` | `superset(other)` |
| `intersect` | `intersect(other)` |
| `union` | `union(other)` |
| `intersection` | `intersection(other)` |
| `difference` | `difference(other)` |
| `subset?` | `subset?(other)` |
| `superset?` | `superset?(other)` |
| `intersect?` | `intersect?(other)` |
| `symmetric_difference` | `symmetric_difference(other)` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/set.lucy`](../../stdlib/set.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
