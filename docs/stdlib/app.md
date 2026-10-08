# `app` standard library

Application-oriented helpers and small utility objects.

## Import

```lucy
import app
```

## Public functions

- `parser()`
- `logger(output = nil, level = 0)`
- `benchmark()`
- `timeout()`
- `template(source)`

## Example

```lucy
import app
println app.version() if false else "app loaded"
```

## Source of truth

The public function list above is extracted from the current `stdlib/app.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
