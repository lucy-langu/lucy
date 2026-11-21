# Lucy 1.0.1 Standard Library

Lucy 1.0.1 uses a **flat module API**. There is no language-level `namespace` feature and public calls are intentionally one level deep:

```lucy
import fs
fs.read "notes.txt"
fs.write "notes.txt", "Hello Lucy\n"

import http
response = http.get "https://example.com"

import data
value = data.parse "{\"name\":\"Lucy\"}"
```

Avoid APIs such as `fs.file.read`, `http.socket.connect`, `data.json.parse`, or `system.process.run`. Related capabilities belong to one module and are exposed directly by that module.

## Modules

| Module | Purpose |
|---|---|
| `app` | CLI option parsing, logging, templates, benchmarking and timeout helpers |
| `crypto` | SHA-256, HMAC and digest helpers |
| `data` | Collection transforms and JSON/YAML/CSV serialization |
| `flow` | Value-first pipelines and functional flow helpers |
| `fs` | Files, directories, paths and IO |
| `http` | HTTP requests, TCP sockets and DNS |
| `math` | Mathematical functions and constants |
| `random` | Random values, choices, shuffling and sampling |
| `repl` | Editable REPL UI/help library |
| `result` | Explicit success/error values |
| `runtime` | Reflection, invocation and runtime inspection |
| `set` | Unique collections and set operations |
| `sqlite` | SQLite databases |
| `system` | OS information, environment, processes and signals |
| `text` | Text, regex, encoding and shell-word operations |
| `time` | Time/date values and temporal operations |

## `fs`

```lucy
import fs

fs.read path
fs.write path, content
fs.append path, content
fs.read_lines path
fs.write_lines path, lines
fs.exists path
fs.size path
fs.remove path
fs.copy source, destination
fs.move source, destination
fs.touch path
fs.chmod path, mode
fs.chown path, uid, gid

fs.entries path
fs.files path
fs.dirs path
fs.mkdir path, true
fs.rmdir path
fs.empty path
fs.glob pattern
fs.walk path

fs.join ["src", "main.cpp"]
fs.absolute path
fs.expand path
fs.basename path
fs.dirname path
fs.extension path
fs.stem path
fs.link source, destination
fs.symlink source, destination

handle = fs.open path, "r"
handle.read()
handle.close()
```

## `http`

```lucy
import http

response = http.get "https://example.com"
response = http.post "https://example.com/api", "hello"
response = http.put url, body
response = http.patch url, body
response = http.delete url
response = http.head url
response = http.request "OPTIONS", url

client = http.client "https://example.com"
client.get "/"

socket = http.connect "127.0.0.1", 8080
http.send socket, "hello"
data = http.recv socket, 4096
http.close socket

server = http.bind "127.0.0.1", 8080
http.listen server, 16
client = http.accept server
http.close client
http.close server

addresses = http.resolve "localhost"
name = http.reverse "127.0.0.1"
```

## `data`

```lucy
import data

mapped = data.map [1, 2, 3], lambda x => x * 2
filtered = data.filter [1, 2, 3, 4], lambda x => x % 2 == 0
total = data.reduce [1, 2, 3], lambda a, b => a + b, 0
unique = data.unique [1, 1, 2, 2]
flat = data.flatten [[1, 2], [3, [4]]]
small = data.pick user, ["name", "version"]
without = data.omit user, ["password"]
merged = data.merge left, right
values = data.values user, ["name", "version"]
object = data.zip ["name", "version"], ["Lucy", "1.0.1"]

value = data.parse "{\"name\":\"Lucy\"}"
json = data.stringify value
pretty = data.pretty value
loaded = data.json_read "config.json"
data.json_write "config.json", value

rows = data.csv_parse "name,age\nNima,21"
text = data.csv_stringify rows
config = data.yaml_load "name: Lucy"
output = data.yaml_dump config
```

## `text`

```lucy
import text

text.capitalize "lucy"
text.upper "lucy"
text.lower "LUCY"
text.trim "  Lucy  "
text.reverse "Lucy"
text.repeat "ha", 3
text.split "a,b,c", ","
text.join ["a", "b", "c"], ","
text.replace "hello", "hello", "hi"
text.contains "hello", "ell"
text.starts_with "hello", "he"
text.ends_with "hello", "lo"
text.length "Lucy"
text.to_int "42"
text.to_float "3.14"

text.match "^[a-z]+$", "lucy"
text.search "world", "hello world"
text.matches "[0-9]+", "a12 b34"
text.replace_regex "[0-9]+", "X", "a12 b34"

text.base64_encode "Lucy"
text.base64_decode encoded
text.hex_encode "Lucy"
text.hex_decode encoded
text.url_encode "hello world"
text.url_decode encoded

text.shell_split "hello 'world test'"
text.shell_escape argument
text.shell_join ["hello", "world"]
```

