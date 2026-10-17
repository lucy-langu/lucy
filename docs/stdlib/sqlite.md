# `sqlite` standard library — Lucy 2.0.1

SQLite database and prepared-statement wrappers; requires a build with SQLite enabled.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import sqlite
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
let db = sqlite.open("example.db")
db.execute("CREATE TABLE IF NOT EXISTS items (id INTEGER PRIMARY KEY, name TEXT)")
db.execute("INSERT INTO items (name) VALUES (?)", ["Lucy"])
println db.query("SELECT id, name FROM items")
db.close()
```

## Module-level functions

| Function | Signature |
|---|---|
| `open` | `open(path)` |

## Classes and methods

### `Database`

| Method | Signature |
|---|---|
| `initialize` | `initialize(path)` |
| `execute` | `execute(sql, params = [])` |
| `query` | `query(sql, params = [])` |
| `prepare` | `prepare(sql)` |
| `begin` | `begin()` |
| `commit` | `commit()` |
| `rollback` | `rollback()` |
| `changes` | `changes()` |
| `last_insert_id` | `last_insert_id()` |
| `close` | `close()` |

### `Statement`

| Method | Signature |
|---|---|
| `initialize` | `initialize(handle, sql)` |
| `bind` | `bind(params = [])` |
| `execute` | `execute()` |
| `query` | `query()` |
| `close` | `close()` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

### Build requirement

Configure with `-DLUCY_ENABLE_SQLITE=ON`. A build without this option cannot open databases. Use prepared-statement placeholders for values and close connections/statements explicitly.

## Source of truth

Implementation: [`stdlib/sqlite.lucy`](../../stdlib/sqlite.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
