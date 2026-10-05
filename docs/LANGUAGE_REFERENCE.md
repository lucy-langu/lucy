# Lucy 2.0.0 Language Reference

## 1. Source structure

Lucy is case-sensitive. Blocks use `{` and `}`. `end` is not required to close a block in the 2.0 language surface.

```lucy
if ready {
    println "ready"
}
```

Statements may be separated by newlines. `;` can separate multiple statements on one line where the parser accepts a statement boundary:

```lucy
let a = 1; let b = 2; println a + b
```

Newlines inside parenthesized, bracketed, and braced expressions may be ignored by the parser. This makes multiline calls and collection literals practical.

Comments use `#` for line comments. ## 2. Names and declarations

```lucy
let name = "Lucy"
var count = 0
const version = 2
```

A `global` declaration creates or accesses a global binding where the runtime permits it:

```lucy
global total = 0
```

Declarations may include runtime-enforced type contracts:

```lucy
let count: Int = 10
var title: String = "Lucy"
```

## 3. Values

The core value model includes nil, booleans, integers, floating-point numbers, strings, arrays, maps, functions, classes, instances, native values, and the runtime's collection/object forms.

### Nil and booleans

```lucy
let missing = nil
let enabled = true
let disabled = false
```

### Numbers

```lucy
let decimal = 42
let grouped = 1_000_000
let hex_value = 0xFF
let binary_value = 0b1010_1010
let ratio = 3.14
```

Hexadecimal and binary integer literals are converted to Lucy integer values. `_` is permitted as a digit separator in valid positions. Malformed prefixes and separators are rejected lexically.

### Strings

```lucy
let a = "hello"
let b = 'world'
let name = "Nima"
let message = "Hello $name"
```

Interpolation uses `$identifier` inside strings.

### Arrays

```lucy
let items = [1, 2, 3]
```

Arrays are mutable runtime values. They support numeric indexing, negative indexes, ranges, and the collection methods documented in **PROGRAMMING_GUIDE.md** and **STANDARD_LIBRARY.md**.

### Maps

```lucy
let user = {
    name: "Lucy",
    "display-name": "Lucy 2",
    'version': 2
}
```

Bare identifier keys and quoted string keys are supported. Quoted keys are the correct form when the key contains punctuation or should be treated explicitly as a string.

## 4. Numeric base conversion

Lucy 2.0 provides direct integer-to-base conversion:

```lucy
println bin(255)       # 11111111
println hex(255)       # FF
println oct(255)       # 377
```

Optional width pads with leading zeroes:

```lucy
println bin(5, 8)      # 00000101
println hex(15, 4)     # 000F
println oct(9, 4)      # 0011
```

Negative values preserve the sign:

```lucy
println bin(-5)        # -101
```

Number values also provide:

```lucy
println 255.to_binary()
println 255.to_hex()
println 255.to_octal()
println 5.to_binary(8)
```

## 5. Operators

### Arithmetic

`+`, `-`, `*`, `/`, `%`, `**`

### Comparison

`==`, `!=`, `===`, `!==`, `>`, `>=`, `<`, `<=`, `<=>`

### Logical

`and`, `or`, `not`, `&&`, `||`, `!`

### Bitwise

`&`, `|`, `^`, `~`, `<<`, `>>`

### Assignment

`=`, `+=`, `-=`, `*=`, `/=`, `%=`, `**=`, `&=`, `|=`, `^=`, `<<=`, `>>=`

### Increment and decrement

`++`, `--`

### Membership and conditional

`in`, `? :`, `??`

### Ranges

`..` creates an inclusive range and `...` creates an exclusive-end range:

```lucy
let inclusive = 1..5
let exclusive = 1...5
```

The same operators can be used for array slicing:

```lucy
let values = [0, 1, 2, 3, 4]
println values[1..3]
println values[1...3]
```

## 6. Precedence

From tighter to looser evaluation, the expression parser separates unary operations, power, multiplication/division/modulo, addition/subtraction, shifts, comparisons, equality, bitwise operators, logical operators, coalescing, ternary expressions, and assignment. Parentheses should be used whenever precedence would make a non-trivial expression difficult to read.

Example:

```lucy
let result = (ready && enabled) ? value : fallback
let name = nickname ?? "anonymous"
```

## 7. Function calls

Both forms are valid when unambiguous:

```lucy
println("hello")
println "hello"
```

Multiline calls are supported:

```lucy
let value = some_function(
    first,
    second,
    third
)
```

## 8. Functions

