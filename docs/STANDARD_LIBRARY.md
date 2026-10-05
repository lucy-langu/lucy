# Lucy 2.0.0 Standard Library Reference

The standard library is written primarily in Lucy and backed by a small native runtime boundary where operating-system or external-library facilities are required. The shipped modules are automatically loaded by the interpreter in 2.0.0.

## Public module index

| Module | Purpose |
|---|---|
| `app` | CLI parsing, logging, benchmarking, timeout, templates |
| `crypto` | digests, HMAC, TLS/OpenSSL-facing objects |
| `data` | collection transforms and JSON/YAML/CSV |
| `flow` | value-oriented flow helpers |
| `fs` | files, directories, paths, IO, links |
| `http` | HTTP, sockets, DNS |
| `math` | numeric helpers |
| `random` | random values and sampling |
| `repl` | REPL-facing helpers |
| `result` | success/error values |
| `runtime` | reflection, formatting, callbacks, collections |
| `set` | unique collections and set algebra |
| `sqlite` | SQLite database access when compiled with SQLite |
| `system` | OS, process, environment, shell, signals |
| `text` | regex, encoding, scanner, shell escaping |
| `time` | time, dates, datetime, sleep |

## `app`

### OptionParser

`OptionParser.new()` creates a parser. Configuration methods include `banner`, `separator`, `version`, `program_name`, `on`, `parse`, `parse!`, `help`, `summarize`, and `abort`.

Example:

```lucy
let parser = app.parser()
parser.program_name "lucy-tool"
parser.on("verbose", "enable verbose mode", false)
let options = parser.parse()
```

### Logger

`Logger.new(output = nil, level = 0)` creates a logger. Use `debug`, `info`, `warn`, `error`, and `fatal`. `level`, `set_level`, and `level_set` manage the current threshold. Predicate methods `debug?`, `info?`, `warn?`, `error?`, and `fatal?` report whether a level is active.

### Benchmark, Timeout, ERB

- `benchmark()` returns the benchmark helper; `Benchmark.realtime(command)` and `Benchmark.measure(command)` measure execution.
- `timeout()` returns the timeout helper; `Timeout.timeout(seconds, callback)` runs a callback with a time limit.
- `template(source)` creates an `ERB` template. `ERB.src`, `ERB.result(values)`, and `ERB.run(values)` expose the template source and rendering.

## `crypto`

The crypto module exposes digest and HMAC helpers plus OpenSSL-facing resource objects.

### Public functions

- `digest(text)` — digest text using the module's configured digest implementation.
- `hexdigest(text)` — hexadecimal digest representation.
- `hash(text)` — hash convenience operation.
- `base64digest(text)` — base64 digest representation.
- `file(path)` — digest a file.
- `hmac(key, data)` — HMAC operation.

### Classes

`Digest` provides `digest`, `hexdigest`, `base64digest`, and `file`.

`HMAC` provides `digest` and `hexdigest`.

`SSLContext`, `SSLSocket`, `Certificate`, `RSA`, `Cipher`, `X509`, `PKey`, and `OpenSSL` expose the current native/OpenSSL-facing object layer. Their constructors and helper methods are:

- `SSLContext.initialize(options)`
- `SSLSocket.initialize(socket, context)`, `connect(host, port)`, `close()`
- `Certificate.initialize(data)`
- `RSA.initialize(pem)`
- `Cipher.initialize(name)`
- `X509.certificate(data)`
- `PKey.rsa(pem)`
- `OpenSSL.hmac()`, `ssl_context(options)`, `ssl_socket(socket, context)`, `certificate(data)`, `rsa(pem)`, `x509()`, `pkey()`, `cipher(name)`

Use these lower-level classes when the simple digest/HMAC helpers are insufficient.

## `data`

### Collection helpers

`pick(source, keys)`, `omit(source, keys)`, `merge(left, right)`, `values(source, keys)`, and `zip(keys, values)` provide common map/collection transformations.

### JSON

`parse(text)`, `stringify(value)`, `pretty(value)`, `json_read(path)`, and `json_write(path, value)` are the flat public helpers.

`JSON.parse`, `JSON.stringify`, `JSON.pretty`, `JSON.load`, and `JSON.dump` expose the same functionality through the class-style API.

Example:

