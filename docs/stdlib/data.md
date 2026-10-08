# `data` standard library

Collection transformation and serialization helpers.

## Import

```lucy
import data
```

## Public functions

- `pick(source, keys)`
- `omit(source, keys)`
- `merge(left, right)`
- `values(source, keys)`
- `zip(keys, values)`
- `parse(text)`
- `stringify(value)`
- `pretty(value)`
- `json_read(path)`
- `json_write(path, value)`
- `yaml_load(text)`
- `yaml_dump(value)`
- `csv_parse(text, delimiter = ",")`
- `csv_stringify(rows, delimiter = ",")`

## Example

```lucy
import data
let user = {name: "Lucy", version: 2}
println data.pick user, ["name"]
```

## Source of truth

The public function list above is extracted from the current `stdlib/data.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
