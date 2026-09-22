# Lucy 1.0.1 — Standard Library Reference

> **Status:** Reference for Lucy 1.0.1  
> **API style:** flat module API  
> **Audience:** Lucy users, library authors, and contributors

Lucy’s standard library is organized into small, focused modules. A module is imported once and its public API is exposed directly from that module.

```lucy
import fs
import text
import http

content = fs.read "notes.txt"
name = text.trim "  Lucy  "
response = http.get "https://example.com"
```

## 1. Design rules

### Flat APIs

Lucy deliberately avoids deeply nested public APIs.

```lucy
# Preferred
fs.read "notes.txt"
http.get url
data.parse json_text

# Avoid
fs.file.read "notes.txt"
http.socket.connect host, port
data.json.parse json_text
```

The module name identifies the capability; the function name identifies the operation.

### Optional parentheses

Lucy functions may be called with or without parentheses.

```lucy
fs.read "notes.txt"
fs.read("notes.txt")
```

### Multiline calls

Newlines inside argument lists are insignificant. This applies consistently to function calls and other delimited expressions.

```lucy
response = http.request(
    "POST",
    url,
    body,
    headers,
    options
)
```

### Return values

Unless otherwise stated, functions return a Lucy value directly. Operations that fail normally raise a runtime/library error rather than returning a special error object.

---

# 2. Module index

| Module | Primary purpose |
|---|---|
| `app` | CLI parsing, logging, templates, benchmarking, timeout helpers |
| `crypto` | Digests, HMAC, cryptographic/OpenSSL wrappers |
| `data` | Collection transforms and JSON/YAML/CSV serialization |
| `flow` | Value-oriented pipelines and flow control |
| `fs` | Files, directories, paths, links, and IO |
| `http` | HTTP, reusable clients, TCP sockets, DNS |
| `math` | Mathematical operations and numeric helpers |
| `random` | Random values, choices, shuffling, sampling |
| `repl` | REPL commands, help, prompt customization |
| `result` | Explicit success/error result values |
| `runtime` | Reflection, invocation, inspection, formatting |
| `set` | Unique collections and set operations |
| `sqlite` | SQLite databases |
| `system` | OS, processes, signals, shell utilities |
| `text` | Strings, regex, encodings, shell words |
| `time` | Time, date, datetime, sleeping |

---

# 3. `app`

Application-level utilities.

```lucy
import app
```

## 3.1 CLI parser

```lucy
parser = app.parser()

parser.banner "Lucy application"
parser.version "1.0.1"
parser.program_name "myapp"

parser.on "--verbose", "Enable verbose output", false

options = parser.parse
```

### `app.parser()`

Creates an option parser.

**Returns:** `OptionParser`

### `parser.banner(text)`

Sets the CLI help banner.

### `parser.separator(text = "")`

Adds a separator to the help output.

### `parser.version(text)`

Sets the displayed version.

### `parser.program_name(text)`

Sets the program name.

### `parser.on(name, description = "", default = nil)`

Registers an option.

### `parser.parse(arguments = ARGV)`

Parses an argument list without replacing the original argument list.

### `parser.parse!(arguments = ARGV)`

Parsing variant intended for destructive/consuming CLI workflows.

### `parser.help()`

Returns or prints the generated help representation.

### `parser.summarize()`

Returns a compact summary of registered options.

### `parser.abort(message)`

Reports a CLI error and aborts the application.

## 3.2 Logger

```lucy
logger = app.logger()
logger.info "Server started"
logger.warn "Configuration is missing"
logger.error "Request failed"
```

Levels:

- `0` — debug
- `1` — info
- `2` — warn
- `3` — error
- `4` — fatal

### Logger API

```text
logger.level()
logger.set_level(value)
logger.level_set(value)

logger.add(level, message)
logger.log(level, message)

logger.debug(message)
logger.info(message)
logger.warn(message)
logger.error(message)
logger.fatal(message)

logger.debug?()
logger.info?()
logger.warn?()
logger.error?()
logger.fatal?()
```

The predicate methods indicate whether a message at that level is currently enabled.

## 3.3 Benchmark

```lucy
bench = app.benchmark()

bench.realtime {
    # operation
}

bench.measure {
    # operation
}
```

- `realtime(command)` measures elapsed real time.
- `measure(command)` performs the module's benchmark measurement workflow.

## 3.4 Timeout

```lucy
app.timeout.timeout 5, {
    # operation
}
```

`timeout(seconds, callback)` executes a callback with a time limit.

