# Lucy 2.0.0 Programming Guide

This guide is about how to use Lucy effectively rather than listing grammar in isolation.

## 1. Prefer explicit data flow

Lucy values can be passed directly between functions and methods:

```lucy
func normalize(values) {
    return values.map(lambda value => value * 2)
}

let result = normalize([1, 2, 3])
println result
```

Keep transformations close to the data they operate on. Use modules for capabilities rather than inventing deep namespaces.

## 2. Arrays

Common methods include:

```lucy
items.push 4
items.pop()
items.shift()
items.unshift 0
items.insert 1, 99
items.remove_at 1
items.first()
items.last()
items.contains 3
items.count 3
items.index 3
items.join ","
items.reverse()
items.length()
items.size()
```

Functional operations accept a callable:

```lucy
let doubled = items.map(lambda x => x * 2)
let even = items.filter(lambda x => x % 2 == 0)
let has_large = items.any(lambda x => x > 100)
let all_positive = items.all(lambda x => x > 0)
```

## 3. Indexing and slicing

Indexes start at zero. Negative indexes count from the end.

```lucy
let values = [10, 20, 30, 40, 50]
println values[0]
println values[-1]
```

Range indexing creates a new array:

```lucy
values[1..3]    # 20, 30, 40
values[1...3]   # 20, 30
values[-3..-1]  # 30, 40, 50
```

The `slice` method uses a start index and optional end index, with an exclusive end:

```lucy
values.slice(1, 4)  # 20, 30, 40
values.slice(-3)    # 30, 40, 50
```

For loops can expose both value and index:

```lucy
for value, index in values {
    println "$index -> $value"
}
```

## 4. Maps

Use maps for named data:

```lucy
let config = {
    "host": "localhost",
    "port": 8080,
    "debug": true
}

println config["host"]
config["port"] = 8081
```

Map methods include `get`, `set`, `has`, `delete`, `keys`, `values`, `length`, `size`, and `clear`.

Use quoted keys when the key is not a valid identifier:

```lucy
let record = {
    "display-name": "Lucy",
    'api.version': 2
}
```

## 5. Strings

Strings expose object-style methods:

```lucy
let text = "  Lucy  "
println text.strip()
println text.upper()
println text.lower()
println text.contains "uc"
println text.starts_with "Lu"
println text.ends_with "y"
println text.slice(1, 3)
println text.char_at 0
println text.split " "
println text.replace("Lucy", "Lucy 2")
```

`to_int` and `to_float` parse ordinary numeric strings. Invalid input raises `ValueError`.

## 6. Numbers and bit-level work

Numeric methods include arithmetic helpers and conversions:

```lucy
println 25.sqrt()
println 25.to_int()
println 25.to_string()
println 255.to_binary()
println 255.to_hex()
println 255.to_octal()
```

Bitwise operators are useful for masks and protocol values:

```lucy
let flags = 0b1010
let mask = 0b0011
println flags & mask
println flags | mask
println flags ^ mask
println flags << 1
println flags >> 1
```

## 7. Functions with contracts

Use annotations where a function has a clear interface:

```lucy
func parse_port(text: String) -> Int {
    return text.to_int()
}
```

Lucy checks parameter and return contracts at runtime. This makes annotations useful even in scripts that do not have a static compiler.

## 8. Defaults and optional behavior

```lucy
func connect(host: String, port: Int = 80) {
    # ...
}
```

Defaults belong after required parameters. The parser rejects a variadic parameter with a default value.

## 9. Lambdas and callbacks

Callbacks are ordinary values:

```lucy
let printer = lambda value => println value
[1, 2, 3].each printer
```

For transformations:

```lucy
let names = ["alice", "bob"]
let upper = names.map(lambda name => name.upper())
```

## 10. Classes and object design

Put behavior with the value it operates on:

```lucy
class Account {
    func initialize(balance: Int = 0) {
        self.balance = balance
    }

    func deposit(amount: Int) {
        self.balance += amount
    }

    func withdraw(amount: Int) -> Bool {
        if amount > self.balance {
            return false
        }
        self.balance -= amount
        return true
    }
}
```

Built-in type extension follows the same public class mechanism:

```lucy
class String {
    func shout() -> String {
        return self.upper() + "!"
    }
}
```

## 11. Structs for data-oriented objects

Structs are useful when the shape of the object is important:

```lucy
struct SensorReading {
    value: Double
    timestamp: Int

    func valid() -> Bool {
        return self.value >= 0
    }
}
```

Use classes when identity and inherited behavior are central; use structs when a compact typed data shape is the main purpose.

## 12. Error strategy

Use exceptions for failures that should interrupt normal execution:

```lucy
try {
    result = risky_operation()
} catch error {
    println error
}
```

Use the `result` module when success/failure should be represented as ordinary data:

```lucy
let result = result.ok 42
if result.success(result) {
    println result.unwrap(result)
}
```

The exact fields depend on the current result implementation; use `result.success`, `result.unwrap`, and `result.message` for the public helper layer.

## 13. Files and data

```lucy
let text = fs.read "config.json"
let config = data.parse text

config["version"] = 2
fs.write "config.json", data.pretty config
```

The standard library is automatically available, so the above does not require imports for the shipped modules.

## 14. HTTP and networking

Simple HTTP:

```lucy
let response = http.get "https://example.com"
println response.status
println response.body
```

For more control:

```lucy
let client = http.client "https://example.com"
client.headers {"Accept": "application/json"}
client.timeout 10
let response = client.get "/api"
```

Raw sockets are exposed through the `http` module's socket layer where appropriate.

## 15. Time

```lucy
let now = time.now()
println now.year()
println now.month()
println now.day()
```

The module also exposes `Date`, `DateTime`, parsing, formatting, timestamps, and sleep helpers.

## 16. Reflection

Runtime reflection belongs in the `runtime` module:

```lucy
println runtime.type value
println runtime.inspect value
println runtime.methods value

if runtime.responds value, "start" {
    runtime.call value, "start", []
}
```

Reflection is useful for tooling, serializers, adapters, and debugging. Do not make normal business logic depend on reflection when direct calls are clearer.

## 17. Process control

The `system` module provides explicit OS/process functions:

```lucy
println system.platform()
println system.cwd()
println system.argv()
println system.capture "git --version"
```

Use `system.run` or `system.spawn` when process behavior matters. Use shell expressions only when the compact syntax is appropriate.

## 18. CLI applications

`app.OptionParser` and `app.Logger` provide higher-level application helpers:

```lucy
let parser = app.parser()
parser.program_name "tool"
parser.on("verbose", "enable verbose output", false)
let options = parser.parse()

let log = app.logger()
log.info "starting"
```

## 19. Keep public APIs flat

The historical standard-library design explicitly moved away from deeply nested wrappers. Prefer:

```lucy
fs.read "notes.txt"
http.get url
data.parse json
```

not invented structures such as:

```text
fs.file.read
http.socket.connect
```

The module is the capability boundary; the function or class is the operation.
