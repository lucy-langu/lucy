# Lucy 2.0.2 — Built-in Functions and Native Value Methods

This reference covers functions compiled into the runtime and the native methods available on primitive values. It complements the source-indexed [`stdlib/` API pages](stdlib/) and [`Standard Library`](STANDARD_LIBRARY.md). Internal names beginning with `__` are deliberately omitted; they are implementation details, not public APIs.

## Global functions

### Output and conversion

| Function | Signature | Contract |
|---|---|---|
| `print` | `print(value, ...)` | Writes values separated by spaces; does not append a newline. |
| `println` | `println(value, ...)` | Writes values separated by spaces and appends a newline. |
| `input` | `input([prompt])` | Optionally writes a prompt, then reads one line from standard input. |
| `str` | `str(value)` | Returns the runtime string representation. |
| `type` | `type(value)` | Returns the runtime type name, such as `string`, `int`, or `array`. |
| `typeof` | `typeof(value)` | Alias of `type`. |
| `len` | `len(value)` | Returns the number of Unicode code points in a valid UTF-8 string, or the number of elements/entries in an array/map. |
| `int` | `int(value[, base])` | Converts an integer, truncates a floating-point value, or parses a numeric string. String bases must be 2–36. |
| `float` | `float(value)` | Converts a number or a fully consumed numeric string; rejects trailing junk and non-finite string values. |
| `bin` | `bin(integer[, width])` | Formats an integer in base 2; optional width zero-pads the digits. |
| `hex` | `hex(integer[, width])` | Formats an integer in base 16 using uppercase digits; optional width zero-pads the digits. |
| `oct` | `oct(integer[, width])` | Formats an integer in base 8; optional width zero-pads the digits. |

### Collections and numeric functions

| Function | Signature | Contract |
|---|---|---|
| `range` | `range(stop)` or `range(start, stop[, step])` | Creates an array of integers; `stop` is exclusive and `step` cannot be zero. |
| `sum` | `sum(array)` | Adds numeric array elements. |
| `min` | `min(value, ...)` | Returns the smallest numeric argument; at least one argument is required. |
| `max` | `max(value, ...)` | Returns the largest numeric argument; at least one argument is required. |
| `abs` | `abs(value)` | Absolute value. |
| `sqrt` | `sqrt(value)` | Square root; negative inputs raise `ValueError`. |
| `sin`, `cos`, `tan` | `sin(value)`, etc. | Trigonometric functions; inputs are interpreted in radians. |
| `floor`, `ceil` | `floor(value)`, `ceil(value)` | Rounds toward negative or positive infinity, respectively. |
| `exp` | `exp(value)` | Exponential function. |
| `log`, `log10` | `log(value)`, `log10(value)` | Natural and base-10 logarithms; inputs must be positive. |
| `pow` | `pow(base, exponent)` | Raises `base` to `exponent`. |
| `random_int` | `random_int(minimum, maximum)` | Returns a pseudo-random integer in the inclusive interval. |
| `assert` | `assert(condition[, message])` | Raises `AssertionError` when the condition is false. |

### Filesystem, process, and runtime inspection

| Function | Signature | Contract |
|---|---|---|
| `read_file` | `read_file(path)` | Reads a file as text; throws `IOError` on failure. Prefer `fs.read` for the standard-library filesystem API. |
| `write_file` | `write_file(path, content)` | Truncates and writes a text file; throws `IOError` if it cannot open the destination. Prefer `fs.write` for the standard-library filesystem API. |
| `exists` | `exists(path)` | Returns whether the path exists (file or directory). |
| `cwd` | `cwd()` | Returns the process current working directory. |
| `getenv` | `getenv(name)` | Returns the environment variable value, or `nil` when it is unset. |
| `sleep` | `sleep(milliseconds)` | Blocks the current thread for the requested number of milliseconds. |
| `millis` | `millis()` | Returns elapsed milliseconds since the timer's first call in the current process. |
| `modules_info` | `modules_info()` | Returns a map of discoverable Lucy module names to source paths. It searches configured roots, not the entire disk. |
| `exit` | `exit([code])` | Terminates the current process with the optional exit status. |

### Predefined bindings

| Name | Meaning |
|---|---|
| `PI`, `E` | Common mathematical constants. |
| `VERSION` | Current Lucy runtime version string. |
| `PLATFORM` | Build platform name (`windows`, `macos`, `linux`, or `unix`). |
| `ARGV`, `argv` | The same mutable array of script arguments, excluding the script path itself. |

## Native methods on values