## 3.5 Templates

```lucy
tpl = app.template("Hello {{name}}")
result = tpl.result({name: "Lucy"})
```

Template object:

```text
template(source)
template.src()
template.result(values = {})
template.run(values = {})
```

---

# 4. `crypto`

Cryptographic helpers and OpenSSL-oriented wrappers.

```lucy
import crypto
```

## 4.1 Digest

```lucy
crypto.digest "hello"
crypto.hexdigest "hello"
crypto.base64digest "hello"
crypto.file "notes.txt"
```

The digest API provides hashing/digest operations over strings and files.

### Convenience functions

```text
crypto.digest(text)
crypto.hexdigest(text)
crypto.hash(text)
crypto.base64digest(text)
crypto.file(path)
```

## 4.2 HMAC

```lucy
crypto.hmac "secret", "message"
```

The HMAC object supports:

```text
hmac.digest(key, data)
hmac.hexdigest(key, data)
```

## 4.3 OpenSSL wrappers

The module also exposes objects for lower-level cryptographic work:

```text
crypto.openssl.hmac()
crypto.openssl.ssl_context(options = {})
crypto.openssl.ssl_socket(socket = nil, context = nil)
crypto.openssl.certificate(data = "")
crypto.openssl.rsa(pem = "")
crypto.openssl.x509()
crypto.openssl.pkey()
crypto.openssl.cipher(name = "")
```

These wrappers are intended for integrations that need certificate, key, cipher, or TLS objects rather than simple hashing.

---

# 5. `data`

Collection transformations and serialization.

```lucy
import data
```

## 5.1 Collection operations

```lucy
data.map [1, 2, 3], lambda x => x * 2
data.filter [1, 2, 3, 4], lambda x => x % 2 == 0
data.reduce [1, 2, 3], lambda a, b => a + b, 0

data.min [3, 1, 2]
data.max [3, 1, 2]
data.unique [1, 1, 2]
data.flatten [[1, 2], [3, 4]]
```

### `map(source, callback)`

Transforms each item.

**Returns:** transformed collection.

### `filter(source, callback)`

Keeps items for which the callback evaluates truthy.

### `reduce(source, callback, initial = nil)`

Reduces a collection to one value.

### `min(source)` / `max(source)`

Return the minimum/maximum item.

### `unique(source)`

Removes duplicate values.

### `flatten(source)`

Flattens nested collection values.

## 5.2 Map/object helpers

```lucy
data.pick user, ["name", "version"]
data.omit user, ["password"]
data.merge defaults, config
data.values user, ["name", "version"]
data.zip ["name", "version"], ["Lucy", "1.0.1"]
```

- `pick(source, keys)` — selects requested keys.
- `omit(source, keys)` — removes requested keys.
- `merge(left, right)` — combines map/object values.
- `values(source, keys)` — extracts values for keys.
- `zip(keys, values)` — constructs a map from parallel key/value collections.

## 5.3 JSON

```lucy
value = data.parse "{\"name\":\"Lucy\"}"

data.stringify value
data.pretty value

data.json_read "config.json"
data.json_write "config.json", value
```

Object API:

```text
JSON.parse(text)
JSON.stringify(value)
JSON.pretty_generate(value)
JSON.fast_generate(value)
JSON.load(path)
JSON.dump(path, value)
```

## 5.4 YAML

```lucy
config = data.yaml_load "name: Lucy"
output = data.yaml_dump config
```

Supported object operations include:

```text
YAML.load(text)
YAML.safe_load(text)
YAML.dump(value)
YAML.load_file(path)
```

## 5.5 CSV

```lucy
rows = data.csv_parse "name,version\nLucy,1.0.1"
csv = data.csv_stringify rows
```

Object API:

```text
CSV.parse(text, delimiter = ",")
CSV.stringify(rows, delimiter = ",")
CSV.load(path, delimiter = ",")
CSV.dump(path, rows, delimiter = ",")
```

---

# 6. `flow`

Small functional helpers for composing operations.

```lucy
import flow
```

## `flow.pipe(value, steps)`

Passes a value through a sequence of operations.

```lucy
result = flow.pipe(
    "  lucy  ",
    [
        lambda x => text.trim x,
        lambda x => text.upper x
    ]
)
```

## `flow.tap(value, action)`

Runs an action for side effects and keeps the value flowing.

## `flow.branch(value, predicate, yes, no = nil)`

Selects one of two operations based on a predicate.

## `flow.repeat(value, count, step)`

