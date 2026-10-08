# Lucy 2.0.0 — Standard Library Reference

This is the canonical user-facing standard-library reference. Every shipped module is covered here, including the public functions/classes, common usage patterns, return conventions, and important platform/build notes.

The final section contains the complete source-derived API inventory. That inventory is intentionally exhaustive: if a public function or class is shipped in `stdlib/*.lucy`, it should appear there.

## 1. Standard-library model

Lucy 2.0.0 automatically loads these shipped modules:

```text
app crypto data flow fs http math random repl result runtime set sqlite system text time
```

Therefore this works without an explicit import:

```lucy
println fs.read("README.md")
println text.upper("hello")
println system.platform()
```

Imports remain useful for user modules, packages, and explicit selective bindings:

```lucy
import text as t
from fs import read
```

Public APIs are intentionally shallow. Prefer `fs.read(...)` over deep namespace chains.

Names beginning with `_` or `__` are implementation details unless a public reference explicitly lists them.

---

# 2. `app` — command-line applications and utilities

`app` contains application-level helpers written in Lucy.

### OptionParser

Use `OptionParser` for simple command-line option parsing:

```lucy
let parser = app.parser()
parser.banner("Lucy example")
parser.on("--name", "user name", "Lucy")
let options = parser.parse(ARGV)
println options
```

Important methods include `banner`, `separator`, `version`, `program_name`, `on`, `parse`, `help`, `summarize`, and `abort`.

### Logger

```lucy
let log = app.logger()
log.info("server started")
log.warn("configuration is incomplete")
```

Levels are controlled with `level`, `set_level`, and `level_set`. Predicate methods such as `debug?()` and `error?()` can be used before expensive log construction.

### Benchmark

```lucy
let elapsed = app.benchmark().realtime(lambda => work())
println elapsed
```

`Benchmark.realtime` and `Benchmark.measure` accept callbacks.

### Timeout

`app.timeout().timeout(seconds, callback)` runs a callback with the library's timeout behavior. Timeout semantics are implemented by the shipped library/runtime and should not be confused with a general thread scheduler.

### ERB

`ERB` provides template rendering from a template string:

```lucy
let template = app.template("Hello $name")
println template.result({"name": "Lucy"})
```

---

# 3. `crypto` — hashing and native cryptographic facilities

`crypto` exposes digest/HMAC helpers and native OpenSSL-backed objects where the build provides them.

Common helpers:

```lucy
println crypto.hexdigest("hello")
println crypto.base64digest("hello")
println crypto.hmac("secret", "message")
```

The module also exposes `Digest`, `HMAC`, `SSLContext`, `SSLSocket`, `Certificate`, `RSA`, `Cipher`, `X509`, `PKey`, and `OpenSSL` classes/factories.

Crypto operations can fail with native/runtime errors. Availability depends on the build and linked crypto support.

---

# 4. `data` — JSON, YAML, CSV, and collection transforms

### Basic transforms

```lucy
let selected = data.pick(user, ["name", "email"])
let without_secret = data.omit(user, ["password"])
let combined = data.merge(left, right)
let pairs = data.zip(keys, values)
```

### JSON

```lucy
let object = data.parse("{\"name\":\"Lucy\"}")
println data.stringify(object)
println data.pretty(object)

data.json_write("config.json", object)
let loaded = data.json_read("config.json")
```

The `JSON` class exposes the same family through `JSON.parse`, `JSON.stringify`, `JSON.pretty`, `JSON.load`, and `JSON.dump`.

### YAML

```lucy
let value = data.yaml_load("name: Lucy")
println data.yaml_dump(value)
```

### CSV

```lucy
let rows = data.csv_parse("name,age\nLucy,2")
println data.csv_stringify(rows)
```

---

# 5. `flow` — small functional control helpers

```lucy
let result = flow.pipe(
    5,
    [lambda x => x * 2, lambda x => x + 1]
)

flow.tap(result, lambda x => println x)
```

Public functions:

- `pipe(value, steps)` — applies callbacks in order.
- `tap(value, action)` — performs an action and preserves the value.
- `branch(value, predicate, yes, no = nil)` — selects a callback based on a predicate.
- `repeat(value, count, step)` — repeatedly applies a transformation.

