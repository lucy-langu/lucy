# Lucy 2.0.0 — Complete Language Reference

This is the canonical language reference for Lucy 2.0.0. It describes behavior implemented by the lexer, parser, runtime, shipped library, and regression tests. Unsupported features are explicitly marked instead of being presented as future syntax.

## 1. Source files and lexical rules

Lucy is case-sensitive. Blocks use `{` and `}`. Normal 2.0 source does not use `end`.

### 1.1 Comments

`#` begins a line comment. The lexer also recognizes the supported multiline-comment form `=begin` / `=end`.

### 1.2 Statement termination

A newline can terminate a statement where the parser accepts a statement boundary. `;` can also separate statements:

```lucy
let a = 1; let b = 2
```

Semicolons are not mandatory after every statement.

A backslash immediately followed by a newline explicitly continues the logical line:

```lucy
let total = first + \
    second
```

### 1.3 Identifiers

Identifiers are ASCII-based. The first character is an ASCII letter or `_`; subsequent characters can contain letters, digits, and `_`. `?` and `!` are supported as trailing identifier characters for method/function names.

Unicode identifiers are not a separate supported lexer feature.

### 1.4 Strings

Both quote styles create `String` values:

```lucy
"hello"
'hello'
```

Supported escapes:

| Escape | Result |
|---|---|
| `\\` | backslash |
| `\"` | double quote |
| `\'` | single quote |
| `\0` | NUL |
| `\b` | backspace |
| `\n` | newline |
| `\r` | carriage return |
| `\t` | tab |
| `\xNN` | byte from two hexadecimal digits |
| `\uXXXX` | Unicode code point encoded as UTF-8 |
| backslash + newline | line continuation |

Unknown escapes are errors. Heredocs, raw-string delimiters, and arbitrary multiline-string syntax are not separate 2.0.0 string forms.

### 1.5 Interpolation

String interpolation resolves simple names and dotted member paths:

```lucy
let name = "Lucy"
println "Hello, $name"
println "Platform: $system.platform"
```

Arbitrary `${expression}` interpolation is not supported. There is no separate interpolation escape syntax for `$`; construct a literal dollar sign when necessary.

### 1.6 Numeric literals

Supported forms include:

```lucy
42
1_000_000
3.14
1.5e2
2e-3
0xFF
0b1010_1010
```

Hexadecimal and binary literals validate digit/separator placement. Legacy leading-zero octal literals, numeric suffixes, and `Infinity`/`NaN` literal spellings are not special literal syntax.

## 2. Keywords and reserved words

The parser/lexer keyword surface includes:

```text
if unless else while repeat do for foreach loop
switch case default
func lambda class struct
return break continue
import from as
const let var global
in and or not
try catch finally throw
true false nil
self super
```

`new` is accepted by the parser as a name token used by construction syntax; it is not a separate lexer keyword in the normal identifier model.

`repeat` and `do` select the same post-test-loop production.

`super` is reserved, but 2.0.0 does not implement a standalone `super()` / `super.method(...)` expression production. Do not document or rely on such syntax.

## 3. Values and runtime types

The runtime value categories are:

| Runtime type | Description |
|---|---|
| `nil` | absence of a value |
| `bool` | `true` / `false` |
| `int` | signed integer represented by the runtime's `long long` storage |
| `double` | IEEE-style host double precision |
| `string` | UTF-8-capable byte string representation |
| `array` | ordered mutable sequence |
| `map` | mutable string-keyed map |
| `function` | Lucy or native callable |
| `class` | class object |
| `instance` | class instance |
| `native` | native extension object |

Public type names available for annotations include:

```text
Nil Bool Int Double Number String Array Map
Function Class Instance Native Any
```

`Number` accepts `Int` and `Double`. `Any` accepts all values.

Generic types, union types, nullable syntax such as `Int?`, and static compile-time type checking are not implemented.

## 4. Declarations and scope

### 4.1 Bindings

```lucy
let name = "Lucy"
var counter = 0
const answer = 42
global shared = 10
```

`let` and `var` are mutable. `const` is immutable after initialization. A declaration without an initializer receives `nil`.

### 4.2 Type annotations

```lucy
let count: Int = 10
var label: String = "hello"
```

The runtime checks the assigned value against the declared type immediately. Function parameter and return contracts are also checked at runtime.

### 4.3 Scope

Blocks and functions create lexical child environments. Closures capture the defining environment by reference.

```lucy
let value = 1
if true {
    let value = 2
    println value
}
println value
```

Same-scope redeclaration replaces the existing binding. Shadowing in a child scope is allowed. There is no temporal-dead-zone model and no separate compile-time hoisting phase.