Applies a step repeatedly.

---

# 7. `fs`

Filesystem and IO operations.

```lucy
import fs
```

## 7.1 Files

```text
fs.read(path)
fs.write(path, content)
fs.append(path, content)
fs.read_lines(path)
fs.write_lines(path, lines)

fs.exists(path)
fs.size(path)
fs.remove(path)

fs.copy(source, destination)
fs.move(source, destination)
fs.touch(path)

fs.chmod(path, mode)
fs.chown(path, uid, gid)

fs.read_json(path)
fs.write_json(path, value)
fs.append_line(path, line)
```

### Example

```lucy
fs.write "hello.txt", "Hello Lucy\n"
fs.append "hello.txt", "Second line\n"

lines = fs.read_lines "hello.txt"
```

## 7.2 Directories

```text
fs.entries(path = ".")
fs.files(path = ".")
fs.dirs(path = ".")
fs.mkdir(path, parents = false)
fs.rmdir(path)
fs.empty(path = ".")
fs.glob(pattern)
fs.walk(path = ".")
```

Example:

```lucy
fs.mkdir "build/cache", true

for file in fs.files "src"
    print file
end
```

## 7.3 Paths

```text
fs.join(parts)
fs.absolute(path)
fs.expand(path)
fs.basename(path)
fs.dirname(path)
fs.extension(path)
fs.stem(path)
```

Example:

```lucy
path = fs.join ["src", "main.cpp"]
name = fs.basename path
ext = fs.extension path
```

## 7.4 Links and permissions

```text
fs.link(source, destination)
fs.symlink(source, destination)
fs.chmod(path, mode)
fs.chown(path, uid, gid)
```

## 7.5 File handles

```lucy
file = fs.open "notes.txt", "r"

content = file.read()
file.close()
```

The `IO` object also supports:

```text
read(prompt = "")
write(text)
print(values)
seek(offset, whence = 0)
pos()
rewind()
eof()
fileno()
close()
pipe()
popen(command)
```

---

# 8. `http`

Lucy’s networking module provides one-shot HTTP requests, reusable HTTP clients, TCP sockets, and DNS.

```lucy
import http
```

## 8.1 One-shot HTTP

```lucy
response = http.get "https://example.com"

response = http.post(
    "https://example.com/api",
    "{\"name\":\"Lucy\"}",
    {
        "content-type": "application/json"
    }
)
```

Available methods:

```text
http.request(method, url, body = "", headers = nil, options = nil)
http.get(url, headers = nil, options = nil)
http.post(url, body = "", headers = nil, options = nil)
http.put(url, body = "", headers = nil, options = nil)
http.patch(url, body = "", headers = nil, options = nil)
http.delete(url, headers = nil, options = nil)
http.head(url, headers = nil, options = nil)
```

## 8.2 Response object

An HTTP response contains:

| Field | Meaning |
|---|---|
| `status` | Numeric HTTP status |
| `ok` | `true` for 2xx responses |
| `body` | Response body |
| `headers` | Response headers |

Example:

```lucy
response = http.get "https://example.com"

print response.status
print response.ok
print response.body
print response.headers["content-type"]
```

## 8.3 Request options

Options:

| Key | Type | Default | Description |
|---|---|---|---|
| `timeout` | number | `30` | Maximum request time |
| `connect_timeout` | number | none | Connection timeout |
| `proxy` | string | none | Proxy URL |
| `user_agent` | string | curl default | User-Agent |
| `follow_redirects` | bool | `true` | Follow redirects |
| `insecure` | bool | `false` | Disable TLS certificate verification |

Example:

```lucy
options = {
    "timeout": 10,
    "connect_timeout": 3,
    "user_agent": "Lucy/1.0.1",
    "follow_redirects": true
}

response = http.get url, nil, options
```

`insecure` disables TLS certificate verification and should only be used intentionally.

## 8.4 Reusable HTTP client

```lucy
client = http.client "https://example.com"

client.headers {
    "accept": "application/json"
}

client.timeout 10
client.connect_timeout 3
client.user_agent "Lucy/1.0.1"

response = client.get "/"
```

Client methods:

```text
http.client(url = "", proxy = nil)

client.headers(value)
client.timeout(seconds)
client.connect_timeout(seconds)
client.proxy(proxy_url)
client.user_agent(value)
client.follow_redirects(value)
client.insecure(value = true)

client.get(path = "")
client.post(path = "", body = "")
client.put(path = "", body = "")
client.patch(path = "", body = "")
client.delete(path = "")
client.head(path = "")
client.request(method, path = "", body = "", headers = nil)
```