---

# 6. `fs` — filesystem and I/O

`fs` is the primary portable filesystem API.

### Files

```lucy
fs.write("hello.txt", "Hello Lucy")
fs.append("hello.txt", "\nSecond line")
println fs.read("hello.txt")
println fs.exists("hello.txt")
println fs.size("hello.txt")
```

Use `read_lines`/`write_lines` for line-oriented work.

### File management

```lucy
fs.copy("a.txt", "b.txt")
fs.move("b.txt", "archive/b.txt")
fs.touch("empty.txt")
fs.remove("empty.txt")
```

### Directories and paths

```lucy
fs.mkdir("build", true)
println fs.entries(".")
println fs.files(".")
println fs.dirs(".")
println fs.basename("src/lucy.cpp")
println fs.extension("src/lucy.cpp")
println fs.stem("src/lucy.cpp")
println fs.dirname("src/lucy.cpp")
```

`join`, `absolute`, and `expand` are the portable path helpers.

### Searching

`glob(pattern)` returns matching paths and `walk(path)` recursively visits a tree.

### IO objects

`fs.open(path, mode)` returns an `IO` object. It provides `print`, `write`, `read`, `ask`, `seek`, `pos`, `rewind`, `eof`, `fileno`, `close`, `pipe`, and `popen`.

Use `IO` for streaming/interactive operations where `fs.read`/`fs.write` are too high-level.

### JSON convenience

`fs.read_json(path)` and `fs.write_json(path, value)` are convenience wrappers for common file/data workflows.

---

# 7. `http` — HTTP and sockets

### Simple HTTP

```lucy
let response = http.get("https://example.com")
println response.status
println response.ok
println response.body
println response.headers
```

Every high-level HTTP request returns a map with exactly these primary fields:

| Field | Type | Meaning |
|---|---|---|
| `status` | Int | HTTP status code |
| `body` | String | response body |
| `headers` | Map | lower-case response-header names to values |
| `ok` | Bool | `true` for status 200–299 |

The module also exposes `post`, `put`, `patch`, `delete`, `head`, and the generic `request` function.

### HTTP client

```lucy
let client = http.client("https://example.com")
client.headers({"Accept": "application/json"})
client.timeout(10)
client.follow_redirects(true)
let response = client.get()
```

Timeout values for the high-level HTTP client are expressed in seconds by the public API.

`HTTPRequest` exposes an explicit request object with method, URL, body, headers, and options.

### Sockets

`Socket.connect`, `bind`, `listen`, `accept`, `recv`, `send`, and `close` expose lower-level network operations.

### DNS

`resolve(host)` / `reverse(address)` and the `Resolv` class provide forward/reverse resolution.

Network failures raise HTTP/process/native errors rather than silently returning a successful response.

---

# 8. `math` — numerical helpers

```lucy
println math.square(5)
println math.factorial(6)
println math.gcd(48, 18)
println math.lerp(0, 100, 0.25)
```

The module provides both a `Math` class and flat helper functions for the common operations: `square`, `cube`, `clamp`, `factorial`, `gcd`, `lcm`, `average`, `lerp`, and `sign`.

`Math.even` and `Math.odd` are available as class methods.

---

# 9. `random` — random values and sampling

```lucy
let n = random.int(1, 10)
let x = random.float()
let item = random.choice(["red", "green", "blue"])
```

`shuffle(items)` and `sample(items, count)` operate on arrays according to the library's current implementation.

Do not use these helpers as a cryptographic random source.

---

# 10. `repl` — interactive help

The REPL's help surface is implemented in Lucy itself:

```lucy
println repl.version()
println repl.commands()
println repl.topics()
help "array"
```

Public functions are `banner`, `version`, `prompt`, `commands`, `topics`, and `help`.

---

# 11. `result` — explicit success/error values

```lucy
let result = result.ok(42)
println result.success()
println result.unwrap(0)
```

The module provides `ok`, `err`, `success`, `unwrap`, and `message`.

Use it when a function wants to represent expected failure as data rather than immediately throwing a runtime exception.