Built-in types can be extended with `class Type { ... }` declarations; extension methods are documented in [the language reference](LANGUAGE_REFERENCE.md#114-built-in-type-extension) and appear in REPL completion. The lists below describe the native methods available before user extensions are added.

### `String`

| Method | Signature | Contract |
|---|---|---|
| `length`, `size` | `value.length()` | Unicode code-point count for valid UTF-8; not grapheme-cluster count. |
| `upper`, `upcase` | `value.upper()` | ASCII case conversion; full Unicode case mapping is not implemented. |
| `lower`, `downcase` | `value.lower()` | ASCII case conversion; full Unicode case mapping is not implemented. |
| `strip`, `trim` | `value.strip()` | Removes leading/trailing ASCII whitespace. |
| `contains` | `value.contains(part)` | Substring test. |
| `starts_with`, `ends_with` | `value.starts_with(part)` | Prefix/suffix tests. |
| `reverse` | `value.reverse()` | Returns a new string with code points reversed. |
| `repeat` | `value.repeat(count)` | Repeats the string; negative counts raise `ValueError`. |
| `to_int`, `to_float` | `value.to_int()` | Strict numeric conversion; trailing junk is rejected. `to_float` also rejects non-finite results. |
| `slice` | `value.slice(start[, end])` | Returns code points from `start` up to but excluding `end`; negative indexes count from the end. |
| `char_at` | `value.char_at(index)` | Returns one code point or `nil` for an out-of-range index. |
| `split` | `value.split(separator)` | Splits on a non-empty separator. |
| `replace` | `value.replace(search, replacement)` | Replaces non-overlapping matches; an empty search string raises `ValueError`. |

### `Array`

| Method | Signature | Contract |
|---|---|---|
| `push` | `array.push(value, ...)` | Appends values and returns the new length. |
| `pop`, `shift` | `array.pop()`, `array.shift()` | Removes and returns the last/first value, or `nil` for an empty array. |
| `unshift` | `array.unshift(value, ...)` | Prepends values and returns the new length. |
| `insert` | `array.insert(index, value)` | Inserts at an index; out-of-range indexes are clamped to the array bounds. |
| `remove_at` | `array.remove_at(index)` | Removes and returns the value, or `nil` for an invalid index. |
| `clear` | `array.clear()` | Removes all elements. |
| `first`, `last` | `array.first()`, `array.last()` | Returns an endpoint or `nil` for an empty array. |
| `contains`, `count`, `index` | `array.contains(value)` | Tests membership, counts equal values, or returns the first index (`nil` if absent). |
| `join` | `array.join(separator)` | Joins string representations using the separator. |
| `reverse` | `array.reverse()` | Reverses the array in place and returns the array. |
| `length`, `size` | `array.length()` | Number of elements. |
| `slice` | `array.slice(start[, end])` | Returns a new array for the half-open index range. |
| `each`, `map`, `filter` | `array.map(callback)` | Calls a callback per element; `map` returns transformed values and `filter` keeps truthy matches. |
| `any`, `all` | `array.any(callback)`, `array.all(callback)` | Tests whether any/all callback results are truthy. |

### `Map`

| Method | Signature | Contract |
|---|---|---|
| `get` | `map.get(key)` | Returns the value or `nil` if absent. |
| `set` | `map.set(key, value)` | Stores and returns the value. Keys must be strings. |
| `has` | `map.has(key)` | Tests whether a string key exists. |
| `delete` | `map.delete(key)` | Removes a key and returns whether it existed. |
| `keys`, `values` | `map.keys()`, `map.values()` | Returns arrays of keys or values. |
| `length`, `size` | `map.length()` | Number of entries. |
| `clear` | `map.clear()` | Removes all entries. |

### `Int`, `Double`, and `Number`

| Method | Signature | Contract |
|---|---|---|
| `abs`, `floor`, `ceil`, `round` | `value.round()` | Numeric absolute value and rounding operations. |
| `sqrt`, `sin`, `cos`, `tan`, `log` | `value.sqrt()` | Numeric operations; square root/log domain errors are reported. Trigonometric inputs use radians. |
| `to_int`, `to_string` | `value.to_int()` | Converts/truncates to integer or returns a string representation. |
| `to_binary`, `to_hex`, `to_octal` | `value.to_hex([width])` | Formats the truncated integer part with optional zero-padding. |
| `pow` | `value.pow(exponent)` | Raises the receiver to the supplied exponent. |

## Examples

```lucy
println len("é🙂")       # 2 code points
println hex(255, 4)      # 00FF
println range(2, 8, 2)   # [2, 4, 6]
println modules_info()   # module name -> source path map

class String {
    func shout() {
        return self.upper() + "!"
    }
}
println "Lucy".shout()  # LUCY!
```

## Contract and known limits

- `len(string)` and `String.length()` count Unicode code points for valid UTF-8, not user-perceived grapheme clusters.
- String upper/lower methods are ASCII-oriented; do not assume full Unicode case conversion.
- `float(string)` rejects trailing junk and non-finite values, while `int(string[, base])` requires a complete numeric string.
- Native built-ins are implemented in C++; their internal `__*` counterparts are not public API. Prefer the flat `fs`, `system`, `math`, `random`, `text`, and other standard-library module APIs for new application code.