## `system`

```lucy
import system

system.platform()
system.version()
system.cwd()
system.argv()
system.env "HOME"
system.setenv "MODE", "production"
system.pid()
system.ppid()
system.home()
system.temp_dir()
system.command_exists "git"

result = system.run "echo Lucy"
output = system.capture "printf hello"
system.spawn "long-running-command"
system.wait pid
system.waitpid pid
system.kill signal, pid
system.uid()
system.gid()
system.euid()
system.egid()
system.groups()
system.clock_gettime()

system.trap signal, callback
system.signals()
system.signal_name signal
system.login()
system.user name
system.user_id uid
system.shell_split command
system.shell_escape argument
system.shell_join arguments
```

## `math`

`math` exposes `square`, `cube`, `clamp`, `even`, `odd`, `abs`, `sqrt`, `pow`, `sin`, `cos`, `tan`, `floor`, `ceil`, `log`, `log10`, `exp`, `min`, `max`, `factorial`, `gcd`, `lcm`, `average`, `pi`, and `e`.

```lucy
import math
print math.sqrt 81
print math.gcd 84, 30
print math.pi()
```

## `random`

```lucy
import random
random.int 1, 100
random.float()
random.bool()
random.choice ["red", "green", "blue"]
random.shuffle [1, 2, 3]
random.sample [1, 2, 3, 4], 2
```

## `crypto`

```lucy
import crypto
crypto.digest "Lucy"
crypto.hexdigest "Lucy"
crypto.hash "Lucy"
crypto.base64digest "Lucy"
crypto.file "package.tar"
crypto.hmac "secret", "message"
```

The compatibility classes `Digest`, `HMAC`, `OpenSSL`, `SSLContext`, `SSLSocket`, `Certificate`, `RSA`, `Cipher`, `X509`, and `PKey` remain available through explicit imports such as `from crypto import Digest` when class-level access is required. They are not exposed by ordinary `import crypto`.

## `set`

```lucy
import set

numbers = set.new [1, 2, 3]
set.add numbers, 4
set.include? numbers, 2
set.union left, right
set.intersection left, right
set.difference left, right
set.symmetric_difference left, right
set.subset? left, right
set.superset? left, right
set.intersect? left, right
```

## `flow`

```lucy
import flow

value = flow.pipe 1, [lambda x => x + 1, lambda x => x * 2]
flow.tap value, lambda x => print x
flow.branch value, lambda x => x > 2, lambda x => x * 10, lambda x => x
flow.repeat 1, 5, lambda x => x * 2
```

## `result`

```lucy
import result

answer = result.ok 42
failure = result.err "not found"
result.success answer
result.unwrap answer
result.unwrap failure, 0
result.message failure
```

## `sqlite`

```lucy
import sqlite

db = sqlite.open "app.sqlite"
sqlite.execute db, "CREATE TABLE users (name TEXT)"
sqlite.execute db, "INSERT INTO users VALUES ('Nima')"
rows = sqlite.query db, "SELECT * FROM users"
```

## `time`

```lucy
import time

now = time.now()
today = time.today()
date = time.date "2026-09-19"
parsed = time.parse "2026-09-19 12:30:00"
time.format parsed, "%Y-%m-%d"
now + 1000
now - 1000
now <=> parsed
```

## `runtime`

```lucy
import runtime

runtime.type value
runtime.inspect value
runtime.methods value
runtime.responds value, "push"
runtime.call value, "push", [42]
runtime.ancestors object
runtime.superclass class_value
runtime.variables()
runtime.globals()
runtime.printf "%s", ["Lucy"]
```

## `app`

```lucy
import app

parser = app.parser()
parser.on "name", "User name", "Lucy"
options = parser.parse ARGV

log = app.logger()
log.info "application started"

bench = app.benchmark()
bench.measure callback

template = app.template "Hello {{name}}"
template.result {name: "Lucy"}
```

## `repl`

`repl.lucy` is ordinary editable Lucy code. The native C++ REPL owns terminal editing, history, completion and command dispatch, while the visible banner, prompt, topics and help text live in this library.

```lucy
import repl
repl.version()
repl.prompt()
repl.commands()
repl.topics()
repl.help "modules"
```

## Compatibility and imports

Ordinary imports expose **only public top-level functions** as the module API. Internal classes, singleton objects and private helpers are not leaked.

```lucy
import fs
fs.read "file.txt"

from fs import File
file = File.new()
```

This gives Lucy a consistent rule:

```text
import module
module.function(...)
```

rather than:

```text
module.object.function(...)
module.subsystem.function(...)
module.namespace.object.function(...)
```

The internal native bridge remains private and uses `__name__` functions. Public Lucy code does not need to know about those bridges.