---

# 12. `runtime` — reflection and callable helpers

`runtime` is the closest thing to a reflection/tooling module in the standard library.

```lucy
println runtime.inspect(value)
println runtime.methods(value)
println runtime.responds(value, "push")
println runtime.call(value, "push", [10])
```

The module also exposes convenience wrappers around the `Kernel` class and collection algorithms.

Important functions include:

- `printf`, `format`
- `catch`, `rescue`, `ensure`
- `methods`, `responds`, `call`
- `inspect`
- `variables`, `globals`
- `ancestors`, `superclass`

`runtime` is not a promise that every C++ runtime implementation detail is public. Internal helpers remain private.

---

# 13. `set` — set algebra

```lucy
let a = set.new([1, 2, 3])
let b = set.new([3, 4])

println a.union(b)
println a.intersection(b)
println a.difference(b)
```

`Set` provides membership, iteration, transformation, selection, and algebraic operations. Flat helper functions are also available.

---

# 14. `sqlite` — SQLite database access

SQLite is optional at build time.

```lucy
let db = sqlite.open("app.db")
db.execute("CREATE TABLE IF NOT EXISTS users (name TEXT)")
db.execute("INSERT INTO users (name) VALUES (?)", ["Lucy"])
let rows = db.query("SELECT name FROM users")
println rows
db.close()
```

`Database` provides `execute`, `query`, `prepare`, transaction operations, change/insert-id inspection, and `close`.

If Lucy was built without SQLite support, using this module reports:

```text
SQLiteError: Lucy was built without SQLite support
```

Enable it with CMake:

```text
-DLUCY_ENABLE_SQLITE=ON
```

---

# 15. `system` — operating-system and process APIs

`system` is the portable boundary for OS-specific operations.

```lucy
println system.platform()
println system.version()
println system.cwd()
println system.env("HOME")
println system.temp_dir()
```

Process helpers include `run`, `capture`, `success`, `output`, `spawn`, `wait`, `waitpid`, and `kill`.

Environment helpers include `env`, `setenv`, and `unsetenv`.

The module also exposes identity, signal, clock, login/user, and shell-quoting helpers. Availability of some identity/signal operations is platform-dependent.

For shell construction, prefer `shell_escape` / `shell_join` rather than manually concatenating untrusted arguments.

---

# 16. `text` — text processing and encoding helpers

```lucy
println text.match("^Lucy", "Lucy 2")
println text.replace_regex("[0-9]+", "N", "Lucy 2")
println text.base64_encode("hello")
println text.hex_encode("hello")
println text.url_encode("hello world")
```

The module provides regex operations, Base64, hexadecimal, URL encoding/decoding, and shell-token helpers.

`StringScanner` provides incremental scanning:

```lucy
let scanner = text.scanner("abc123")
println scanner.scan("[a-z]+")
println scanner.scan("[0-9]+")
```

---

# 17. `time` — time, date, and formatting

```lucy
let now = time.now()
println now.year()
println now.format("%Y-%m-%d")
println time.timestamp()
```

`Time`, `Date`, and `DateTime` provide construction, parsing, formatting, component access, arithmetic, and comparison.

Important unit rule:

- `Time.add`, `subtract`, `plus`, `minus`, and `add_ms`/`subtract_ms` use milliseconds where the signature says `milliseconds`.
- `Date.add`/`subtract` operate in days.
- `Date.shift_months` / `add_months` / `subtract_months` operate in months.
- `sleep(milliseconds)` uses milliseconds in the public `time` API.
- HTTP timeout APIs use seconds.

Read the signature inventory below when a function name has both a high-level and low-level form.

---

# 18. Global builtins and standard-library overlap

The following are global runtime builtins rather than module members:

```text
print println input exit
len str int float type typeof
range sum min max abs sqrt sin cos tan exp floor ceil log log10 pow
assert bin hex oct
```

There are also compatibility/runtime globals such as `read_file`, `write_file`, `exists`, `cwd`, `getenv`, `sleep`, and `millis`.

Prefer the module APIs in new application code when an equivalent exists. For example, prefer `fs.read(path)` over compatibility `read_file(path)` and `system.cwd()` over global `cwd()`.

