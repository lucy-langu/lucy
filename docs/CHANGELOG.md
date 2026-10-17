# Lucy changelog

## 2.0.1 — reliability and tooling release

- Prevented `String.replace` from looping forever when the search string is empty; it now raises `ValueError`.
- Made `String.length`, `char_at`, `slice`, and `reverse` operate on UTF-8 code points instead of raw bytes. This does not promise grapheme-cluster segmentation.
- Made `String.to_int` and `String.to_float` reject trailing junk; non-finite float spellings are rejected by these conversion methods.
- Reworked CSV parsing to handle empty input and final newlines without synthetic records, and to reject malformed quoting.
- Added correct JSON `\uXXXX` surrogate-pair handling and rejection of isolated surrogates.
- Expanded edge-case regression coverage and documentation for the behaviors above.

## 2.0.0

Initial Lucy 2.0 language release. See [history and migration](HISTORY_AND_MIGRATION.md).
