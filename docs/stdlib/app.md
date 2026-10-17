# `app` standard library — Lucy 2.0.2

Command-line option parsing, logging facade, callable benchmarking, post-hoc duration checks, and templates.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import app
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
let parser = app.parser()
parser.on("output", "output path", "report.csv")
let options = parser.parse(ARGV)
println options.get("output")

let elapsed = app.benchmark().realtime(lambda() => 1 + 1)
println elapsed
```

## Exported module-level objects

`benchmark`, `erb`, `logger`, `option_parser`, `timeout`

## Module-level functions

| Function | Signature |
|---|---|
| `parser` | `parser()` |
| `logger` | `logger(output = nil, level = 0)` |
| `benchmark` | `benchmark()` |
| `timeout` | `timeout()` |
| `template` | `template(source)` |

## Classes and methods

### `OptionParser`

| Method | Signature |
|---|---|
| `initialize` | `initialize()` |
| `new` | `new()` |
| `banner` | `banner(text)` |
| `separator` | `separator(text = "")` |
| `version` | `version(text)` |
| `program_name` | `program_name(text)` |
| `on` | `on(name, description = "", default = nil)` |
| `parse` | `parse(arguments = ARGV)` |
| `parse!` | `parse!(arguments = ARGV)` |
| `help` | `help()` |
| `summarize` | `summarize()` |
| `abort` | `abort(message)` |

### `Logger`

| Method | Signature |
|---|---|
| `initialize` | `initialize(output = nil, level = 0)` |
| `new` | `new(output = nil, level = 0)` |
| `level` | `level()` |
| `set_level` | `set_level(value)` |
| `level_set` | `level_set(value)` |
| `add` | `add(level, message)` |
| `log` | `log(level, message)` |
| `debug` | `debug(message)` |
| `info` | `info(message)` |
| `warn` | `warn(message)` |
| `error` | `error(message)` |
| `fatal` | `fatal(message)` |
| `debug?` | `debug?()` |
| `info?` | `info?()` |
| `warn?` | `warn?()` |
| `error?` | `error?()` |
| `fatal?` | `fatal?()` |

### `Benchmark`

| Method | Signature |
|---|---|
| `realtime` | `realtime(callback)` |
| `measure` | `measure(callback)` |

### `Timeout`

| Method | Signature |
|---|---|
| `timeout` | `timeout(seconds, callback)` |

### `ERB`

| Method | Signature |
|---|---|
| `initialize` | `initialize(template)` |
| `src` | `src()` |
| `result` | `result(values = {})` |
| `run` | `run(values = {})` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

### Important behavioral contracts

- `OptionParser` accepts `--name value` and `--name=value`; boolean options are declared by setting a boolean default. Unknown flags, missing values, and positional arguments are errors. Short aliases, automatic type conversion, and required-option metadata are not implemented.
- `Benchmark.realtime(callback)` and `Benchmark.measure(callback)` time a Lucy callback without launching a shell. The timer has millisecond resolution.
- `Timeout.timeout(seconds, callback)` checks elapsed time only after the callback returns. It cannot cancel a blocked or infinite callback.
- `Logger` is a lightweight facade and should not be assumed to provide a full logging backend or structured sink without inspecting the current implementation.

## Source of truth

Implementation: [`stdlib/app.lucy`](../../stdlib/app.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
