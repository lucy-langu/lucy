# Lucy 2.0.1 — Real-world workflows and engineering patterns

This guide demonstrates how the language and standard library fit together in complete workflows. Every code block is intended to be copied into a `.lucy` file after replacing paths and environment-specific values. Network examples are deliberately excluded from automated CI because public endpoints are not deterministic dependencies.

## 1. Validate and transform JSON configuration

Use JSON parsing for syntax, then perform explicit application-level validation. Parsing alone does not validate your application's schema.

```lucy
let raw = fs.read("config.json")
let config = data.json.parse(raw)

if !config.has("service") {
    throw "ConfigurationError: missing service"
}
if !config.has("port") {
    throw "ConfigurationError: missing port"
}

let service = config.get("service")
let port = config.get("port").to_int()
if port < 1 or port > 65535 {
    throw "ConfigurationError: port must be between 1 and 65535"
}

let normalized = {
    service: service,
    port: port,
    enabled: config.has("enabled") ? config.get("enabled") : true
}
fs.write("config.normalized.json", data.json.pretty(normalized))
println "Configuration validated and normalized."
```

Engineering notes:

- Validate the type of each field before calling conversion methods if the input is untrusted.
- Treat missing keys separately from keys explicitly set to `nil`.
- Write to a temporary file and rename it if a partially written configuration would be harmful.
- Do not store credentials in committed example configuration files.

## 2. CSV ingestion and reporting

```lucy
let input = fs.read("sales.csv")
let rows = data.csv_parse(input)
if rows.length() == 0 {
    throw "DataError: sales.csv has no records"
}

let header = rows[0]
if header.length() < 3 or header[0] != "product" or header[1] != "quantity" or header[2] != "unit_price" {
    throw "DataError: expected product,quantity,unit_price columns"
}

let report = [["product", "quantity", "revenue"]]
var total = 0.0
var i = 1
while i < rows.length() {
    let row = rows[i]
    if row.length() != 3 {
        throw "DataError: malformed row " + i.to_string()
    }
    let quantity = row[1].to_int()
    let unit_price = row[2].to_float()
    if quantity < 0 or unit_price < 0 {
        throw "DataError: negative quantity or price"
    }
    let revenue = quantity * unit_price
    total += revenue
    report.push([row[0], quantity.to_string(), revenue.to_string()])
    i += 1
}
report.push(["TOTAL", "", total.to_string()])
fs.write("sales-report.csv", data.csv_stringify(report))
println "Wrote sales-report.csv; total revenue = " + total.to_string()
```

The loop uses a strict `i < rows.length()` bound, so a file containing only a header produces a header-only report without indexing beyond the array.

CSV is not a database and does not infer column types. Every value is initially text. The parser rejects malformed quote placement and unterminated quoted fields, but the application must still validate column count, headers, numeric ranges, duplicate records, and business rules.

## 3. Transactional SQLite workflow

Build Lucy with `-DLUCY_ENABLE_SQLITE=ON` and use prepared parameters for data values.

```lucy
let db = sqlite.open("tasks.db")
db.execute("CREATE TABLE IF NOT EXISTS tasks (id INTEGER PRIMARY KEY, title TEXT NOT NULL, done INTEGER NOT NULL DEFAULT 0)")

db.begin()
try {
    db.execute("INSERT INTO tasks (title, done) VALUES (?, ?)", ["Review Lucy release", 0])
    db.execute("INSERT INTO tasks (title, done) VALUES (?, ?)", ["Run regression suite", 0])
    db.commit()
} catch Exception as error {
    db.rollback()
    throw error
}

let tasks = db.query("SELECT id, title, done FROM tasks ORDER BY id")
for task in tasks {
    println task
}
db.close()
```

Use placeholders for values rather than concatenating strings into SQL. Identifiers such as table names cannot generally be bound as values; if identifiers must be dynamic, choose them from a strict allowlist. Close database handles deterministically, especially in long-running processes.

## 4. Small command-line utility

```lucy
let parser = app.parser()
parser.banner("Usage: lucy report --input PATH --output PATH")
parser.on("input", "CSV input path", nil)
parser.on("output", "CSV report path", "report.csv")
let options = parser.parse(ARGV)

if options.get("input") == nil {
    parser.abort("--input is required")
}
let content = fs.read(options.get("input"))
let rows = data.csv_parse(content)
fs.write(options.get("output"), data.csv_stringify(rows))
println "Wrote " + options.get("output")
```

`OptionParser` is intentionally compact, not a full POSIX/GNU parser. In 2.0.1 it accepts `--name value` and `--name=value`, boolean options whose default is a boolean, and rejects unknown options, missing values, and unexpected positional arguments. It does not implement short aliases, automatic type conversion, required-option metadata, or `--` positional argument collection. The sample's required check is therefore application-level.

## 5. Measure a callable without shell execution

```lucy
func calculate() {
    let values = [1, 2, 3, 4, 5]
    return values.map(lambda value => value * value)
}

let seconds = app.benchmark().realtime(lambda() => calculate())
println "elapsed seconds: " + seconds.to_string()
```

The callback runs in the current process; it is not passed to a shell. The timer uses a millisecond clock, so microbenchmarks are noisy and can return `0.0`. For serious performance comparisons, warm up the code, repeat it many times, isolate the machine, and report distributions rather than a single timing.

## 6. Operational and security checklist

- Never pass untrusted text to `system.run`, `system.capture`, `system.spawn`, `IO.popen`, or another shell/process API.
- Prefer argument arrays and safe quoting where the API supports them; shell escaping is platform-specific.
- Set explicit timeouts at the network/client layer. Lucy's `app.timeout` is post-hoc and cannot stop a blocked callback.
- Treat JSON, CSV, YAML, and environment variables as untrusted input. Parse success is not schema validation.
- Use SQLite prepared statements for data values and explicit transactions for multi-step changes.
- Check filesystem permissions and symlink behavior before using `fs.remove`, `fs.move`, `fs.chmod`, or `fs.chown` in privileged processes.
- Do not assume `http.get` validates certificates or applies a particular timeout unless the current platform/build configuration confirms it.
- Keep secrets out of source files, shell history, REPL history, logs, and crash reports.
- Run `ctest --output-on-failure` after every runtime or standard-library change; add a regression test for every defect.