```lucy
let payload = data.parse "{\"name\":\"Lucy\"}"
println data.pretty payload
```

### YAML

`yaml_load(text)`, `yaml_dump(value)` and `YAML.load`, `YAML.dump`, `YAML.load_file` handle YAML values.

### CSV

`csv_parse(text, delimiter)`, `csv_stringify(rows, delimiter)` and `CSV.parse`, `CSV.stringify`, `CSV.load`, `CSV.dump` handle delimited tabular data.

## `flow`

- `pipe(value, steps)` — apply a sequence of operations to a value.
- `tap(value, action)` — run an action while preserving the original value.
- `branch(value, predicate, yes, no)` — select a path based on a predicate.
- `repeat(value, count, step)` — repeatedly transform a value.

Example:

```lucy
let result = flow.pipe 2, [lambda x => x + 1, lambda x => x * 10]
println result
```

## `fs`

### File content

`read`, `write`, `append`, `read_lines`, and `write_lines` operate on file contents.

### File state and mutation

`exists`, `size`, `remove`, `copy`, `move`, `touch`, `chmod`, `chown` inspect or modify filesystem entries.

### Directories

`entries`, `files`, `dirs`, `mkdir`, `rmdir`, `empty`, `glob`, and `walk` enumerate or manage directory trees.

### Paths

`join`, `absolute`, `expand`, `basename`, `dirname`, `extension`, and `stem` manipulate path names without requiring callers to build platform-specific separators themselves.

### Links and IO

`link`, `symlink`, `open`, and `console` expose links and stream-like IO. `IO` supports `print`, `write`, `read`, `ask`, `seek`, `pos`, `rewind`, `eof`, `fileno`, `close`, `pipe`, and `popen`.

### JSON convenience

`read_json(path)` and `write_json(path, value)` combine filesystem and data operations.

Example:

```lucy
fs.write "hello.txt", "Lucy"
println fs.read "hello.txt"
```

## `http`

### HTTP functions

- `request(method, url, body, headers, options)`
- `get(url, headers, options)`
- `post(url, body, headers, options)`
- `put(url, body, headers, options)`
- `patch(url, body, headers, options)`
- `delete(url, headers, options)`
- `head(url, headers, options)`
- `client(url, proxy)`

`HTTPRequest` models a configured request. `HTTP` provides a client object with `headers`, `timeout`, `connect_timeout`, `proxy`, `user_agent`, `follow_redirects`, `insecure`, and HTTP verb methods.

### Sockets

`connect`, `bind`, `listen`, `accept`, `recv`, `send`, and `close` are the flat socket helpers. `Socket` provides the object-oriented form. `SocketAPI.new()` provides the API object. `Resolv.getaddress(host)` and `Resolv.getname(address)` expose DNS operations; the flat forms are `resolve(host)` and `reverse(address)`.

Example:

```lucy
let response = http.get "https://example.com"
println response.status
```

## `math`

Public helpers:

- `square(x)`
- `cube(x)`
- `clamp(x, low, high)`
- `factorial(n)`
- `gcd(a, b)`
- `lcm(a, b)`
- `average(values)`
- `lerp(a, b, t)`
- `sign(x)`

`Math` exposes the same operations and additionally has `even(x)` and `odd(x)`.

## `random`

- `int(low, high)` — integer in the requested range.
- `float()` — random floating-point value.
- `bool()` — random boolean.
- `choice(items)` — choose one item.
- `shuffle(items)` — shuffle a collection.
- `sample(items, count)` — choose a sample.

## `repl`

- `banner()`
- `version()`
- `prompt(depth = 0)`
- `commands()`
- `topics()`
- `help(topic = nil)`

These helpers expose information used by the interactive environment.

## `result`

- `ok(value)` — construct a successful result.
- `err(message)` — construct an error result.
- `success(result)` — inspect success state.
- `unwrap(result, fallback)` — obtain a value or fallback.
- `message(result)` — obtain an error message.

Use this module when failure is part of normal data flow rather than an exceptional runtime condition.

## `runtime`

### Formatting and exception helpers

`printf`, `format`, `catch`, `rescue`, and `ensure` support formatted output and controlled callback execution.

### Reflection

`methods`, `responds`, `call`, `inspect`, `variables`, `globals`, `ancestors`, and `superclass` expose runtime information and dynamic invocation.

