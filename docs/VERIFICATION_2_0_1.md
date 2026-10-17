# Lucy 2.0.1 verification record

This file records what was executed for the 2.0.1 source package. It is a test log, not a claim that the interpreter is defect-free.

## Environment

- Linux x86-64 container
- GCC 14.2.0
- CMake 3.31 toolchain
- SQLite 3.46.1
- Debug and Release configurations
- SQLite explicitly enabled with `-DLUCY_ENABLE_SQLITE=ON`

## Results

| Check | Result |
|---|---|
| Fresh Debug build | Passed |
| Fresh Release build | Passed |
| CTest, SQLite enabled | 26/26 passed |
| CTest under AddressSanitizer + UndefinedBehaviorSanitizer | 26/26 passed (`ASAN_OPTIONS=detect_leaks=0`) |
| Shipped standalone examples | 25/25 exited successfully in this environment |
| End-to-end JSON config validator | Passed |
| End-to-end CSV report generator | Passed; fixture total was 179.73 |
| End-to-end SQLite task-store example | Passed |
| REPL piped smoke (`:version`, `:help string`, `:about`, `:reset`, `:quit`) | Passed |
| Source-derived standard-library documentation generator | Executed successfully |

## Added regression coverage

- UTF-8 code-point length, indexing, slicing, and reversal for two-, three-, and four-byte characters.
- Empty-string replacement rejection.
- Strict integer/float conversion rejection for trailing junk.
- CSV empty input, final newline, malformed quoting, and invalid delimiter.
- JSON surrogate-pair decoding and isolated-surrogate rejection.
- OptionParser `--name value`, `--name=value`, unknown options, missing values, unexpected positional arguments, and help metadata.
- Benchmark callback execution without shell invocation.

## Not verified

- Native Windows build/runtime and Windows console editing.
- macOS build/runtime and terminal behavior.
- Coverage-guided fuzzing at scale or long-duration stress testing.
- Exhaustive TLS certificate validation, HTTP timeout semantics, process API security, filesystem race conditions, or adversarial native extension behavior.
- Full Unicode grapheme segmentation or locale-independent Unicode case mapping.
- Hard cancellation for `app.timeout`; it remains post-hoc elapsed-time checking and cannot stop a blocked/infinite callback.

Re-run the current Linux checks with `./tools/verify_release.sh`. Cross-platform release claims require independent native CI jobs on those operating systems.