---

# 19. Complete API inventory

The following inventory is source-derived from the shipped `stdlib/*.lucy` files. It is deliberately exhaustive and is the checklist used to prevent documentation drift.


- `class OptionParser`
- `OptionParser.initialize()`
- `OptionParser.new()`
- `OptionParser.banner(text)`
- `OptionParser.separator(text = "")`
- `OptionParser.version(text)`
- `OptionParser.program_name(text)`
- `OptionParser.on(name, description = "", default = nil)`
- `OptionParser.parse(arguments = ARGV)`
- `OptionParser.help()`
- `OptionParser.summarize()`
- `OptionParser.abort(message)`
- `class Logger`
- `Logger.initialize(output = nil, level = 0)`
- `Logger.new(output = nil, level = 0)`
- `Logger.level()`
- `Logger.set_level(value)`
- `Logger.level_set(value)`
- `Logger.add(level, message)`
- `Logger.log(level, message)`
- `Logger.debug(message)`
- `Logger.info(message)`
- `Logger.warn(message)`
- `Logger.error(message)`
- `Logger.fatal(message)`
- `Logger.debug?()`
- `Logger.info?()`
- `Logger.warn?()`
- `Logger.error?()`
- `Logger.fatal?()`
- `class Benchmark`
- `Benchmark.realtime(command)`
- `Benchmark.measure(command)`
- `class Timeout`
- `Timeout.timeout(seconds, callback)`
- `class ERB`
- `ERB.initialize(template)`
- `ERB.src()`
- `ERB.result(values = {})`
- `ERB.run(values = {})`
- `parser()`
- `logger(output = nil, level = 0)`
- `benchmark()`
- `timeout()`
- `template(source)`

### Complete inventory: `crypto`

- `class Digest`
- `Digest.digest(text)`
- `Digest.hexdigest(text)`
- `Digest.base64digest(text)`
- `Digest.file(path)`
- `class HMAC`
- `HMAC.digest(key, data)`
- `HMAC.hexdigest(key, data)`
- `class SSLContext`
- `SSLContext.initialize(options = {})`
- `class SSLSocket`
- `SSLSocket.initialize(socket = nil, context = nil)`
- `SSLSocket.connect(host, port)`
- `SSLSocket.close()`
- `class Certificate`
- `Certificate.initialize(data = "")`
- `class RSA`
- `RSA.initialize(pem = "")`
- `class Cipher`
- `Cipher.initialize(name = "")`
- `class X509`
- `X509.certificate(data = "")`
- `class PKey`
- `PKey.rsa(pem = "")`
- `class OpenSSL`
- `OpenSSL.hmac()`
- `OpenSSL.ssl_context(options = {})`
- `OpenSSL.ssl_socket(socket = nil, context = nil)`
- `OpenSSL.certificate(data = "")`
- `OpenSSL.rsa(pem = "")`
- `OpenSSL.x509()`
- `OpenSSL.pkey()`
- `OpenSSL.cipher(name = "")`
- `digest(text)`
- `hexdigest(text)`
- `hash(text)`
- `base64digest(text)`
- `file(path)`
- `hmac(key, data)`

### Complete inventory: `data`

- `pick(source, keys)`
- `omit(source, keys)`
- `merge(left, right)`
- `values(source, keys)`
- `zip(keys, values)`
- `class JSON`
- `JSON.parse(text)`
- `JSON.stringify(value)`
- `JSON.pretty(value)`
- `JSON.load(path)`
- `JSON.dump(path, value)`
- `parse(text)`
- `stringify(value)`
- `pretty(value)`
- `json_read(path)`
- `json_write(path, value)`
- `class YAML`
- `YAML.load(text)`
- `YAML.dump(value)`
- `YAML.load_file(path)`
- `yaml_load(text)`
- `yaml_dump(value)`
- `class CSV`
- `CSV.parse(text, delimiter = ",")`
- `CSV.stringify(rows, delimiter = ",")`
- `CSV.load(path, delimiter = ",")`
- `CSV.dump(path, rows, delimiter = ",")`
- `csv_parse(text, delimiter = ",")`
- `csv_stringify(rows, delimiter = ",")`

