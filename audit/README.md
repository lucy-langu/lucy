# Lucy 2.0.1 audit and regression projects

These are project-level probes retained from the 2.0.0 red-team audit and updated for the 2.0.1 fixes. Run from the repository root after a fresh build with SQLite enabled:

```sh
cmake -S . -B build-audit -DCMAKE_BUILD_TYPE=Debug -DLUCY_ENABLE_SQLITE=ON
cmake --build build-audit -j2
ctest --test-dir build-audit --output-on-failure
export LUCY_PATH="$PWD/stdlib"
./build-audit/lucy audit/projects/todo_sqlite.lucy
./build-audit/lucy audit/projects/csv_pipeline.lucy
./build-audit/lucy audit/projects/cli_notes.lucy
./build-audit/lucy audit/projects/edge_cases.lucy
```

The projects exercise database persistence/transactions, CSV/JSON serialization, command-line parsing, and the previously reported string/CSV/JSON/numeric edge cases. `edge_cases.lucy` now asserts that malformed inputs are rejected and that Unicode operations preserve valid UTF-8. They are integration probes, not production-ready applications.
