# Lucy changelog

## 2.0.2 — REPL usability, introspection, and documentation synchronization

- Removed ANSI color styling from the REPL banner and prompts; retained cursor-control sequences required for editing.
- Updated Tab completion to expand a common prefix, show candidates on the next Tab, and cycle on subsequent Tabs.
- Added `:vars`, `:vars --all`, `:functions`, `:modules`, `:where <name>`, and `:methods <Type>` commands.
- Added source-file tracking for user-defined functions and module-path inspection; interactive definitions correctly report that line metadata is unavailable.
- Restricted executable-derived module discovery to recognizable Lucy install roots so a build-tree executable does not scan arbitrary parent directories.
- Included built-in type extension methods in completion and added runnable examples/tests for type extensions and module discovery.
- Fixed `len(string)` to count UTF-8 code points, made `float(string)` reject trailing junk/non-finite values, and implemented the documented `Map.length()`/`Map.size()` methods.
- Added a consolidated native built-in reference and an executable documentation-example checker; updated the language reference, REPL manual, and source-indexed standard-library pages to version 2.0.2.

## 2.0.1 — reliability and tooling release

- Prevented `String.replace` from looping forever when the search string is empty; it now raises `ValueError`.
- Made `String.length`, `char_at`, `slice`, and `reverse` operate on UTF-8 code points instead of raw bytes. This does not promise grapheme-cluster segmentation.
- Made `String.to_int` and `String.to_float` reject trailing junk; non-finite float spellings are rejected by these conversion methods.
- Reworked CSV parsing to handle empty input and final newlines without synthetic records, and to reject malformed quoting.
- Added correct JSON `\uXXXX` surrogate-pair handling and rejection of isolated surrogates.
- Expanded edge-case regression coverage and documentation for the behaviors above.

## 2.0.0

Initial Lucy 2.0 language release. See [history and migration](HISTORY_AND_MIGRATION.md).
