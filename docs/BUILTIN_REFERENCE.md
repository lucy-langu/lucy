# Lucy 2.0.7 — Built-in Functions and Native Value Methods

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

## Import-free core filesystem objects (2.0.3)

These global objects are installed by the runtime and require no `import`. They are namespace-like facades over the canonical `fs` and `system` modules. Existing lowercase APIs remain available.

### `Dir`

| Method | Signature | Contract |
|---|---|---|
| `cwd`, `pwd` | `Dir.cwd()`, `Dir.pwd()` | Return the current working directory. |
| `entries`, `files`, `dirs` | `Dir.entries([path])` | Return directory entries, files, or subdirectories. Defaults follow the underlying `fs` API. |
| `exist` | `Dir.exist(path)` | Return whether a filesystem entry exists. |
| `mkdir`, `rmdir` | `Dir.mkdir(path[, parents])`, `Dir.rmdir(path[, recursive])` | Create or remove a directory. `rmdir` is non-recursive by default; pass `true` explicitly to remove a directory tree. Filesystem errors are propagated. |
| `empty`, `glob`, `walk` | `Dir.empty([path])`, `Dir.glob(pattern)`, `Dir.walk([path])` | Directory utility operations forwarded to `fs`. |

### `File`

| Method | Signature | Contract |
|---|---|---|
| `read`, `write`, `append` | `File.read(path)`, `File.write(path, content)`, `File.append(path, content)` | Read text, overwrite a file, or append text. |
| `read_lines`, `write_lines` | `File.read_lines(path)`, `File.write_lines(path, lines)` | Read/write line-oriented text. |
| `exists`, `exist` | `File.exists(path)` | Return whether a file or directory exists (same contract as `fs.exists`). |
| `size` | `File.size(path)` | Return the file size using the underlying filesystem API. |
| `delete`, `remove`, `copy`, `move`, `touch` | `File.delete(path)`, `File.copy(source, destination)` | File operations delegated to `fs`. `delete`/`remove` take one path. |
| `open` | `File.open(path[, mode])` | Open a stream using the existing `fs.IO` abstraction. |

### `Path`

| Method | Signature | Contract |
|---|---|---|
| `join` | `Path.join(parts)` | Join an array of path components. |
| `absolute`, `expand` | `Path.absolute(path)`, `Path.expand(path)` | Convert a path to an absolute or expanded path using the current platform implementation. |
| `basename`, `dirname`, `extension`, `stem` | `Path.basename(path)` etc. | Extract common path components. |

## Native methods on values

Every value participates in a shared Object protocol. These methods are available without imports and can be overridden by user-defined methods on the corresponding built-in type:

| Method | Contract |
|---|---|
| `to_s()`, `to_string()` | Return the runtime's user-facing string representation. Both names are aliases. |
| `inspect()` | Return a diagnostic representation; strings are quoted and common control characters are escaped. |
| `type()`, `type_name()` | Return the runtime type identifier (`nil`, `bool`, `int`, `double`, `string`, `array`, `map`, `function`, `class`, `instance`, or `native`). |
| `class_name()` | Return the language-level class name, including the declared class name for user instances. |
| `nil?()` | Return whether the value is `nil`. |
| `is_a?(Class)` | Test a built-in or user-defined class; accepts a class object or its name as a string. `Number` accepts both `Int` and `Double`. |
| `respond_to?(name)` | Check whether the receiver exposes the named method. Collection-only methods are not reported for unrelated scalar types. |
| `equals?(other)` | Perform Lucy value equality, including structural equality for supported collection values. |

Function values additionally expose `name()`, `arity()`, `required_arity()`, `parameters()`, and `call(args...)`. User-defined functions report their declared parameter counts; native functions without signature metadata report `-1` for `arity()` and `required_arity()`. Class values expose `name()`, `superclass()`, `methods()`, and `is_struct?()`. User instances expose `class_of()`, `fields()` / `to_h()`, `has_field?(name)`, and `methods()`.

### String methods

`String` additionally supports `to_int()` / `to_i()`, `to_float()` / `to_f()`, `capitalize()`, `chars()`, `index_of(text)`, and `empty?()`. `chars()` splits a UTF-8 string into Unicode code points. `index_of()` returns a zero-based code-point index or `nil`. `capitalize()` currently uppercases an ASCII first character; it does not claim full Unicode case mapping.

