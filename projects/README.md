# End-to-end Lucy projects

Run these from the repository root after building `lucy` and setting `LUCY_PATH` to the repository's `stdlib` directory.

```sh
LUCY_PATH=./stdlib ./build/lucy projects/config_validator.lucy
LUCY_PATH=./stdlib ./build/lucy projects/sales_report.lucy
```

SQLite example (requires a build configured with `-DLUCY_ENABLE_SQLITE=ON`):

```sh
LUCY_PATH=./stdlib ./build/lucy projects/sqlite_task_store.lucy
```

The examples use `projects/fixtures/` for generated outputs and the SQLite database. Delete generated files before rerunning if you want a clean fixture state. These are small integration examples, not production-ready applications: they intentionally omit configuration discovery, structured logging, concurrency, migrations, and a comprehensive test suite.
