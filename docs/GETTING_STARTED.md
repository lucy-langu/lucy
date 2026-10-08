# Lucy 2.0.0 — Getting Started

This is the practical tutorial. It assumes no previous Lucy knowledge and deliberately uses complete, runnable examples.

## 1. Installation and first run

Build Lucy from the repository with CMake and C++17:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Optional SQLite support:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLUCY_ENABLE_SQLITE=ON
```

Run a program:

```text
lucy hello.lucy
```

Run the interactive REPL:

```text
lucy
```

The exact executable location depends on the build/install layout. See `REPL_AND_TOOLING.md` for CLI and platform details.

## 2. Your first program

```lucy
func square(value: Number) -> Number {
    return value * value
}

let values = [1, 2, 3, 4]
let squares = values.map(lambda value => square(value))

println squares
```

Lucy functions are first-class values, arrays provide functional methods, and lambdas use an expression-body syntax.

## 3. Variables and constants

```lucy
let name = "Lucy"
var counter = 0
const language_version = "2.0.0"

counter += 1
println name
println counter
```

`let` and `var` are mutable bindings. `const` cannot be reassigned.

An uninitialized declaration receives `nil`:

```lucy
let value
println value == nil
```

## 4. Type contracts

Annotations are runtime contracts, not compile-time-only comments:

```lucy
func add(a: Int, b: Int) -> Int {
    return a + b
}

println add(2, 3)
```

Passing the wrong type or returning the wrong type raises `TypeError`.

Available annotation names include `Nil`, `Bool`, `Int`, `Double`, `Number`, `String`, `Array`, `Map`, `Function`, `Class`, `Instance`, `Native`, and `Any`.

## 5. Strings

Both quote styles produce strings:

```lucy
let name = "Lucy"
let other = 'Lucy'
println "Hello, $name"
println name.upper()
println name.replace("Lucy", "Lucy 2")
```

Interpolation supports `$name` and dotted member paths. Arbitrary `${expression}` interpolation is not part of 2.0.0.

## 6. Arrays

```lucy
let values = [10, 20, 30]
values.push 40

println values[0]
println values[-1]
println values[1..2]
println values.map(lambda x => x * 2)
println values.filter(lambda x => x > 20)
```

Indexes start at zero. Negative indexes count from the end. `a..b` is inclusive; `a...b` has an exclusive end.

## 7. Maps

```lucy
let user = {
    "name": "Lucy",
    "version": 2
}

println user["name"]
user.set "version", 3
println user.get("version")
```

Map keys are strings in the current runtime. Use quoted keys when the spelling is not a valid identifier.

## 8. Conditions and loops

```lucy
if user.has("name") {
    println "named user"
} else {
    println "anonymous"
}

for value, index in ["a", "b", "c"] {
    println "$index -> $value"
}

let n = 0
while n < 3 {
    println n
    n += 1
}
```

Post-test loops use `repeat` or `do`:

```lucy
repeat {
    n += 1
} while n < 5
```

The body executes at least once.

## 9. Functions, defaults, named arguments, and variadics

```lucy
func connect(host: String, port: Int = 80) -> String {
    return "$host:$port"
}

println connect("localhost")
println connect(host: "example.com", port: 443)
```

Variadic parameters use `*name` and must be last:

```lucy
func collect(first, *rest) {
    return [first, rest]
}

println collect(1, 2, 3, 4)
```

Defaults are evaluated at call time. Positional arguments cannot appear after a named argument.

## 10. Classes

```lucy
class User {
    func initialize(name: String) {
        self.name = name
    }

    func greet() {
        return "Hello, $self.name"
    }
}

let user = User.new("Lucy")
println user.greet()
```

Inheritance is single-parent:

```lucy
class Admin < User {
    func is_admin() -> Bool {
        return true
    }
}
```

There are no visibility modifiers, interfaces, mixins, multiple inheritance, or separate static/class-method syntax in 2.0.0.

## 11. Structs

Structs use the same runtime object machinery as classes but are marked as struct classes:

```lucy
struct Point {
    x: Int
    y: Int
}

let p = Point.new(10, 20)
println p.x
```

They are not C++-style stack value types.

## 12. Modules

The shipped standard modules are loaded automatically. For example:

```lucy
println fs.read("hello.txt")
println text.upper("hello")
println system.platform()
println time.now()
```

You still use `import` for user modules and packages:

```lucy
import tools
import tools as t
from tools import helper
```

## 13. File and system work

```lucy
fs.write("notes.txt", "hello")
let content = fs.read("notes.txt")
println content

println system.cwd()
println system.platform()
println system.env("PATH")
```

For command execution, use the `system` module or backtick expressions as described in the language and standard-library references.

## 14. Error handling

```lucy
try {
    println int("not-a-number")
} catch ValueError as error {
    println "conversion failed: $error"
} finally {
    println "finished"
}
```

The caught value is currently the textual error message. Lucy 2.0.0 does not expose a public exception-object hierarchy.

## 15. Discovering Lucy interactively

Start with:

```text
Lucy$ :help
Lucy$ help()
Lucy$ help "modules"
```

Use Tab aggressively:

```text
Lucy$ fs.<Tab>
Lucy$ text.u<Tab>
Lucy$ values.<Tab>
```

For multiple matches, the first Tab displays candidates and completes their common prefix; subsequent Tabs cycle through them.

Use `repl.topics()` to see the built-in help topics and `repl.commands()` for REPL commands.

## 16. Recommended learning order

1. Variables, values, strings, arrays, and maps.
2. Conditions and loops.
3. Functions, defaults, named arguments, and lambdas.
4. Classes and built-in type extension.
5. Modules and the standard library.
6. Errors and filesystem/system APIs.
7. Packages and native extensions.

The complete semantics are in `LANGUAGE_REFERENCE.md`; do not infer a feature from an example alone.
