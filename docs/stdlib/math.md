# `math` standard library — Lucy 2.0.1

Mathematical constants/functions and common numeric helpers.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import math
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println math.square(9)
println math.clamp(12, 0, 10)
println math.gcd(24, 18)
```

## Exported module-level objects

`math`

## Module-level functions

| Function | Signature |
|---|---|
| `square` | `square(x)` |
| `cube` | `cube(x)` |
| `clamp` | `clamp(x, low, high)` |
| `factorial` | `factorial(n)` |
| `gcd` | `gcd(a, b)` |
| `lcm` | `lcm(a, b)` |
| `average` | `average(values)` |
| `lerp` | `lerp(a, b, t)` |
| `sign` | `sign(x)` |

## Classes and methods

### `Math`

| Method | Signature |
|---|---|
| `square` | `square(x)` |
| `cube` | `cube(x)` |
| `clamp` | `clamp(x, low, high)` |
| `even` | `even(x)` |
| `odd` | `odd(x)` |
| `factorial` | `factorial(n)` |
| `gcd` | `gcd(a, b)` |
| `lcm` | `lcm(a, b)` |
| `average` | `average(values)` |
| `lerp` | `lerp(a, b, t)` |
| `sign` | `sign(x)` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/math.lucy`](../../stdlib/math.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
