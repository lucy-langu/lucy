# `flow` standard library

Small functional composition helpers.

## Import

```lucy
import flow
```

## Public functions

- `pipe(value, steps)`
- `tap(value, action)`
- `branch(value, predicate, yes, no = nil)`
- `repeat(value, count, step)`

## Example

```lucy
import flow
let value = flow.pipe 2, [lambda x => x * 2, lambda x => x + 1]
println value
```

## Source of truth

The public function list above is extracted from the current `stdlib/flow.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
