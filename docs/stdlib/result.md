# `result` standard library

Small result/value helpers.

## Import

```lucy
import result
```

## Public functions

- `ok(value)`
- `err(message)`
- `success(result)`
- `unwrap(result, fallback = nil)`
- `message(result)`

## Example

```lucy
import result
let value = result.ok 42
println result.unwrap value
```

## Source of truth

The public function list above is extracted from the current `stdlib/result.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
