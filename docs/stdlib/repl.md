# `repl` standard library

Help and presentation helpers for the interactive environment.

## Import

```lucy
import repl
```

## Public functions

- `banner()`
- `version()`
- `prompt(depth = 0)`
- `commands()`
- `topics()`
- `help(topic = nil)`

## Example

```lucy
import repl
println repl.version()
```

## Source of truth

The public function list above is extracted from the current `stdlib/repl.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
