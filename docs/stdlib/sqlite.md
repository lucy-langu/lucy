# `sqlite` standard library

SQLite entry point. Availability depends on the CMake SQLite option.

## Import

```lucy
import sqlite
```

## Public functions

- `open(path)`

## Example

```lucy
import sqlite
let db = sqlite.open "test.sqlite"
```

## Source of truth

The public function list above is extracted from the current `stdlib/sqlite.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