Each configuration method returns the client, so calls can be chained.

## 8.5 Request object

```lucy
request = http.HTTPRequest.new(
    "GET",
    "https://example.com"
)

response = request.execute()
```

Constructor:

```text
HTTPRequest.new(method, url, body = "", headers = nil, options = nil)
```

## 8.6 TCP sockets

```lucy
socket = http.connect "127.0.0.1", 8080

http.send socket, "hello"
data = http.recv socket, 4096

http.close socket
```

Low-level socket object:

```text
Socket.new()
socket.connect(host, port)
socket.bind(host, port)
socket.listen(backlog = 16)
socket.accept()
socket.recv(size = 4096)
socket.send(data)
socket.close()
```

Server example:

```lucy
server = http.bind "127.0.0.1", 8080
http.listen server, 16

client = http.accept server
request = http.recv client, 4096

http.send client, "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK"

http.close client
http.close server
```

## 8.7 DNS

```lucy
address = http.resolve "example.com"
name = http.reverse address
```

API:

```text
http.resolve(host)
http.reverse(address)

http.resolv.getaddress(host)
http.resolv.getname(address)
```

The native DNS helpers resolve IPv4 addresses.

## 8.8 HTTP runtime dependency

The high-level HTTP client uses the host `curl` executable. If `curl` is unavailable on `PATH`, an HTTP request fails instead of silently switching to another transport.

---

# 9. `math`

Mathematical functions and numeric helpers.

```lucy
import math
```

```text
math.square(x)
math.cube(x)
math.clamp(x, low, high)
math.even(x)
math.odd(x)

math.abs(x)
math.sqrt(x)
math.pow(x, y)

math.sin(x)
math.cos(x)
math.tan(x)

math.floor(x)
math.ceil(x)
math.log(x)

math.min(a, b)
math.max(a, b)

math.factorial(n)
math.gcd(a, b)
math.lcm(a, b)
math.average(values)
```

Examples:

```lucy
math.square 12
math.clamp temperature, 0, 100
math.gcd 48, 18
math.average [10, 20, 30]
```

---

# 10. `random`

Random-value utilities.

```lucy
import random
```

The module supports creation of a random generator and common random operations.

Typical usage:

```lucy
rng = random.new 42
```

The generator API includes random integer/value generation, selection, shuffling, and sampling as provided by the current implementation.

---

# 11. `repl`

REPL-facing helpers.

```lucy
import repl
```

The REPL library keeps help and interactive commands outside the language core.

Typical API areas include:

- help and topic lookup
- REPL version information
- prompt customization
- command registration
- command listing

The language-level convenience form remains:

```lucy
help print
```

The implementation of the help system lives in the `repl` library rather than in the core runtime.

---

# 12. `result`

Explicit success/error values.

```lucy
import result
```

The module is intended for APIs where failure is expected and should be represented as data instead of immediately raising an exception.

Core operations:

```text
result.ok(value)
result.err(error)
```

Typical pattern:

```lucy
value = result.ok 42
failure = result.err "invalid input"
```

Use `result` when callers should explicitly inspect success versus failure.

---

# 13. `runtime`

Runtime inspection, reflection, invocation, and formatting.

```lucy
import runtime
```

Core operations include:

```text
runtime.type(value)
runtime.inspect(value)

runtime.methods(value)
runtime.responds(value, name)

runtime.call(value, name, args = [])

runtime.variables()
runtime.globals()

runtime.ancestors(value)
runtime.superclass(value)
```

Formatting helpers:

```text
runtime.printf(format_string, values = [])
runtime.format(format_string, values = [])
```

Exception helpers:

```text
runtime.catch(callback)
runtime.rescue(callback, handler)
runtime.ensure(callback, cleanup)
```

Example:

```lucy
print runtime.type value
print runtime.inspect value

if runtime.responds object, "start"
    runtime.call object, "start", []
end
```

---

# 14. `set`

Unique-value collections and set algebra.

```lucy
import set
```

## Construction

```lucy
numbers = set.new [1, 2, 2, 3]
```

Set methods:

```text
add(value)
delete(value)
include?(value)
member?(value)

each(callback)

size()
length()
empty?()
clear()

map(callback)
select(callback)
reject(callback)
```

## Set algebra

```text
union(other)
intersection(other)
difference(other)
symmetric_difference(other)

subset?(other)
superset?(other)
intersect?(other)
```

