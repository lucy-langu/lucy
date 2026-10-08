# `set` standard library

Set construction and set operations.

## Import

```lucy
import set
```

## Public functions

- `new(values = [])`
- `from_values(values)`
- `add(target, value)`
- `delete(target, value)`
- `include?(target, value)`
- `union(left, right)`
- `intersection(left, right)`
- `difference(left, right)`
- `symmetric_difference(left, right)`
- `subset?(left, right)`
- `superset?(left, right)`
- `intersect?(left, right)`

## Example

```lucy
import set
let values = set.new [1, 2, 3]
println set.include? values, 2
```

## Source of truth

The public function list above is extracted from the current `stdlib/set.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
