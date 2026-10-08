# `runtime` standard library

Runtime introspection and dynamic invocation helpers.

## Import

```lucy
import runtime
```

## Public functions

- `printf(format_string, values = [])`
- `format(format_string, values = [])`
- `catch(callback)`
- `rescue(callback, handler)`
- `ensure(callback, cleanup)`
- `methods(value)`
- `responds(value, name)`
- `call(value, name, args = [])`
- `inspect(value)`
- `variables()`
- `globals()`
- `ancestors(value)`
- `superclass(value)`

## Example

```lucy
import runtime
println runtime.inspect 42
```

## Source of truth

The public function list above is extracted from the current `stdlib/runtime.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
