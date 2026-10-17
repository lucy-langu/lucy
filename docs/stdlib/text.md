# `text` standard library — Lucy 2.0.1

Regular expressions, Base64/hex/URL encoding, shell escaping, and string scanner.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import text
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println text.match("^lucy", "lucy-lang")
println text.hex_encode("Lucy")
println text.url_encode("hello world")
```

## Module-level functions

| Function | Signature |
|---|---|
| `match` | `match(pattern, text)` |
| `search` | `search(pattern, text)` |
| `find_all` | `find_all(pattern, text)` |
| `replace_regex` | `replace_regex(pattern, replacement, text)` |
| `base64_encode` | `base64_encode(text)` |
| `base64_decode` | `base64_decode(text)` |
| `hex_encode` | `hex_encode(text)` |
| `hex_decode` | `hex_decode(text)` |
| `url_encode` | `url_encode(text)` |
| `url_decode` | `url_decode(text)` |
| `scanner` | `scanner(text)` |
| `shell_split` | `shell_split(text)` |
| `shell_escape` | `shell_escape(text)` |
| `shell_join` | `shell_join(items)` |

## Classes and methods

### `StringScanner`

| Method | Signature |
|---|---|
| `initialize` | `initialize(text)` |
| `scan` | `scan(pattern)` |
| `scan_until` | `scan_until(pattern)` |
| `skip` | `skip(pattern)` |
| `skip_until` | `skip_until(pattern)` |
| `check` | `check(pattern)` |
| `check_until` | `check_until(pattern)` |
| `match?` | `match?()` |
| `matched` | `matched()` |
| `matched_size` | `matched_size()` |
| `pre_match` | `pre_match()` |
| `post_match` | `post_match()` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/text.lucy`](../../stdlib/text.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
