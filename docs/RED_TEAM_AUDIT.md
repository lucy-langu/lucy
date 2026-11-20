# Lucy 2.0.1 — Red-team audit findings and 2.0.0 historical reproductions


## 2.0.1 disposition

The findings below record the original 2.0.0 observations. The following changes were made for 2.0.1 and have regression coverage on Linux/GCC:

| Finding | 2.0.1 status | Evidence / remaining boundary |
|---|---|---|
| R0-01 empty-pattern replacement hang | Fixed | Empty search pattern raises `ValueError`; covered by `21_reliability_regressions.lucy`. |
| R0-02 UTF-8 byte splitting | Partially fixed | `length`, `char_at`, `slice`, and `reverse` use code-point boundaries. Grapheme clusters and full Unicode case mapping remain unsupported. |
| R0-03 CSV synthetic rows | Fixed | Empty input returns an empty array; final newline does not create a record. Covered by regression tests. |
| R0-04 malformed CSV quoting | Fixed | Invalid quote placement and unterminated quotes are rejected; covered by expected-failure test. |
| R1-01 numeric prefix acceptance | Fixed | Trailing non-whitespace is rejected; regression coverage included. |
| R1-02 JSON surrogate handling | Fixed | Valid pairs are combined; isolated surrogates are rejected. Raw invalid UTF-8 validation remains incomplete. |
| R1-03 `Timeout` is not cancellation | Documented limitation | Callback execution cannot safely be interrupted by this synchronous runtime. Do not use as a hard deadline or sandbox. |
| R1-04 OptionParser validation | Partially fixed | Unknown options, missing values and unexpected positionals now fail; `--name=value` is supported. Short aliases, required-option metadata and automatic type conversion remain unsupported. |
| R1-05 empty CSV delimiter | Fixed | Delimiters must be one byte; empty/multi-byte values are rejected. |
| R1-06 Benchmark shell execution | Fixed | Benchmark now accepts a Lucy callback and does not pass the argument to a shell. Clock resolution is milliseconds. |

Validation performed for this disposition: fresh CMake Debug build with SQLite enabled, 26/26 CTest cases passing, and three runnable end-to-end project examples. Windows/macOS builds, coverage-guided fuzzing, and adversarial network/security testing remain unverified. ASan/UBSan Debug build and the full CTest suite passed on Linux.

---

**Audit scope:** the supplied source archive, fresh Linux/GCC build, SQLite-enabled build, CTest, all shipped examples, independent documentation-snippet execution, three end-to-end scripts, string/CSV/JSON edge cases, and 500 randomized malformed source inputs.

**Important limits:** no Windows/macOS execution was available in this run; no external HTTP behavior was considered verified because the environment did not provide reliable network access; the 500-input run is a small smoke fuzz pass, not coverage-guided fuzzing. Absence of a crash in that pass is not evidence of memory safety.

## Severity 0 — can hang or corrupt data

### R0-01: empty-string replacement can loop forever

**Location:** `src/runtime.cpp`, native string `replace` implementation.

Reproduction:

```lucy
println "abc".replace("", "")
```

Observed: the process did not terminate within a two-second timeout. The loop searches for the empty string and advances by `to.size()`, which is zero in this case. This can permanently hang any script that reaches the call. The process must be killed externally.

**Required fix:** reject an empty `from` string with `ValueError`, or implement documented empty-pattern semantics with guaranteed forward progress. Add a regression test with a process-level timeout.

### R0-02: Unicode string operations split UTF-8 sequences

**Location:** `src/runtime.cpp`, string methods.

Observed with `"é"`:

- `length()` returns `2`, because it counts bytes.
- `char_at(0)` returns only the first UTF-8 byte, not the character.
- `reverse()` reverses the two UTF-8 bytes and emits invalid UTF-8.

The same design affects `slice()` and ASCII-only `toupper`/`tolower` transforms. This is not merely a display issue: returned strings can be malformed and downstream JSON, terminal, filesystem, or network consumers may reject them.

**Required fix:** explicitly define strings as byte strings and rename/document APIs accordingly, or implement UTF-8-aware code-point operations and separate grapheme-aware operations where required. Add tests for two-, three-, and four-byte sequences, combining marks, emoji, and invalid UTF-8.

### R0-03: CSV parser invents records for empty input and trailing newlines

**Location:** `src/phase2.cpp`, `parse_csv`.

Observed:

- `data.csv.parse("")` returns `[ [""] ]` rather than an empty document/record set under the likely expected contract.
- `data.csv.parse("a,b\n")` returns `[["a","b"],[""]]`.
- CRLF input ending in CRLF also produces an extra blank row.
- The shipped `csv_pipeline.lucy` demonstrates this in a realistic parse/stringify/parse workflow.

