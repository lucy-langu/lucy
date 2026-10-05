# Getting Started with Lucy 2.0.0

## 1. What you need

A Lucy source tree contains the interpreter, runtime library, standard-library sources, documentation, examples, tests, and optional extension infrastructure. A C++17-capable toolchain and CMake are used by the project build.

The normal project workflow is:

```text
source tree → CMake configure → build → tests → install → run lucy
```

SQLite is optional. The default CMake option is `LUCY_ENABLE_SQLITE=OFF`; enable it when the platform provides SQLite development support.

## 2. Build

A typical out-of-tree build is:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

For a local installation:

```sh
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
cmake --install build
```

Windows can use the generator appropriate to the installed compiler, for example a MinGW generator when MinGW is installed.

## 3. Your first program

Create `hello.lucy`:

```lucy
let name = "Lucy"
println "Hello $name"
```

Run it with:

```sh
lucy hello.lucy
```

Lucy is case-sensitive, so `name`, `Name`, and `NAME` are different identifiers.

## 4. Variables and types

```lucy
let language: String = "Lucy"
var count: Int = 0

count += 1
println language
println count
```

`let` and `const` create immutable bindings. `var` creates a mutable binding. A type annotation is a runtime contract, not merely editor documentation.

```lucy
let port: Int = 8080
# port = "8080"  # TypeError
```

## 5. Functions

```lucy
func add(a: Int, b: Int) -> Int {
    return a + b
}

println add(20, 22)
println add 20, 22
```

Parentheses are optional when command-style parsing is unambiguous. Parenthesized calls remain available and are often clearer for nested expressions.

## 6. Control flow

```lucy
for value, index in [10, 20, 30] {
    println "$index: $value"
}

if count > 0 {
    println "running"
} else {
    println "empty"
}
```

Ranges are useful for numeric iteration:

```lucy
for i in 0...10 {
    println i
}
```

`...` excludes the end. `..` includes it.

## 7. Collections

```lucy
let user = {
    "name": "Nima",
    'active': true
}

let values = [10, 20, 30, 40, 50]
println values[1]
println values[1..3]
println values[-3..-1]
```

Array ranges are inclusive with `..` and exclusive with `...`:

```lucy
values[1..3]   # 20, 30, 40
values[1...3]  # 20, 30
```

## 8. Objects

```lucy
class Counter {
    func initialize(value: Int = 0) {
        self.value = value
    }

    func increment() {
        self.value += 1
    }
}

let counter = Counter.new(5)
counter.increment()
println counter.value
```

Built-in types can also be reopened with ordinary class declarations:

```lucy
class String {
    func shout() -> String {
        return self.upper() + "!"
    }
}

println "hello".shout()
```

The receiver is available as `self` even though the receiver is a built-in value.

## 9. REPL

Run `lucy` without a source file:

```text
Lucy$ 2 + 2
4
```

Use the REPL for experiments, inspect members with Tab completion, and use `:help`, `:history`, `:version`, `:clear`, `:exit`, or `:quit` where supported by the interactive command set.

## 10. A small real program

```lucy
let values = [12, 18, 7, 21]
let total = sum values

println "total = $total"

for value, index in values {
    if value >= 18 {
        println "[$index] $value: high"
    } else {
        println "[$index] $value: normal"
    }
}
```

This demonstrates the intended style: immutable data by default, explicit mutation when required, expression-oriented values, and direct standard-library access.

## 11. Where to go next

- Need syntax? Read **LANGUAGE_REFERENCE.md**.
- Need practical design patterns? Read **PROGRAMMING_GUIDE.md**.
- Need an API? Read **STANDARD_LIBRARY.md**.
- Need a package or C++ extension? Read **PACKAGES_AND_EXTENSIONS.md**.
- Need to understand an error? Read **RUNTIME_AND_ERRORS.md**.
