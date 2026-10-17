# `data` standard library — Lucy 2.0.1

Map transformations plus JSON, YAML, and CSV serialization/parsing.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import data
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
let value = data.json.parse("{\"name\":\"Lucy\"}")
println data.json.pretty(value)
let rows = data.csv_parse("name,age\nLucy,2")
println data.csv_stringify(rows)
```

## Exported module-level objects

`csv`, `json`, `yaml`

## Module-level functions

| Function | Signature |
|---|---|
| `pick` | `pick(source, keys)` |
| `omit` | `omit(source, keys)` |
| `merge` | `merge(left, right)` |
| `values` | `values(source, keys)` |
| `zip` | `zip(keys, values)` |
| `parse` | `parse(text)` |
| `stringify` | `stringify(value)` |
| `pretty` | `pretty(value)` |
| `json_read` | `json_read(path)` |
| `json_write` | `json_write(path, value)` |
| `yaml_load` | `yaml_load(text)` |
| `yaml_dump` | `yaml_dump(value)` |
| `csv_parse` | `csv_parse(text, delimiter = ",")` |
| `csv_stringify` | `csv_stringify(rows, delimiter = ",")` |

## Classes and methods

### `JSON`

| Method | Signature |
|---|---|
| `parse` | `parse(text)` |
| `stringify` | `stringify(value)` |
| `pretty` | `pretty(value)` |
| `load` | `load(path)` |
| `dump` | `dump(path, value)` |

### `YAML`

| Method | Signature |
|---|---|
| `load` | `load(text)` |
| `dump` | `dump(value)` |
| `load_file` | `load_file(path)` |

### `CSV`

| Method | Signature |
|---|---|
| `parse` | `parse(text, delimiter = ",")` |
| `stringify` | `stringify(rows, delimiter = ",")` |
| `load` | `load(path, delimiter = ",")` |
| `dump` | `dump(path, rows, delimiter = ",")` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/data.lucy`](../../stdlib/data.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
