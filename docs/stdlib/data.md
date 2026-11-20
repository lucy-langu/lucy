# `data` standard library — Lucy 2.0.7 · module 1.1.0

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
| `compact` | `compact(items)` |
| `unique` | `unique(items)` |
| `flatten_one` | `flatten_one(items)` |
| `chunk` | `chunk(items, chunk_size)` |
| `first` | `first(items, fallback = nil)` |
| `last` | `last(items, fallback = nil)` |
| `take` | `take(items, count)` |
| `drop` | `drop(items, count)` |
| `select` | `select(items, predicate)` |
| `reject` | `reject(items, predicate)` |
| `any` | `any(items, predicate)` |
| `all` | `all(items, predicate)` |
| `find_index` | `find_index(items, expected)` |
| `pluck` | `pluck(items, key, fallback = nil)` |
| `index_by` | `index_by(items, key)` |
| `group_by` | `group_by(items, key)` |
| `count_by` | `count_by(items, key)` |
| `sum_by` | `sum_by(items, key)` |
| `invert` | `invert(source)` |
| `defaults` | `defaults(source, default_values)` |
| `sorted_keys` | `sorted_keys(source)` |
| `values_for` | `values_for(source, keys, fallback = nil)` |

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

## Collection transformation helpers

These helpers are pure Lucy-level operations over arrays and maps.

| Function | Purpose |
|---|---|
| `compact(items)` | Remove `nil` entries. |
| `unique(items)` | Keep the first occurrence of each value. |
| `flatten_one(items)` | Flatten one array level. |
| `chunk(items, chunk_size)` | Split an array into fixed-size groups; final group may be shorter. |
| `first(items, fallback = nil)` | Return the first item or fallback. |
| `last(items, fallback = nil)` | Return the last item or fallback. |
| `take(items, count)` | Copy the first `count` items. |
| `drop(items, count)` | Copy items after the first `count`. |
| `select(items, predicate)` | Keep items for which predicate returns true. |
| `reject(items, predicate)` | Keep items for which predicate returns false. |
| `any(items, predicate)` | True when at least one item matches. |
| `all(items, predicate)` | True when every item matches; true for an empty array. |
| `find_index(items, expected)` | Return the first matching index or `nil`. |
| `pluck(items, key, fallback = nil)` | Extract one field from an array of maps. |
| `index_by(items, key)` | Index maps by a field value converted to string. |
| `group_by(items, key)` | Group maps by a field value converted to string. |
| `count_by(items, key)` | Count maps by a field value converted to string. |
| `sum_by(items, key)` | Sum numeric values under one map key. |
| `invert(source)` | Reverse a map's key/value mapping; values become string keys. |
| `defaults(source, default_values)` | Merge defaults first, then source values. |
| `sorted_keys(source)` | Return sorted map keys. |
| `values_for(source, keys, fallback = nil)` | Read map values in requested key order. |

`select`, `reject`, `any`, and `all` accept Lucy lambdas, for example `data.select(items, lambda item => item > 2)`.
