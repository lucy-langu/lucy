# `math` standard library

Convenience mathematical helpers.

## Import

```lucy
import math
```

## Public functions

- `square(x)`
- `cube(x)`
- `clamp(x, low, high)`
- `factorial(n)`
- `gcd(a, b)`
- `lcm(a, b)`
- `average(values)`
- `lerp(a, b, t)`
- `sign(x)`

## Example

```lucy
import math
println math.square 12
println math.gcd 12, 18
```

## Source of truth

The public function list above is extracted from the current `stdlib/math.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