### Complete inventory: `flow`

- `pipe(value, steps)`
- `tap(value, action)`
- `branch(value, predicate, yes, no = nil)`
- `repeat(value, count, step)`

### Complete inventory: `fs`

- `read(path)`
- `write(path, content)`
- `append(path, content)`
- `read_lines(path)`
- `write_lines(path, lines)`
- `exists(path)`
- `size(path)`
- `remove(path)`
- `copy(source, destination)`
- `move(source, destination)`
- `touch(path)`
- `chmod(path, mode)`
- `chown(path, uid, gid)`
- `entries(path = ".")`
- `files(path = ".")`
- `dirs(path = ".")`
- `mkdir(path, parents = false)`
- `rmdir(path)`
- `empty(path = ".")`
- `glob(pattern)`
- `walk(path = ".")`
- `join(parts)`
- `absolute(path)`
- `expand(path)`
- `basename(path)`
- `dirname(path)`
- `extension(path)`
- `stem(path)`
- `link(source, destination)`
- `symlink(source, destination)`
- `open(path, mode = "r")`
- `console(prompt = "")`
- `read_json(path)`
- `write_json(path, value)`
- `class IO`
- `IO.initialize(path = nil, mode = "r")`
- `IO.print(values)`
- `IO.write(text)`
- `IO.read(prompt = "")`
- `IO.ask(prompt)`
- `IO.seek(offset, whence = 0)`
- `IO.pos()`
- `IO.rewind()`
- `IO.eof()`
- `IO.fileno()`
- `IO.close()`
- `IO.pipe()`
- `IO.popen(command)`

### Complete inventory: `http`

- `class HTTPRequest`
- `HTTPRequest.initialize(method, url, body = "", headers = nil, options = nil)`
- `HTTPRequest.execute()`
- `class HTTP`
- `HTTP.initialize(url = "", proxy = nil)`
- `HTTP.headers(value)`
- `HTTP.timeout(seconds)`
- `HTTP.connect_timeout(seconds)`
- `HTTP.proxy(proxy_url)`
- `HTTP.user_agent(value)`
- `HTTP.follow_redirects(value)`
- `HTTP.insecure(value = true)`
- `HTTP.get(path = "")`
- `HTTP.post(path = "", body = "")`
- `HTTP.put(path = "", body = "")`
- `HTTP.patch(path = "", body = "")`
- `HTTP.delete(path = "")`
- `HTTP.head(path = "")`
- `HTTP.request(method, path = "", body = "", headers = nil)`
- `request(method, url, body = "", headers = nil, options = nil)`
- `get(url, headers = nil, options = nil)`
- `post(url, body = "", headers = nil, options = nil)`
- `put(url, body = "", headers = nil, options = nil)`
- `patch(url, body = "", headers = nil, options = nil)`
- `delete(url, headers = nil, options = nil)`
- `head(url, headers = nil, options = nil)`
- `client(url = "", proxy = nil)`
- `class Socket`
- `Socket.initialize()`
- `Socket.connect(host, port)`
- `Socket.bind(host, port)`
- `Socket.listen(backlog = 16)`
- `Socket.accept()`
- `Socket.recv(size = 4096)`
- `Socket.send(data)`
- `Socket.close()`
- `class SocketAPI`
- `SocketAPI.new()`
- `class Resolv`
- `Resolv.getaddress(host)`
- `Resolv.getname(address)`
- `connect(host, port)`
- `bind(host, port)`
- `listen(socket, backlog = 16)`
- `accept(socket)`
- `recv(socket, size = 4096)`
- `send(socket, data)`
- `close(socket)`
- `resolve(host)`
- `reverse(address)`

### Complete inventory: `math`

- `class Math`
- `Math.square(x)`
- `Math.cube(x)`
- `Math.clamp(x, low, high)`
- `Math.even(x)`
- `Math.odd(x)`
- `Math.factorial(n)`
- `Math.gcd(a, b)`
- `Math.lcm(a, b)`
- `Math.average(values)`
- `Math.lerp(a, b, t)`
- `Math.sign(x)`
- `square(x)`
- `cube(x)`
- `clamp(x, low, high)`
- `factorial(n)`
- `gcd(a, b)`
- `lcm(a, b)`
- `average(values)`
- `lerp(a, b, t)`
- `sign(x)`