**Required fix:** distinguish EOF from a real line terminator, define empty-file semantics, and test LF, CRLF, final newline/no final newline, empty fields, blank lines, quoted newlines, escaped quotes, malformed quotes, and custom delimiters.

### R0-04: CSV parser accepts malformed quoting

**Location:** `src/phase2.cpp`, `parse_csv`.

Observed: an input such as `a"b,c\n` is accepted and returned as data instead of being rejected as malformed CSV. The parser toggles quote state only when a quote occurs at the start of a field; quotes inside unquoted fields are retained as ordinary characters. It also does not enforce that only a delimiter or record terminator may follow a closing quote.

**Required fix:** implement an explicit CSV state machine (field start, unquoted, quoted, after closing quote) and test malformed transitions. Document the accepted dialect if strict RFC-style behavior is not intended.

## Severity 1 — incorrect results or API contract violations

### R1-01: numeric string conversions accept trailing junk

**Location:** `src/runtime.cpp`, `String.to_int()` and `String.to_float()`.

Observed:

```lucy
println "12x".to_int()      # returns 12
println "1.25abc".to_float() # returns 1.25
```

`std::stoll`/`std::stod` are called without checking how many characters were consumed. Inputs that look invalid are silently truncated to a valid numeric prefix.

**Required fix:** require complete consumption after permitted whitespace, define sign/overflow behavior, and test suffixes, empty strings, whitespace, exponent tails, overflow, underflow, NaN/Inf spellings, and locale independence.

### R1-02: JSON escaped surrogate pairs are not combined

**Location:** `src/phase2.cpp`, JSON parser `parse_unicode_escape`.

Observed parsing `"\\uD83D\\uDE00"` yields the UTF-8 bytes for two surrogate code points (`ED A0 BD ED B8 80`) rather than the valid UTF-8 encoding of U+1F600. Surrogates are not Unicode scalar values; this output is invalid UTF-8.

**Required fix:** combine high/low surrogate pairs, reject isolated surrogates, and test BMP values, supplementary-plane values, lone high/low surrogates, malformed hex, and truncated escapes.

### R1-03: `app.timeout` is not a timeout/cancellation mechanism

**Location:** `stdlib/app.lucy`, `Timeout.timeout`.

The implementation runs `callback()` to completion, checks elapsed time afterward, and only then raises. A callback that blocks forever or hangs never reaches the check. Blocking I/O is not cancelled and work is not interrupted.

**Required fix:** rename this helper to reflect post-hoc duration checking, or implement actual cooperative cancellation/scheduler support. Documentation must not imply it protects against non-terminating work.

### R1-04: command-line parser silently ignores unsupported/invalid input

**Location:** `stdlib/app.lucy`, `OptionParser.parse`.

Observed from code inspection:

- unknown `--options` are ignored;
- short options are not implemented;
- `--name=value` is not parsed;
- a value-taking option with no following value can silently retain its default;
- there is no required-option validation or typed conversion;
- `on()` expects the bare name (for example `name`), not `--name`, because `parse()` strips the leading dashes before lookup.

**Required fix:** reject unknown flags by default, support or explicitly document unsupported forms, distinguish boolean flags from value options, detect missing values, and test duplicate flags and positional arguments.

### R1-05: delimiter argument can be an empty string

**Location:** `src/phase2.cpp`, `__csv_parse` / `__csv_stringify` builtins.

The implementation reads `string_arg(args[1], ...)[0]` without validating that the delimiter string is non-empty. An empty delimiter is not rejected and effectively becomes a NUL byte on this standard-library implementation; a multi-byte delimiter is silently truncated to its first byte. This is a contract bug, but I did not establish an out-of-bounds access here.

**Required fix:** reject empty delimiters and define whether non-ASCII delimiters are supported; the current implementation silently takes the first byte. Add a regression test.


### R1-06: Benchmark API executes a shell command rather than measuring a Lucy callback

**Location:** `stdlib/app.lucy` (`Benchmark.realtime`) and `src/stdlib_native.cpp` (`__benchmark_realtime`).

The public documentation described callback timing, but the native implementation requires a string and passes it to the process command runner. A callback fails with `TypeError: Benchmark.realtime expects a string`. This is a contract/documentation mismatch and creates command-execution risk if untrusted text reaches the API.

**Required fix:** either implement timing of callable values in the runtime, or rename the method to make shell execution explicit and document platform-dependent command syntax. Never interpolate untrusted input into the command string.

