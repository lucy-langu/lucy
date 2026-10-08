# `random` standard library

Random value and collection helpers.

## Import

```lucy
import random
```

## Public functions

- `int(low, high)`
- `float()`
- `bool()`
- `choice(items)`
- `shuffle(items)`
- `sample(items, count)`

## Example

```lucy
import random
println random.int 1, 10
```

## Source of truth

The public function list above is extracted from the current `stdlib/random.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