### Complete inventory: `random`

- `int(low, high)`
- `float()`
- `bool()`
- `choice(items)`
- `shuffle(items)`
- `sample(items, count)`

### Complete inventory: `repl`

- `banner()`
- `version()`
- `prompt(depth = 0)`
- `commands()`
- `topics()`
- `help(topic = nil)`

### Complete inventory: `result`

- `ok(value)`
- `err(message)`
- `success(result)`
- `unwrap(result, fallback = nil)`
- `message(result)`

### Complete inventory: `runtime`

- `class Kernel`
- `Kernel.printf(format_string, values = [])`
- `Kernel.p(value)`
- `Kernel.pp(value)`
- `Kernel.format(format_string, values = [])`
- `Kernel.sprintf(format_string, values = [])`
- `Kernel.catch(callback)`
- `Kernel.rescue(callback, handler)`
- `Kernel.ensure(callback, cleanup)`
- `Kernel.system(command)`
- `Kernel.spawn(command)`
- `Kernel.trap(signal_number, callback)`
- `Kernel.global_variables()`
- `Kernel.local_variables()`
- `Kernel.methods(value)`
- `Kernel.respond_to?(value, name)`
- `Kernel.send(value, name, args = [])`
- `Kernel.inspect(value)`
- `Kernel.to_a(value)`
- `Kernel.to_h(value)`
- `Kernel.to_sym(value)`
- `Kernel.ancestors(value)`
- `Kernel.superclass(value)`
- `Kernel.singleton_class(value)`
- `Kernel.proc(callback)`
- `Kernel.lambda(callback)`
- `class Collections`
- `Collections.first(items)`
- `Collections.last(items)`
- `Collections.reverse(items)`
- `Collections.contains(items, value)`
- `Collections.count(items, value)`
- `Collections.index(items, value)`
- `Collections.compact(items)`
- `Collections.unique(items)`
- `Collections.flatten(items)`
- `Collections.sum(items)`
- `Collections.min(items)`
- `Collections.max(items)`
- `printf(format_string, values = [])`
- `format(format_string, values = [])`
- `catch(callback)`
- `rescue(callback, handler)`
- `ensure(callback, cleanup)`
- `methods(value)`
- `responds(value, name)`
- `call(value, name, args = [])`
- `inspect(value)`
- `variables()`
- `globals()`
- `ancestors(value)`
- `superclass(value)`

### Complete inventory: `set`

- `class Set`
- `Set.initialize(values = [])`
- `Set.new(values = [])`
- `Set.add(value)`
- `Set.delete(value)`
- `Set.include?(value)`
- `Set.member?(value)`
- `Set.each(callback)`
- `Set.size()`
- `Set.length()`
- `Set.empty?()`
- `Set.clear()`
- `Set.map(callback)`
- `Set.select(callback)`
- `Set.reject(callback)`
- `Set.merge(other)`
- `Set.subset(other)`
- `Set.superset(other)`
- `Set.intersect(other)`
- `Set.union(other)`
- `Set.intersection(other)`
- `Set.difference(other)`
- `Set.subset?(other)`
- `Set.superset?(other)`
- `Set.intersect?(other)`
- `Set.symmetric_difference(other)`
- `new(values = [])`
- `from_values(values)`
- `add(target, value)`
- `delete(target, value)`
- `include?(target, value)`
- `union(left, right)`
- `intersection(left, right)`
- `difference(left, right)`
- `symmetric_difference(left, right)`
- `subset?(left, right)`
- `superset?(left, right)`
- `intersect?(left, right)`

### Complete inventory: `sqlite`