## Severity 2 — documentation and discoverability failures

### R2-01: API examples refer to nonexistent or mislocated APIs

The initial audit found these documented examples failing against the shipped implementation:

- `text.upper("hello")`: `upper` is a String instance method; use `"hello".upper()`.
- `runtime.type`: no public function with this name is defined in `stdlib/runtime.lucy`.
- `result.success()` and `result.unwrap(0)`: public functions require the result map as the first argument; the example also shadows the module name when it uses `result` for both.
- `app.template("Hello $name")`: the ERB implementation substitutes `{{name}}`, not `$name`.
- Benchmark examples claimed callback timing; the native implementation instead requires a shell command string. The earlier Lambda-based example therefore fails with a type error, not merely a syntax issue.
- The option parser example registered `--name` while the implementation expects `name`.

Some of these examples were corrected during this audit in `STANDARD_LIBRARY.md` and `PROGRAMMING_GUIDE.md`. The source implementation was not changed by those documentation edits.

### R2-02: standalone execution of documentation code blocks is not a valid universal test

The first automated pass attempted every `lucy` fenced block as a standalone program. Many failed because they intentionally omit setup, refer to a fictional function such as `work()`, depend on files not created in the block, or require live network access. The documentation does not machine-label blocks as standalone, illustrative fragments, expected errors, or network-dependent examples.

**Required fix:** add metadata or conventions for snippet classes and a CI harness that runs all standalone/offline snippets, asserts expected failures for negative examples, and skips only explicitly tagged environment-dependent examples.

### R2-03: API inventory claims source-derived completeness but is not generated

`STANDARD_LIBRARY_API_INVENTORY.md` says it is generated from source and should be regenerated, but the archive does not provide a generator/verification command that proves completeness. An inventory can drift even if the prose claims it is exhaustive.

**Required fix:** add a deterministic generator and CI diff/check. Decide explicitly whether class methods, native builtins, aliases, and REPL-only commands are public API.

### R2-04: installation and platform claims exceed the tested matrix

The code includes platform-specific branches and Windows networking linkage, but this audit only executed Linux/GCC. The archived `build-install` directory contains stale build state and absolute paths; moving a build tree is not a supported reproducible-build test. No Windows or macOS result is established by this report.

**Required fix:** add clean CI jobs for Linux, Windows (MSVC and MinGW if both are claimed), and macOS; test install from a fresh prefix, runtime library discovery, `LUCY_PATH`, SQLite enabled/disabled, and invocation from arbitrary working directories.

## Build and test observations

- Fresh Linux/GCC Debug build with `LUCY_ENABLE_SQLITE=ON`: successful.
- CTest with SQLite disabled: 20/20 passed.
- CTest with SQLite enabled: 21/21 passed.
- CTest under AddressSanitizer + UndefinedBehaviorSanitizer with SQLite enabled: 21/21 passed. This does not cover every native path; the known string hang and data-format edge cases were established separately.
- Shipped examples: 25/25 exited successfully in the tested environment. This is only a smoke check; some examples have side effects and external-service assumptions.
- Three small end-to-end scripts (SQLite task CRUD/transaction rollback, CSV/JSON data pipeline, and CLI/file-backed notes) completed. The CSV project exposed the trailing-blank-row bug.
- Random malformed-source smoke fuzz: 500 inputs, no process crashes or timeouts. This does not replace libFuzzer/AFL++ with sanitizers.
- A build with additional warnings enabled surfaced an unused `longest_common_prefix` helper in `src/repl.cpp` (`-Wunused-function`).

## Required next test campaign

1. Build and run under ASan + UBSan; include invalid delimiter, empty string replace, malformed JSON, huge numbers, deeply nested expressions/collections, and exception/finally paths.
2. Add coverage-guided fuzz targets for lexer, parser, JSON parser, CSV parser, format strings, and extension boundary.
3. Add semantic differential tests for integer overflow, division/modulo by zero, floating-point NaN/Inf, equality/hash behavior for maps/arrays, closure capture, shadowing, recursion depth, and mutation during iteration.
4. Add stress tests for large files/strings, deeply nested input, repeated imports, repeated SQLite open/close, socket errors, subprocess timeouts, and memory growth in long-lived REPL sessions.
5. Add security tests for command execution argument quoting, path traversal, symlink handling, filesystem permissions, TLS certificate validation, HTTP redirects, response-size limits, and secrets in exception/log output.
6. Define contracts for Unicode, CSV dialect, JSON surrogate handling, timeout behavior, CLI parser behavior, and exit-code semantics before declaring 2.0 stable.