Example:

```lucy
a = set.new [1, 2, 3]
b = set.new [3, 4, 5]

both = a.intersection b
all = a.union b
only_a = a.difference b
```

Flat helpers are also available:

```text
set.union(left, right)
set.intersection(left, right)
set.difference(left, right)
set.symmetric_difference(left, right)
set.subset?(left, right)
set.superset?(left, right)
set.intersect?(left, right)
```

---

# 15. `sqlite`

SQLite database access.

```lucy
import sqlite
```

## Open

```lucy
db = sqlite.open "app.db"
```

## Execute

```lucy
sqlite.execute db, "CREATE TABLE users (id INTEGER, name TEXT)"
```

## Query

```lucy
rows = sqlite.query db, "SELECT * FROM users"
```

## Insert

```lucy
sqlite.insert(
    db,
    "users",
    ["id", "name"],
    [1, "Nima"]
)
```

## Rows

```lucy
rows = sqlite.rows db, "SELECT id, name FROM users"
```

## Database object

```text
Database.new(path)
db.execute(sql)
db.query(sql)
db.create_table(name, columns)
db.insert(table, columns, values)
db.rows(sql)
```

The module provides a deliberately small SQLite API; SQL remains SQL rather than being hidden behind a large ORM layer.

---

# 16. `system`

Operating-system, process, shell, signal, and user utilities.

```lucy
import system
```

## 16.1 OS information

```text
system.platform()
system.version()
system.cwd()
system.env(name)
system.setenv(name, value)
system.unsetenv(name)

system.pid()
system.ppid()

system.home()
system.temp_dir()

system.cpu_count()
system.command_exists(command)
system.which(command)
```

## 16.2 Arguments

```lucy
args = system.argv()
```

`system.argv()` returns the process argument list.

## 16.3 Processes and commands

```text
system.run(command)
system.capture(command)
system.success(command)
system.output(command)

system.spawn(command)
system.wait(pid)
system.waitpid(pid)

system.kill(signal, pid)
```

Differences:

- `run` — execute a command.
- `capture` — execute while capturing process output.
- `success` — test command success.
- `output` — obtain command output.
- `spawn` — start a process without waiting.
- `wait` / `waitpid` — wait for a child process.
- `kill` — send a signal to a process.

## 16.4 Process identity

```text
system.uid()
system.gid()
system.euid()
system.egid()
system.groups()
system.clock_gettime(clock = "monotonic")
```

## 16.5 Signals

```lucy
system.trap signal_number, callback
signals = system.signals()
name = system.signal_name signal_number
```

## 16.6 User information

```text
system.login()
system.user(name)
system.user_id(uid)
```

## 16.7 Shell words

```lucy
parts = system.shell_split command
safe = system.shell_escape value
command = system.shell_join parts
```

Available operations:

```text
shell_split(text)
shell_escape(text)
shell_join(items)
```

These helpers are useful when constructing command arguments while preserving shell-word boundaries.

---

# 17. `text`

Text processing, regular expressions, encodings, and shell words.

```lucy
import text
```

## 17.1 Basic string operations

```text
text.capitalize(value)
text.upper(value)
text.lower(value)
text.trim(value)
text.reverse(value)
text.repeat(value, count)

text.split(value, separator)
text.join(values, separator = "")

text.replace(value, old_value, new_value)
text.contains(value, needle)
text.starts_with(value, needle)
text.ends_with(value, needle)

text.length(value)
text.to_int(value)
text.to_float(value)
```

Example:

```lucy
name = text.trim "  Nima  "
name = text.capitalize name
```

## 17.2 Regex

```text
text.match(pattern, value)
text.search(pattern, value)
text.matches(pattern, value)
text.replace_regex(pattern, replacement, value)
```

Example:

```lucy
if text.match "^[a-z]+$", "lucy"
    print "valid"
end
```

## 17.3 Encodings

```text
text.base64_encode(value)
text.base64_decode(value)

text.hex_encode(value)
text.hex_decode(value)

text.url_encode(value)
text.url_decode(value)
```

## 17.4 Scanner

`StringScanner` supports incremental pattern matching:

```text
StringScanner.new(text)

scanner.scan(pattern)
scanner.scan_until(pattern)

scanner.skip(pattern)
scanner.skip_until(pattern)

scanner.check(pattern)
scanner.check_until(pattern)

scanner.match?()
scanner.matched()
scanner.matched_size()
scanner.pre_match()
scanner.post_match()
```