An unknown name raises `NameError`.

### 4.4 `global`

`global name = value` writes to the root environment. It is useful when a function or nested block must intentionally update a root binding. Ordinary reads do not require a `global` declaration.

## 5. Truthiness

Falsy values are:

- `nil`
- `false`
- numeric `0` and `0.0`
- `""`
- `[]`
- `{}`

All other values are truthy.

This rule is used by `if`, `while`, `repeat`/`do`, `unless`, logical operators, and other runtime condition checks.

## 6. Operators

The parser precedence, from highest binding to lowest, is:

| Level | Operators | Associativity |
|---:|---|---|
| 1 | calls `f(...)`, indexing `a[...]`, member `.`, postfix `++ --` | left-to-right chaining |
| 2 | prefix `! not`, unary `+ - ~`, prefix `++ --` | right-to-left |
| 3 | `**` | right-to-left |
| 4 | `* / %` | left-to-right |
| 5 | `+ -` | left-to-right |
| 6 | `.. ...` | range production |
| 7 | `<< >>` | left-to-right |
| 8 | `< <= > >= <=> in` | left-to-right |
| 9 | `== != === !==` | left-to-right |
| 10 | `&` | left-to-right |
| 11 | `^` | left-to-right |
| 12 | `|` | left-to-right |
| 13 | `and &&` | left-to-right |
| 14 | `or ||` | left-to-right |
| 15 | `??` | right-recursive |
| 16 | `? :` | ternary |

Assignment operators are statement-level assignment forms rather than ordinary expression operators:

```text
= += -= *= /= %= **= &= |= ^= <<= >>=
```

### 6.1 Arithmetic

`+ - * / % **` operate on numeric values; `+` also supports string/sequence behavior where implemented by the runtime.

Division by zero produces `ZeroDivisionError` where the runtime detects it.

### 6.2 Comparison and equality

`< <= > >=` compare supported comparable values.

`<=>` returns `-1`, `0`, or `1` for supported comparable values.

`==` is Lucy's ordinary equality operation. Numeric integers and doubles compare numerically. For complex values, the runtime's implemented equality behavior applies; it is not a promise of universal deep structural equality.

`===` requires matching runtime type names and equality.

`!=` and `!==` are the corresponding negations.

### 6.3 `in`

- Array: tests membership using `==`.
- Map: tests a key using the runtime's string-key representation.
- String: tests substring membership.
- Other right-hand values: `TypeError`.

### 6.4 Logical operators

`and`/`&&` and `or`/`||` short-circuit. `not`/`!` perform logical negation.

### 6.5 Null coalescing

```lucy
let name = nickname ?? "Lucy"
```

The left side is evaluated first. If it is `nil`, the right side is evaluated. A specific missing-name `NameError` on the left is also treated as an absent value for `??`; unrelated errors propagate.

### 6.6 Bitwise operators

`& | ^ ~ << >>` require integer-compatible operands. Floating-point bitwise operations raise `TypeError`.

### 6.7 Increment/decrement

Prefix forms return the new value; postfix forms return the old value while mutating the target:

```lucy
++count
count++
--count
count--
```

### 6.8 Ranges

`a..b` is inclusive. `a...b` excludes the end.

Ranges can be used for array slicing and iteration. `range(start, stop, step)` is a separate builtin that uses an exclusive stop.

## 7. Indexing and collections

Arrays use zero-based indexes. Negative indexes count backward from the end. Range indexing creates a new array.

```lucy
let values = [10, 20, 30, 40]
println values[0]
println values[-1]
println values[1..2]
println values[1...3]
```

Maps use string keys:

```lucy
let config = {"host": "localhost", "port": 8080}
println config["host"]
```

Strings can be indexed/sliced according to the runtime's string representation; string methods are documented in the standard-library reference.

## 8. Functions

### 8.1 Declaration

```lucy
func add(a: Int, b: Int) -> Int {
    return a + b
}
```

Functions are first-class values and may be stored in variables, passed to callbacks, returned, or captured by closures.

### 8.2 Calls

Normal calls:

```lucy
add(2, 3)
```

Command-style calls are supported where the parser can unambiguously treat the next expression as an argument:

```lucy
println "hello"
items.push 10
```

Use parentheses when the expression would otherwise be ambiguous.

### 8.3 Named arguments

```lucy
func connect(host, port = 80) {
    return "$host:$port"
}

connect(host: "localhost", port: 8080)
```

Once a named argument appears, later arguments must also be named.

### 8.4 Defaults

Default expressions are evaluated at call time. They are not frozen at function declaration time.

### 8.5 Variadic parameters