```lucy
func add(a: Int, b: Int) -> Int {
    return a + b
}
```

Parameters can have defaults:

```lucy
func greet(name: String = "Lucy") -> String {
    return "Hello $name"
}
```

Variadic parameters are supported by the function parser where the current implementation permits them. A variadic parameter cannot also have a default value.

Return contracts are enforced at runtime:

```lucy
func bad() -> Int {
    return "not an integer"
}
```

The call raises `TypeError` rather than silently returning a value of the wrong type.

## 9. Lambdas

Lucy 2.0 implements expression lambdas:

```lucy
let double = lambda x => x * 2
let add = lambda(x, y) => x + y
```

The current grammar treats the expression after `=>` as the lambda result. Block-bodied lambda syntax should not be documented as a current feature.

## 10. Conditions

```lucy
if score >= 90 {
    println "excellent"
} else if score >= 60 {
    println "pass"
} else {
    println "retry"
}
```

`unless` provides the inverse condition form:

```lucy
unless ready {
    println "waiting"
}
```

## 11. Loops

### while

```lucy
while count < 10 {
    count += 1
}
```

### do-while

The parser accepts the `do` form for a post-test loop:

```lucy
do {
    count += 1
} while count < 10
```

### repeat

`repeat` is the language token used by the current implementation for the do-while-style form in the documented syntax surface where applicable. Prefer the syntax accepted by the current parser/tests when writing portable examples.

### for

```lucy
for value in [10, 20, 30] {
    println value
}
```

An index variable may be requested:

```lucy
for value, index in [10, 20, 30] {
    println "$index = $value"
}
```

### foreach

`foreach` is an explicit collection-loop spelling with the same indexed form:

```lucy
foreach value, index in ["a", "b"] {
    println "$index: $value"
}
```

### loop

```lucy
loop {
    println "running"
    break
}
```

`break` exits the nearest loop. `continue` skips to the next iteration.

## 12. switch

```lucy
switch status {
case 200:
    println "ok"
case 404:
    println "missing"
default:
    println "other"
}
```

The implementation selects the first matching case. Keep case bodies explicit and use `break` where the current switch semantics require early exit.

## 13. Nil coalescing

`??` returns the left value when present and non-nil; otherwise it evaluates the fallback:

```lucy
let nickname = nil
let name = nickname ?? "nima"
```

A missing variable on the left is treated as absent only for the coalescing operation:

```lucy
let name = nickname ?? "nima"
```

A normal reference to `nickname` still raises `NameError`.

## 14. Exceptions

```lucy
try {
    throw "invalid state"
} catch error {
    println error
} finally {
    println "cleanup"
}
```

`throw` raises a runtime exception. `catch` handles an exception and `finally` runs cleanup logic.

## 15. Imports and modules

```lucy
import mymodule
from mymodule import helper
import mymodule as m
```

The shipped standard modules are loaded automatically in 2.0. User modules and packages still use the import system.

## 16. Classes

```lucy
class User {
    func initialize(name) {
        self.name = name
    }

    func label() -> String {
        return self.name
    }
}

let user = User.new("Nima")
println user.label()
```

Inheritance uses `<`:

```lucy
class Dog < Animal {
    func speak() -> String {
        return "dog"
    }
}
```

`self` refers to the receiver. `super` participates in inherited behavior where supported by the runtime.

## 17. Structs

```lucy
struct Point {
    x: Int
    y: Int

    func sum() -> Int {
        return self.x + self.y
    }
}

let p = Point.new(3, 4)
println p.sum()
p.x = 10
```

Struct fields can have runtime type contracts. Struct construction and field access use the same object model exposed by the runtime.

## 18. Extending built-in types

A built-in type can be reopened with a normal class declaration:

```lucy
class String {
    func shout() -> String {
        return self.upper() + "!"
    }
}

println "hello".shout()
```

This is the public language mechanism. Application code should not use an internal `runtime.extend_type(...)` API; that is not the current public model.

## 19. Shell expressions

Backticks execute a host command and capture its output:

```lucy
let output = `pwd`
println output
```

Shell expressions are host-dependent. Use the `system` standard module when you need explicit process and command APIs.

## 20. Program termination

```lucy
exit()
exit(0)
exit(1)
```

`exit` terminates the current process with an integer status. In scripts, uncaught runtime failures also result in a non-zero process status.

## 21. Empty blocks

Empty blocks are valid:

```lucy
func noop() {}
if true {}
```

This is useful when generating code or intentionally leaving a hook without a body.
