# `text` standard library — Lucy 2.0.7 · module 1.1.0

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
| `blank` | `blank(value)` |
| `present` | `present(value)` |
| `truncate` | `truncate(value, max_length, suffix = "...")` |
| `pad_left` | `pad_left(value, width, fill = " ")` |
| `pad_right` | `pad_right(value, width, fill = " ")` |
| `remove_prefix` | `remove_prefix(value, prefix)` |
| `remove_suffix` | `remove_suffix(value, suffix)` |
| `before` | `before(value, delimiter)` |
| `after` | `after(value, delimiter)` |
| `count_occurrences` | `count_occurrences(value, needle)` |
| `equals_ignore_case` | `equals_ignore_case(left, right)` |
| `words` | `words(value)` |
| `join` | `join(items, separator = "")` |
| `lines` | `lines(value)` |
| `ensure_prefix` | `ensure_prefix(value, prefix)` |
| `ensure_suffix` | `ensure_suffix(value, suffix)` |
| `indent` | `indent(value, prefix = "    ")` |
| `dedent` | `dedent(value)` |
| `snake_case` | `snake_case(value)` |
| `kebab_case` | `kebab_case(value)` |
| `title_case` | `title_case(value)` |
| `reverse_words` | `reverse_words(value)` |

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

## Text normalization and formatting helpers

These functions are composed from the runtime's existing UTF-8-aware string methods. Case conversion remains ASCII-oriented, matching the core string contract.

| Function | Purpose |
|---|---|
| `blank(value)` | True when the stripped string is empty. |
| `present(value)` | Negation of `blank`. |
| `truncate(value, max_length, suffix = "...")` | Limit code-point length and append a suffix when truncated. |
| `pad_left(value, width, fill = " ")` | Left-pad with one code point. |
| `pad_right(value, width, fill = " ")` | Right-pad with one code point. |
| `remove_prefix(value, prefix)` | Remove a matching prefix. |
| `remove_suffix(value, suffix)` | Remove a matching suffix. |
| `before(value, delimiter)` | Return text before the first delimiter. |
| `after(value, delimiter)` | Return text after the first delimiter. |
| `count_occurrences(value, needle)` | Count non-overlapping occurrences. |
| `equals_ignore_case(left, right)` | Compare after ASCII-oriented lowercase conversion. |
| `words(value)` | Split on spaces and discard empty tokens. |
| `join(items, separator = "")` | Join stringified values. |
| `lines(value)` | Normalize CRLF/CR and split into lines. |
| `ensure_prefix(value, prefix)` | Add a missing prefix. |
| `ensure_suffix(value, suffix)` | Add a missing suffix. |
| `indent(value, prefix = "    ")` | Prefix each line. |
| `dedent(value)` | Remove the minimum leading whitespace count from non-blank lines. |
| `snake_case(value)` | Convert space/hyphen-separated words to lowercase underscores. |
| `kebab_case(value)` | Convert space/underscore-separated words to lowercase hyphens. |
| `title_case(value)` | Capitalize the first ASCII character of each word. |
| `reverse_words(value)` | Reverse word order. |