```lucy
func collect(first, *rest) {
    return rest
}
```

The variadic parameter must be last, cannot have a default, and receives an Array of remaining positional arguments.

### 8.6 Return

`return value` exits the current function. Bare `return` returns `nil`.

### 8.7 Closures and recursion

Closures capture lexical environments by reference:

```lucy
func make_adder() {
    let offset = 10
    return lambda value => value + offset
}
```

Recursion is supported. Lucy does not promise tail-call optimization; deep recursion is limited by the host/runtime call stack.

## 9. Lambdas

Lucy 2.0.0 has expression-bodied lambdas:

```lucy
let double = lambda x => x * 2
let add = lambda(a, b) => a + b
```

They are closures and first-class functions.

Block-bodied or statement-bodied lambda syntax is not a separate supported form in 2.0.0.

## 10. Control flow

### 10.1 `if` / `else`

```lucy
if score >= 90 {
    println "A"
} else {
    println "below A"
}
```

### 10.2 `unless`

`unless condition { ... }` is the inverse conditional. An `else` branch is supported where the parser accepts the ordinary conditional structure.

### 10.3 `while`

```lucy
while condition {
    # body
}
```

### 10.4 Post-test loop

`repeat` and `do` are aliases for the same post-test loop:

```lucy
repeat {
    work()
} while condition
```

The body executes once before the condition is tested.

### 10.5 `loop`

`loop { ... }` is an unconditional loop. Lucy has a runtime loop-iteration guard to prevent accidental infinite execution from consuming the process indefinitely.

### 10.6 `for` / `foreach`

```lucy
for value in values {
    println value
}

for value, index in values {
    println "$index: $value"
}
```

`foreach` is the parser's alias form. The iterable must be an Array or range in the current runtime. Maps and strings are not general `for` iterables.

### 10.7 `switch`

```lucy
switch value {
    case 1: println "one"
    case 2: println "two"
    default: println "other"
}
```

The first matching case executes and the switch exits. There is no implicit fallthrough model. `break` is not required to stop the selected case.

### 10.8 `break` and `continue`

`break` exits the current supported loop/switch context. `continue` advances the current loop. Labeled breaks/continues are not implemented.

## 11. Classes and objects

### 11.1 Class declaration

```lucy
class User {
    func initialize(name) {
        self.name = name
    }

    func greet() {
        return "Hello, $self.name"
    }
}
```

`initialize` is the constructor convention. `User.new(...)` creates an instance and invokes the first available `initialize` method along the inheritance chain.

### 11.2 Inheritance

```lucy
class Admin < User {
    func admin?() {
        return true
    }
}
```

Lucy supports single inheritance.

### 11.3 Unsupported OOP features

2.0.0 does not provide separate visibility modifiers, static/class method declarations, method overloading as a language feature, multiple inheritance, mixins, interfaces, abstract classes, or a standalone `super` expression syntax.

### 11.4 Built-in type extension

Built-in types are exposed as classes and can be extended:

```lucy
class String {
    func shout() {
        return self.upper() + "!"
    }
}

println "hello".shout()
```

The runtime installs built-in type classes for `Object`, `Nil`, `Bool`, `Int`, `Double`, `Number`, `String`, `Array`, `Map`, `Function`, `Class`, `Instance`, and `Native`.

## 12. Structs

Structs are class-like runtime objects carrying struct metadata:

```lucy
struct Point {
    x: Int
    y: Int
}
```

They are not a separate C++-style value representation. Construction uses `.new(...)`.

## 13. Modules and imports

### 13.1 Automatic standard modules

The interpreter automatically loads the shipped modules:

```text
app crypto data flow fs http math random repl result runtime set sqlite system text time
```

SQLite may be unavailable when Lucy was built without SQLite support; other standard-library load failures are reported as standard-library errors.

### 13.2 Import forms

```lucy
import tools
import tools as t
from tools import helper, other
```

A normal module import binds a module object. A selective import binds named members directly.

### 13.3 User modules

A module is a Lucy source file. Its public bindings become members of the module object. Names beginning with `_` are kept private; public functions, classes, constants, and helper objects can be exposed through normal module-member access.

### 13.4 Circular imports

Circular module dependencies are detected and raise `ImportError`.

### 13.5 Packages

A package can contain `src/init.lucy`. The current runtime resolves package/source paths through the project/install search path. `lucy.toml` is not currently parsed as an enforced package manifest by the runtime.

## 14. Exceptions

Runtime errors are reported as textual categories, for example:

```text
SyntaxError
NameError
TypeError
ArgumentError
IndexError
ValueError
IOError
ImportError
RuntimeError
OperatorError
ZeroDivisionError
LoopError
SQLiteError
HTTPError
ProcessError
ExtensionError
AssertionError
```

