# `time` standard library

Time, date, parsing, formatting, and sleeping helpers.

## Import

```lucy
import time
```

## Public functions

- `now()`
- `today()`
- `timestamp()`
- `parse(text)`
- `date(text)`
- `format(value, pattern = "%Y-%m-%d %H:%M:%S")`
- `_time_sleep(milliseconds)`
- `sleep(milliseconds)`

## Example

```lucy
import time
println time.now()
println time.format time.now()
```

## Source of truth

The public function list above is extracted from the current `stdlib/time.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