### Kernel and Collections

`Kernel` also exposes `p`, `pp`, `sprintf`, `system`, `spawn`, `trap`, `global_variables`, `local_variables`, `to_a`, `to_h`, `to_sym`, `singleton_class`, `proc`, and `lambda`.

`Collections` provides `first`, `last`, `reverse`, `contains`, `count`, `index`, `compact`, `unique`, `flatten`, `sum`, `min`, and `max`.

Example:

```lucy
println runtime.type value
println runtime.inspect value
println runtime.methods value
```

## `set`

`Set` is the unique-value collection. Construction and flat helpers are available through `new`, `from_values`, `add`, `delete`, `include?`, `union`, `intersection`, `difference`, `symmetric_difference`, `subset?`, `superset?`, and `intersect?`.

The object API additionally provides `member?`, `each`, `size`, `length`, `empty?`, `clear`, `map`, `select`, `reject`, `merge`, `subset`, `superset`, and `intersect`.

Example:

```lucy
let a = set.new [1, 2, 3]
let b = set.new [3, 4]
println a.union b
println a.intersection b
```

## `sqlite`

SQLite is optional at build time. When enabled:

```lucy
let db = sqlite.open "app.db"
db.execute "create table if not exists items (id integer, name text)"
```

`Database` provides `execute`, `query`, `prepare`, `begin`, `commit`, `rollback`, `changes`, `last_insert_id`, and `close`.

`Statement` provides `bind`, `execute`, `query`, and `close`.

When SQLite support is not compiled in, importing or opening SQLite reports a clear runtime/library error instead of silently pretending the backend exists.

## `system`

### Environment

`platform`, `version`, `argv`, `cwd`, `env`, `setenv`, `unsetenv`, `home`, `temp_dir`, and `command_exists` expose process environment information.

### Processes

`pid`, `ppid`, `run`, `capture`, `success`, `output`, `spawn`, `wait`, `waitpid`, and `kill` manage or inspect processes.

### Identity and clocks

`uid`, `gid`, `euid`, `egid`, `groups`, and `clock_gettime` expose OS identity and clock information where supported.

### Signals and users

`trap`, `signals`, `signal_name`, `login`, `user`, and `user_id` expose signal and user information.

### Shell safety helpers

`shell_split`, `shell_escape`, and `shell_join` are available for parsing and constructing shell command arguments.

## `text`

### Pattern operations

`match`, `search`, `find_all`, and `replace_regex` provide regex-oriented operations.

### Encoding

`base64_encode`, `base64_decode`, `hex_encode`, `hex_decode`, `url_encode`, and `url_decode` handle text/data encodings. These are different from numeric `bin`, `hex`, and `oct`: `text.hex_encode` encodes data, while numeric `hex(255)` formats an integer.

### Scanner

`scanner(text)` creates a `StringScanner`. Its methods are `scan`, `scan_until`, `skip`, `skip_until`, `check`, `check_until`, `match?`, `matched`, `matched_size`, `pre_match`, and `post_match`.

### Shell helpers

The module also provides `shell_split`, `shell_escape`, and `shell_join` for shell argument handling.

## `time`

### Time

`Time` provides `now`, `today`, `strptime`, `format`, `parts`, `year`, `month`, `day`, `hour`, `minute`, `second`, `add`, `subtract`, `plus`, `minus`, `compare`, `succ`, `add_ms`, and `subtract_ms`.

### Date

`Date` provides `parse`, `strptime`, `format`, `add`, `subtract`, `next_day`, `prev_day`, `succ`, `shift_months`, `add_months`, `subtract_months`, and `compare`.

### DateTime

`DateTime` provides `timestamp`, `format`, `parts`, `year`, `month`, `day`, `hour`, `minute`, `second`, `weekday`, and `iso`.

### Module helpers

`now`, `today`, `timestamp`, `parse`, `date`, `format`, and `sleep` are the flat entry points.

## Complete signature inventory

For an exact source-derived list of every shipped class and function signature, see `STANDARD_LIBRARY_API_INVENTORY.md`.

## Naming and compatibility rule

The public API is intentionally flat. Internal names beginning with `__` or private implementation helpers beginning with `_` are not stable application API. Documentation for old names is kept in migration/history material rather than mixed into current usage.