The language supports:

```lucy
try {
    risky()
} catch TypeError as error {
    println error
} finally {
    cleanup()
}
```

Catch matching uses the runtime's textual error category. The caught value is currently a string containing the error message.

There is no public custom exception-class API, structured stack-trace object, or dedicated re-raise syntax in 2.0.0. Multiple `catch` branches can be represented by category matching supported by the parser/runtime, but there is no promise of a conventional object-oriented exception hierarchy.

## 15. Shell and process interaction

Backticks provide command expressions:

```lucy
let output = `echo Lucy`
println output
```

For richer process control use the `system` standard-library module. Exact stdout/stderr and process semantics are documented there.

## 16. Built-in functions

The normal global builtins and their current call shapes are:

| Function | Signature | Purpose |
|---|---|---|
| `print` | `print(value, ...)` | write values separated by spaces, no newline |
| `println` | `println(value, ...)` | write values separated by spaces and add newline |
| `input` | `input(prompt = nil)` | read one line |
| `exit` | `exit(code = 0)` | terminate the process |
| `len` | `len(value)` | string/array/map length |
| `str` | `str(value)` | string representation |
| `int` | `int(value, base = 10)` | integer conversion |
| `float` | `float(value)` | floating-point conversion |
| `type` | `type(value)` | runtime type name |
| `typeof` | `typeof(value)` | alias of `type` |
| `range` | `range(stop)` / `range(start, stop, step)` | integer array with exclusive stop |
| `sum` | `sum(array)` | numeric sum |
| `min` | `min(value, ...)` | minimum numeric argument |
| `max` | `max(value, ...)` | maximum numeric argument |
| `abs` | `abs(value)` | absolute value |
| `sqrt` | `sqrt(value)` | square root |
| `sin` / `cos` / `tan` | `(value)` | trigonometric functions |
| `exp` | `exp(value)` | exponential |
| `floor` / `ceil` | `(value)` | floor/ceiling |
| `log` / `log10` | `(value)` | natural/base-10 logarithm |
| `pow` | `pow(base, exponent)` | exponentiation |
| `assert` | `assert(condition, message = nil)` | raises `AssertionError` when false |
| `bin` / `hex` / `oct` | `(integer, width = 0)` | base conversion with optional zero padding |

Compatibility/runtime globals also exist:

```text
read_file write_file exists cwd getenv sleep millis modules_info
```

Prefer `fs`, `system`, or `time` module APIs in new code when an equivalent exists. `modules_info()` is useful for inspecting resolved module paths.

Some `__*` names are runtime implementation helpers and are not public application APIs. There is no ordinary global `bool()` conversion builtin in the current runtime.

## 17. Reflection and runtime utilities

The `runtime` module provides Lucy-level helpers for reflection and invocation, including `methods`, `responds`, `call`, `inspect`, `variables`, `globals`, `ancestors`, and `superclass`. See the standard-library reference for exact signatures.

## 18. Memory, concurrency, and execution model

Lucy 2.0.0 is a C++17 interpreter. Runtime values use C++ containers and smart pointers. There is no public language-level garbage collector API.

The language does not define a built-in async/await model, generators/yield, scheduler, thread abstraction, or thread-safety guarantee. Native extensions may use host facilities, but they must not assume the interpreter is generally thread-safe.

## 19. Limits

- Integer storage follows the C++ `long long` runtime representation.
- Floating-point values use the host `double` representation.
- Loop execution has a runtime guard to prevent unbounded accidental loops.
- Recursion is ultimately limited by the host call stack.
- Collection and string sizes are limited by available process memory and host container limits rather than a Lucy language-level maximum.

## 20. What Lucy 2.0.0 does not implement

The following should not be written as 2.0 code without a corresponding language/runtime change:

- `${expression}` interpolation.
- Heredocs/raw-string syntax.
- Unicode identifiers.
- Generic/union/nullable type syntax.
- Static compile-time type checking.
- Block-bodied lambdas.
- Multiple inheritance/mixins/interfaces.
- Standalone `super` expressions.
- Public custom exception classes and structured stack traces.
- Async/await, generators, or a language-level scheduler.
- A package-manager command such as `lucy pkg install`.
- An LSP server or built-in debugger.
- A formal language-level thread API.

## 21. Grammar reference

The parser is implemented as a recursive-descent grammar in `src/parser.cpp`. The precedence table above is the authoritative semantic ordering. `GRAMMAR.md` is intentionally an EBNF-style explanatory grammar, not a claim of mechanically generated parser grammar.
