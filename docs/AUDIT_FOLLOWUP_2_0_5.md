# Lucy 2.0.5 — Windows build follow-up

## Root cause

The Windows GitHub Actions job failed in MSVC while compiling `src/runtime.cpp` and `src/stdlib_native.cpp`. The diagnostics showed Windows `min`/`max` macros expanding at calls to `std::min` and `std::max`. The resulting parse errors caused secondary diagnostics such as local function definitions being illegal and helper names appearing undefined.

## Fix

The `lucy_runtime` target now defines `NOMINMAX` when `WIN32` is true. Because both affected translation units belong to that target, the definition is applied consistently before their headers are preprocessed. The fix is target-scoped and does not alter non-Windows builds.

## Verification status

- Source and version metadata updated to `2.0.5`.
- The GitHub Actions workflow already builds on Ubuntu, Windows, and macOS in Debug and Release; rerunning it is the authoritative check for MSVC.
- Do not treat the Windows fix as verified until the Windows jobs complete successfully.