### Array methods

`Array` supports the existing indexed and higher-order methods plus `append(value, ...)`, `prepend(value, ...)`, `concat(array)`, `compact()`, `uniq()`, `find(predicate)`, `each_with_index(callback)`, and `reduce(callback[, initial])`. `append`, `prepend`, and `concat` mutate the receiver; `compact`, `uniq`, and `find` do not. `reduce` passes `(accumulator, element)` to its callback and returns `nil` for an empty array without an initial value. `each_with_index` passes `(element, zero_based_index)`.

`sum()`, `min()`, and `max()` require finite numeric elements. `sort()` returns a stable sorted copy and accepts either all-numeric or all-string arrays; mixed or unsupported element types raise `TypeError`. `take(count)` and `drop(count)` return copies and reject negative counts.

### Map methods

`Map` additionally supports `fetch(key[, default])`, `has_key?(key)`, `merge(other_map)`, `each(callback)`, `to_a()`, and `empty?()`. `fetch` raises `KeyError` for a missing key without a default. `merge` returns a new map and leaves both inputs unchanged. `each` calls the callback with `(key, value)`. `to_a` returns key/value pairs; map iteration order is unspecified.

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
| `to_int`, `to_i`, `to_float`, `to_f` | `value.to_i()`, `value.to_f()` | Strict numeric conversion; trailing junk is rejected. Float conversion also rejects non-finite results. |
| `slice` | `value.slice(start[, end])` | Returns code points from `start` up to but excluding `end`; negative indexes count from the end. |
| `char_at` | `value.char_at(index)` | Returns one code point or `nil` for an out-of-range index. |
| `split` | `value.split(separator)` | Splits on a non-empty separator. |
| `replace` | `value.replace(search, replacement)` | Replaces non-overlapping matches; an empty search string raises `ValueError`. |
| `capitalize`, `chars`, `index_of`, `empty?` | `value.chars()`, `value.index_of(part)` | First-character ASCII capitalization, UTF-8 code-point array, zero-based code-point search index (`nil` when absent), and emptiness check. |

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
| `append`, `prepend`, `concat` | `array.append(values...)`, `array.concat(other)` | Mutating aliases for append/prepend/concatenation; return the receiver. |
| `compact`, `uniq` | `array.compact()`, `array.uniq()` | Return copies with `nil` values or duplicate values removed. |
| `find`, `each_with_index`, `reduce` | `array.find(predicate)`, `array.reduce(callback[, initial])` | Find first matching value, iterate with zero-based indexes, or fold values. |
| `sum`, `min`, `max` | `array.sum()`, `array.min()`, `array.max()` | Numeric-only aggregate methods; empty min/max return `nil`. |
| `sort`, `take`, `drop` | `array.sort()`, `array.take(count)`, `array.drop(count)` | Stable homogeneous numeric/string sort and non-mutating prefix/suffix selection. |

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
| `fetch`, `has_key?` | `map.fetch(key[, default])`, `map.has_key?(key)` | Strict lookup with optional default and key-existence alias. |
| `merge`, `each`, `to_a`, `empty?` | `map.merge(other)`, `map.each(callback)`, `map.to_a()` | Non-mutating merge, `(key, value)` iteration, key/value pair conversion, and emptiness check. |

### `Int`, `Double`, and `Number`

| Method | Signature | Contract |
|---|---|---|
| `abs`, `floor`, `ceil`, `round` | `value.round()` | Numeric absolute value and rounding operations. |
| `sqrt`, `sin`, `cos`, `tan`, `log` | `value.sqrt()` | Numeric operations; square root/log domain errors are reported. Trigonometric inputs use radians. |
| `to_int`, `to_i`, `to_float`, `to_f`, `to_s`, `to_string` | `value.to_i()`, `value.to_f()`, `value.to_s()` | Numeric conversions and consistent string conversion. |
| `is_integer?`, `positive?`, `negative?`, `zero?`, `even?`, `odd?` | `value.is_integer?()` | Numeric predicates; parity methods require an integer receiver. |
| `clamp` | `value.clamp(minimum, maximum)` | Returns the receiver bounded to the inclusive interval; minimum must not exceed maximum. |
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
