# `text` standard library

Text, encoding, URL, pattern, and shell-escaping helpers.

## Import

```lucy
import text
```

## Public functions

- `match(pattern, text)`
- `search(pattern, text)`
- `find_all(pattern, text)`
- `replace_regex(pattern, replacement, text)`
- `base64_encode(text)`
- `base64_decode(text)`
- `hex_encode(text)`
- `hex_decode(text)`
- `url_encode(text)`
- `url_decode(text)`
- `scanner(text)`
- `shell_split(text)`
- `shell_escape(text)`
- `shell_join(items)`

## Example

```lucy
import text
println text.base64_encode "hello"
println text.hex_encode "hello"
```

## Source of truth

The public function list above is extracted from the current `stdlib/text.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