## 17.5 Shell words

```text
text.shell_split(value)
text.shell_escape(value)
text.shell_join(value)
```

---

# 18. `time`

Time, date, datetime, formatting, arithmetic, and sleeping.

```lucy
import time
```

## 18.1 Current time

```lucy
now = time.now
today = time.today
stamp = time.timestamp
```

## 18.2 Time object

```text
Time.new(timestamp = nil)
time.now()
time.today()

time.strptime(text, format)
time.format(format_string)

time.parts()

time.year()
time.month()
time.day()
time.hour()
time.minute()
time.second()

time.add(milliseconds)
time.subtract(milliseconds)
time.plus(milliseconds)
time.minus(milliseconds)

time.compare(other)
time.succ()

time.add_ms(milliseconds)
time.subtract_ms(milliseconds)
```

Example:

```lucy
now = time.now
print now.year
print now.month
print now.day
```

Time arithmetic uses milliseconds for the low-level `Time` object.

## 18.3 Date

```text
Date.new(timestamp = nil)
Date.parse(text)
Date.strptime(text, format)

date.format(format_string = "%Y-%m-%d")

date.add(days)
date.subtract(days)

date.next_day()
date.prev_day()
date.succ()

date.shift_months(months)
date.add_months(months)
date.subtract_months(months)

date.compare(other)
```

## 18.4 DateTime

```text
DateTime.new(timestamp)

datetime.timestamp()
datetime.format(pattern)
datetime.parts()

datetime.year()
datetime.month()
datetime.day()
datetime.hour()
datetime.minute()
datetime.second()

datetime.weekday()
datetime.iso()
```

Module helpers:

```text
time.now()
time.today()
time.timestamp()
time.parse(text)
time.date(text)
time.format(value, pattern = "%Y-%m-%d %H:%M:%S")
time.sleep(milliseconds)
```

Example:

```lucy
started = time.now

# milliseconds
time.sleep 500

finished = time.now
```

---

# 19. Common usage patterns

## Importing several modules

```lucy
import fs
import text
import data

content = fs.read "config.json"
config = data.parse content
name = text.trim config["name"]
```

## HTTP + JSON

```lucy
import http
import data

response = http.get "https://example.com/api"

if response.ok
    payload = data.parse response.body
    print payload
else
    print response.status
end
```

## File + data

```lucy
import fs
import data

config = data.parse fs.read "config.json"

config["version"] = "1.0.1"

fs.write "config.json", data.pretty config
```

## Process + filesystem

```lucy
import system
import fs

if system.command_exists "git"
    output = system.capture "git --version"
    fs.write "git-version.txt", output
end
```

---

# 20. Native/private functions

The standard library also contains internal helpers whose names begin with `__` or `_`.

Examples include:

```text
__http_request
__socket_new
__socket_connect
__socket_recv
__socket_send
__socket_close
__resolv_getaddress
__resolv_getname
__regex_match
__regex_search
__encoding_base64_encode
```

These are implementation details used by the public standard-library API. **They are not stable public API and should not be called directly by normal Lucy programs.**

The public API should remain flat and readable:

```lucy
http.get url
text.match pattern, value
data.parse json
fs.read path
```

rather than calling internal runtime primitives.

---

# 21. API stability

For Lucy 1.0.1:

- Public module functions are the preferred API.
- Internal `__*` and `_name` helpers are implementation details.
- New functionality should normally be exposed through the existing flat module instead of introducing another namespace level.
- Documentation should describe behavior that exists in the implementation, not merely planned functionality.
- Examples should prefer the no-parentheses Lucy syntax where it improves readability, while parentheses remain valid.
- Multiline argument lists are valid and should be treated exactly like their single-line equivalents.

---

# 22. Quick reference

```text
app       CLI / logging / templates / benchmark / timeout
crypto    digest / HMAC / TLS / certificates / keys
data      map / filter / reduce / JSON / YAML / CSV
flow      pipe / tap / branch / repeat
fs        files / directories / paths / IO / links
http      HTTP / client / TCP / DNS
math      numeric and mathematical operations
random    random generation / choice / shuffle / sample
repl      REPL help / commands / prompt
result    explicit ok / err values
runtime   reflection / invocation / inspection
set       unique collections / set algebra
sqlite    SQLite databases
system    OS / environment / process / signal / shell
text      strings / regex / encoding / scanner
time      time / date / datetime / sleep
```

---

## Version

This document describes the **Lucy 1.0.1** standard library layout and public API.