- `class Database`
- `Database.initialize(path)`
- `Database.execute(sql, params = [])`
- `Database.query(sql, params = [])`
- `Database.prepare(sql)`
- `Database.begin()`
- `Database.commit()`
- `Database.rollback()`
- `Database.changes()`
- `Database.last_insert_id()`
- `Database.close()`
- `class Statement`
- `Statement.initialize(handle, sql)`
- `Statement.bind(params = [])`
- `Statement.execute()`
- `Statement.query()`
- `Statement.close()`
- `open(path)`

### Complete inventory: `system`

- `platform()`
- `version()`
- `argv()`
- `cwd()`
- `env(name)`
- `setenv(name, value)`
- `unsetenv(name)`
- `home()`
- `temp_dir()`
- `command_exists(command)`
- `pid()`
- `ppid()`
- `run(command)`
- `capture(command)`
- `success(command)`
- `output(command)`
- `spawn(command)`
- `wait(pid)`
- `waitpid(pid)`
- `kill(signal_number, pid)`
- `uid()`
- `gid()`
- `euid()`
- `egid()`
- `groups()`
- `clock_gettime(clock = "monotonic")`
- `trap(signal_number, callback)`
- `signals()`
- `signal_name(signal_number)`
- `login()`
- `user(name)`
- `user_id(uid_value)`
- `shell_split(text)`
- `shell_escape(text)`
- `shell_join(items)`

### Complete inventory: `text`

- `match(pattern, text)`
- `search(pattern, text)`
- `find_all(pattern, text)`
- `replace_regex(pattern, replacement, text)`
- `base64_encode(text)`
- `base64_decode(text)`
- `hex_encode(text)`
- `hex_decode(text)`
- `url_encode(text)`
- `url_decode(text)`
- `scanner(text)`
- `shell_split(text)`
- `shell_escape(text)`
- `shell_join(items)`
- `class StringScanner`
- `StringScanner.initialize(text)`
- `StringScanner.scan(pattern)`
- `StringScanner.scan_until(pattern)`
- `StringScanner.skip(pattern)`
- `StringScanner.skip_until(pattern)`
- `StringScanner.check(pattern)`
- `StringScanner.check_until(pattern)`
- `StringScanner.match?()`
- `StringScanner.matched()`
- `StringScanner.matched_size()`
- `StringScanner.pre_match()`
- `StringScanner.post_match()`

### Complete inventory: `time`

- `class Time`
- `Time.initialize(timestamp = nil)`
- `Time.now()`
- `Time.today()`
- `Time.strptime(text, format)`
- `Time.format(format_string)`
- `Time.parts()`
- `Time.year()`
- `Time.month()`
- `Time.day()`
- `Time.hour()`
- `Time.minute()`
- `Time.second()`
- `Time.add(milliseconds)`
- `Time.subtract(milliseconds)`
- `Time.plus(milliseconds)`
- `Time.minus(milliseconds)`
- `Time.compare(other)`
- `Time.succ()`
- `Time.add_ms(milliseconds)`
- `Time.subtract_ms(milliseconds)`
- `class Date`
- `Date.initialize(timestamp = nil)`
- `Date.parse(text)`
- `Date.strptime(text, format)`
- `Date.format(format_string = "%Y-%m-%d")`
- `Date.add(days)`
- `Date.subtract(days)`
- `Date.next_day()`
- `Date.prev_day()`
- `Date.succ()`
- `Date.shift_months(months)`
- `Date.add_months(months)`
- `Date.subtract_months(months)`
- `Date.compare(other)`
- `class DateTime`
- `DateTime.initialize(timestamp)`
- `DateTime.timestamp()`
- `DateTime.format(pattern)`
- `DateTime.parts()`
- `DateTime.year()`
- `DateTime.month()`
- `DateTime.day()`
- `DateTime.hour()`
- `DateTime.minute()`
- `DateTime.second()`
- `DateTime.weekday()`
- `DateTime.iso()`
- `class DateTimeModule`
- `DateTimeModule.now()`
- `DateTimeModule.from_timestamp(milliseconds)`
- `DateTimeModule.format(milliseconds, pattern)`
- `now()`
- `today()`
- `timestamp()`
- `parse(text)`
- `date(text)`
- `format(value, pattern = "%Y-%m-%d %H:%M:%S")`
- `_time_sleep(milliseconds)`
- `sleep(milliseconds)`
