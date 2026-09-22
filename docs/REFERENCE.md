# Lucy Complete Reference

**Language version:** 1.0.1  
**License:** GNU GPL v3  
**Author:** Nima (`nimacpp` / `nimacxx`)

This is the complete reference for the Lucy language as implemented by the 1.0.1 interpreter. It is written to be useful to a first-time programmer without hiding details from experienced developers.

> **Important:** This document describes the language that exists in the current interpreter. It does not describe planned features as if they already existed.

---

## 1. What is Lucy?

Lucy is a dynamically typed, general-purpose programming language implemented in C++17.

Its design combines:

- a compact, readable syntax;
- Ruby-inspired method calls such as `items.push 10`;
- a Python-like interactive REPL;
- first-class functions and closures;
- classes, instances, constructors, and inheritance;
- exceptions;
- modules loaded from `.lucy` source files;
- a standard library whose public high-level API is written in Lucy;
- small native C++ primitives for operating-system access and native libraries.

A normal Lucy program is a text file ending in `.lucy`:

```lucy
print "Hello, Lucy!"
```

Run it with:

```bash
lucy hello.lucy
```

---

# Part I — Language Fundamentals

## 2. Source files

Lucy source files normally use the `.lucy` extension.

Statements are executed from top to bottom.

```lucy
name = "Lucy"
version = 11
print name
print version
```

There is no required module declaration and no required `main` function.

The first executable statement is simply the first statement in the file.

---

## 3. Source layout and multiline expressions

A newline normally separates statements. Inside matching `()`, `[]`, and `{}` delimiters, newlines are ignored so expressions can be formatted across multiple lines. The same rule applies to function and lambda parameter lists.

```lucy
response = http.get(
    url,
    headers,
    options
)

items = [
    first,
    second,
    third
]

config = {
    host: "localhost",
    port: 8080
}
```

This does not turn arbitrary newlines into whitespace: two top-level expressions on separate lines remain separate statements.

---

## 4. Comments

### Single-line comments

A `#` starts a comment that continues to the end of the line.

```lucy
# This is a comment.
name = "Lucy" # This is also a comment.
```

### Multiline comments

Lucy supports Ruby-style multiline comment markers:

```lucy
=begin
This entire section is ignored.
It can contain multiple lines.
=end
```

The closing marker must be `=end`.

---

## 5. Values and types

Lucy is dynamically typed. A variable does not have a permanently declared type.

The runtime currently supports these value types:

| Type | Example | Description |
|---|---|---|
| `nil` | `nil` | Absence of a value |
| `bool` | `true` | Boolean value |
| `int` | `42` | Signed integer |
| `double` | `3.14` | Floating-point number |
| `text` | `"Lucy"` | Text |
| `array` | `[1, 2, 3]` | Ordered mutable collection |
| `map` | `{name: "Lucy"}` | String-keyed mutable collection |
| `function` | `def add...` | Callable function |
| `class` | `class User...` | Class object |
| `instance` | `User.new()` | Object created from a class |

Check a value's runtime type with `type()` or `typeof()`:

```lucy
print type(nil)
print type(true)
print type(42)
print type(3.14)
print type("Lucy")
print type([1, 2])
print type({name: "Lucy"})
```

The returned names are exactly:

```text
nil
bool
int
double
string
array
map
function
class
instance
```

---

## 6. Truthiness

Lucy uses truthiness in conditions.

The following values are falsey:

- `nil`
- `false`
- integer `0`
- double `0.0`
- an empty string
- an empty array
- an empty map

Other supported values are truthy.

```lucy
if "hello"
    print "truthy"
end

if []
    print "this does not run"
end
```

This rule is also used by `not`, `and`, `or`, `assert`, and loop conditions.

---

## 7. Literals

### `nil`

```lucy
value = nil
```

### Booleans

```lucy
enabled = true
debug = false
```

### Integers

Integer literals are decimal integers.

```lucy
zero = 0
count = 42
negative = -7
```

Hexadecimal integer literals are not part of the current syntax.

### Doubles

```lucy
pi = 3.14159
ratio = 0.5
```

A decimal point followed by digits creates a `double`.

### Strings

Both single and double quotes are supported:

```lucy
name = "Lucy"
message = 'Hello'
```

Supported escapes include:

| Escape | Result |
|---|---|
| `\\n` | newline |
| `\\t` | tab |
| `\\r` | carriage return |
| `\\"` | double quote |
| `\\'` | single quote |
| `\\\\` | backslash |

Unknown escapes are preserved as a backslash followed by the character.

---

## 8. String interpolation

A `$name` sequence inside a string is replaced with the value of the variable.

```lucy
name = "Nima"
print "Hello $name"
```

One member access is supported in interpolation:

```lucy
user = {name: "Nima"}
print "Hello $user.name"
```

Interpolation works in both single-quoted and double-quoted strings.

An unknown interpolation target raises `NameError`.

---

# Part II — Variables and Scope

## 9. Variables

Variables are created by assignment:

```lucy
name = "Lucy"
age = 1
```

A variable can later hold a different type:

```lucy
value = 10
value = "ten"
```

If assignment targets an existing variable in an enclosing scope, Lucy updates that variable. If the name does not exist in an enclosing scope, a new variable is created in the current environment.

---

## 10. Constants

Use `const` for a binding that cannot be reassigned:

```lucy
const VERSION = "1.0.1"
```

Reassigning a constant raises `NameError`.

A constant may be declared without a value; it then contains `nil`:

```lucy
const VALUE
```

`const` prevents reassignment of the binding. Lucy's current runtime does not make an array or map deeply immutable merely because the variable is constant.

---

## 11. Global variables

Use `global` when a declaration should be placed directly in the global environment:

```lucy
global application_name = "Lucy App"
```

This is mainly useful when code is executing inside another scope and needs to deliberately publish a global binding.

---

## 12. Block scope

Control-flow blocks and explicit statement blocks use child environments.

A new variable created inside a block does not become visible after the block ends:

```lucy
if true
    inside = 10
end

# `inside` is not available here.
```

An assignment to an already-existing outer variable updates the outer binding:

```lucy
value = 10

if true
    value = 20
end

print value
```

This prints `20`.

---

## 13. Closures

Functions capture their lexical environment.

```lucy
def make_counter()
    value = 0

    def increment()
        value += 1
        return value
    end

    return increment
end

counter = make_counter()
print counter()
print counter()
```

The returned function continues to access the captured `value` after `make_counter()` has returned.

---

# Part III — Operators

## 14. Arithmetic operators

Lucy supports:

```text
+   -   *   /   %   **
```

Examples:

```lucy
print 2 + 3
print 7 - 4
print 3 * 4
print 10 / 4
print 10 % 3
print 2 ** 3
```

### Numeric result rules

- `int + int`, `int - int`, and `int * int` produce `int`.
- `/` produces a `double`.
- `**` produces a `double`.
- `%` requires integer-compatible operands and produces an `int`.
- Mixing `int` and `double` produces a `double` for arithmetic operations.

Division or modulo by zero raises `ZeroDivisionError`.

---

## 15. String addition

If the left operand of `+` is a string, Lucy converts the right operand to its string representation and concatenates it.

```lucy
print "count: " + 10
```

---

## 16. Array addition

If the left operand is an array:

- array + array creates a new concatenated array;
- array + another value creates a new array with that value appended.

```lucy
print [1, 2] + [3, 4]
print [1, 2] + 3
```

The original left array is not modified by `+`.

---

## 17. Comparison operators

```text
==   !=   ===   !==
>    >=   <    <=
```

### `==` and `!=`

`==` performs value equality. Numeric `int` and `double` values are compared numerically even when their runtime types differ.

```lucy
print 1 == 1.0
print 1 != 2
```

### `===` and `!==`

Strict equality also requires the same runtime type.

```lucy
print 1 === 1
print 1 === 1.0
```

The second expression is false because `int` and `double` are different types.

For arrays, maps, functions, classes, and instances, the current runtime uses their runtime string representation when performing its generic equality fallback. Code should not rely on deep structural equality for these composite values.

### Ordering

Numbers can be compared with `>`, `>=`, `<`, and `<=`.

Strings can also be compared lexicographically.

Comparing incompatible types raises `TypeError`.

---

## 18. Logical operators

Lucy supports both word and symbolic forms:

```text
and   &&
or    ||
not   !
```

Examples:

```lucy
if age >= 18 and active
    print "allowed"
end

if name == "Lucy" || name == "Ruby"
    print "language"
end

if !enabled
    print "disabled"
end
```

`and` and `or` evaluate to a boolean result in the current interpreter.

They use short-circuit evaluation.

---

## 19. Bitwise operators

```text
&   |   ^   ~   <<   >>
```

These operations require integer-compatible values.

```lucy
print 6 & 3
print 6 | 3
print 6 ^ 3
print ~1
print 1 << 3
print 8 >> 2
```

Compound assignment forms are also available.

---

## 20. Membership operator

`in` checks whether the left value exists in the right value.

### Array

```lucy
print 3 in [1, 2, 3]
```

### Map

The left value is converted to a string and checked as a key:

```lucy
user = {name: "Nima"}
print "name" in user
```

### String

The left value is converted to a string and searched as a substring:

```lucy
print "world" in "hello world"
```

The right operand must be an array, map, or string.

---

## 21. Ternary operator

```text
condition ? value_if_true : value_if_false
```

Example:

```lucy
status = age >= 18 ? "adult" : "minor"
```

---

## 22. Ranges

Lucy has inclusive and exclusive ranges:

```lucy
1..5
1...5
```

`1..5` produces:

```text
[1, 2, 3, 4, 5]
```

`1...5` produces:

```text
[1, 2, 3, 4]
```

Descending ranges are supported:

```lucy
5..1
5...1
```

Ranges are materialized as arrays by the current interpreter.

---

## 23. Assignment operators

Lucy supports:

```text
=   +=   -=   *=   /=   %=   **=
&=  |=   ^=   <<=  >>=
```

Examples:

```lucy
count = 1
count += 2
count *= 3
```

Assignments may target variables, array indexes, instance fields, and map members where supported.

---

## 24. Increment and decrement

Both prefix and postfix forms are recognized:

```lucy
++count
count++
--count
count--
```

The postfix form returns the old value while updating the target. The prefix form returns the new value.

---

# Part IV — Functions

## 25. Defining functions

`def` defines a function:

```lucy
def add(a, b)
    return a + b
end
```

`function` is an alias:

```lucy
function add(a, b)
    return a + b
end
```

Functions are first-class values and can be assigned, returned, and passed to other functions.

---

## 26. Calling functions

Normal call syntax:

```lucy
print add(2, 3)
```

Lucy also supports command-style calls when the parser can unambiguously recognize the argument expression:

```lucy
print "hello"
items.push 10
```

Parentheses are preferred when expressions become complex.

---

## 27. Return values

```lucy
def square(x)
    return x * x
end

result = square(5)
```

A bare `return` returns `nil`:

```lucy
def stop()
    return
end
```

A function that reaches its `end` without returning a value returns `nil`.

---

## 28. Default arguments

A parameter can have a default expression:

```lucy
def greet(name, greeting = "Hello")
    return greeting + ", " + name
end

print greet("Nima")
print greet("Nima", "Hi")
```

Required parameters cannot appear after a parameter that has a default value.

---

## 29. Named arguments

Named arguments use `name: value` at the call site:

```lucy
def connect(host, port = 80)
    return host + ":" + str(port)
end

print connect(host: "localhost", port: 8080)
```

Rules:

1. positional arguments must come before named arguments;
2. a named argument must match a parameter name;
3. the same parameter cannot be supplied twice;
4. a variadic parameter must be passed positionally.

---

## 30. Variadic arguments

A parameter prefixed with `*` collects remaining positional arguments into an array:

```lucy
def total(*values)
    return sum(values)
end

print total(1, 2, 3, 4)
```

A variadic parameter must be the final parameter.

Calling `total()` produces an empty array for `values`.

---

# Part V — Control Flow

## 31. `if`, `else if`, and `else`

```lucy
if score >= 90
    print "excellent"
else if score >= 60
    print "pass"
else
    print "retry"
end
```

The first truthy branch is executed.

---

## 32. `while`

```lucy
count = 0
while count < 5
    print count
    count += 1
end
```

The condition is evaluated before every iteration.

The interpreter has a safety limit of 10,000,000 iterations for a single `while` loop. Exceeding it raises `LoopError`.

---

## 33. `for`

`for` iterates over an array or range:

```lucy
for i in 1..5
    print i
end
```

The iterable must evaluate to an array.

---

## 34. `foreach`

`foreach` is an alias-like spelling of the array iteration form:

```lucy
foreach item in ["a", "b", "c"]
    print item
end
```

It uses the same runtime behavior as `for`.

---

## 35. `loop`

`loop` creates an unconditional loop:

```lucy
loop
    print "running"
    break
end
```

It also has a 10,000,000-iteration safety limit.

---

## 36. `break`

`break` exits the nearest `while`, `for`, `foreach`, or `loop`.

```lucy
for i in 1..10
    if i == 5
        break
    end
    print i
end
```

---

## 37. `continue`

`continue` skips the remainder of the current loop iteration.

```lucy
for i in 1..5
    if i == 3
        continue
    end
    print i
end
```

---

# Part VI — Arrays

## 38. Array literals

```lucy
numbers = [1, 2, 3]
mixed = [1, "two", true, nil]
empty = []
```

Arrays are ordered and mutable.

---

## 39. Array indexing

Indexes are zero-based:

```lucy
items = ["a", "b", "c"]
print items[0]
print items[2]
```

Negative indexes count from the end:

```lucy
print items[-1]
```

An out-of-range index raises `IndexError`.

---

## 40. Array assignment

```lucy
items = [1, 2, 3]
items[1] = 20
```

Assignment requires an existing index; Lucy does not automatically grow an array through an out-of-range assignment.

---

## 41. Array methods

Every array supports these methods:

| Method | Signature | Return |
|---|---|---|
| `push` | `push(value, ...)` | new length |
| `pop` | `pop()` | removed last value or `nil` |
| `shift` | `shift()` | removed first value or `nil` |
| `unshift` | `unshift(value, ...)` | new length |
| `insert` | `insert(index, value)` | same array |
| `remove_at` | `remove_at(index)` | removed value or `nil` |
| `clear` | `clear()` | `nil` |
| `first` | `first()` | first value or `nil` |
| `last` | `last()` | last value or `nil` |
| `contains` | `contains(value)` | `bool` |
| `count` | `count(value)` | `int` |
| `index` | `index(value)` | index or `nil` |
| `join` | `join(separator)` | `text` |
| `reverse` | `reverse()` | same array |
| `length` | `length()` | `int` |
| `size` | `size()` | `int` |
| `each` | `each(function)` | same array |
| `map` | `map(function)` | new array |
| `filter` | `filter(function)` | new array |
| `any` | `any(function)` | `bool` |
| `all` | `all(function)` | `bool` |

### `push`

```lucy
items.push 4
items.push 5, 6
```

It mutates the array and returns its new length.

### `pop`

Removes and returns the final element. Empty arrays return `nil`.

### `shift`

Removes and returns the first element. Empty arrays return `nil`.

### `unshift`

Adds one or more values to the beginning and returns the new length.

### `insert`

Inserts one value at a zero-based index. Negative indexes are counted from the end and are clamped to the valid insertion range. Returns the same array.

```lucy
items = [1, 3]
items.insert(1, 2)
```

### `remove_at`

Removes and returns the value at an index. Negative indexes are supported. An invalid index returns `nil`.

### `clear`

Removes all elements.

### `contains`

```lucy
items.contains(3)
```

Uses Lucy's equality rules.

### `count`

Counts values equal to the supplied value.

### `index`

Returns the first matching zero-based index or `nil`.

### `join`

Converts each element to its string representation and joins them:

```lucy
[1, 2, 3].join(", ")
```

### `reverse`

Reverses the array in place and returns the same array.

### `each`

Calls a function once for every element:

```lucy
def show(value)
    print value
end

[1, 2, 3].each(show)
```

### `map`

Creates a new array from callback return values:

```lucy
def double(x)
    return x * 2
end

result = [1, 2, 3].map(double)
```

### `filter`

Keeps elements for which the callback returns a truthy value.

### `any`

Returns true as soon as a callback returns a truthy value.

### `all`

Returns true only if every callback result is truthy. An empty array returns true.

---

# Part VII — Maps

## 42. Map literals

Map keys in a literal are identifiers and are stored as strings:

```lucy
user = {
    name: "Nima",
    age: 21
}
```

Map values can be any Lucy value.

Maps are mutable.

---

## 43. Map indexing

Map indexes must be strings:

```lucy
user = {name: "Nima"}
print user["name"]
```

A missing key returns `nil` through normal map indexing.

---

## 44. Map member syntax

A string key can also be accessed as a member:

```lucy
print user.name
user.city = "Birjand"
```

This is map key access, not a class method.

---

## 45. Map methods

| Method | Signature | Return |
|---|---|---|
| `get` | `get(key)` | value or `nil` |
| `set` | `set(key, value)` | stored value |
| `has` | `has(key)` | `bool` |
| `delete` | `delete(key)` | `bool` |
| `keys` | `keys()` | array of keys |
| `values` | `values()` | array of values |
| `clear` | `clear()` | `nil` |

Map keys supplied to these methods must be strings.

`delete` returns true if a key existed and was removed.

---

# Part VIII — Strings

## 46. String indexing

Strings can be indexed with integer positions:

```lucy
text = "Lucy"
print text[0]
print text[-1]
```

The current implementation indexes bytes rather than Unicode code points. `length()` is therefore also a byte count for UTF-8 text.

---

## 47. String methods

| Method | Signature | Return |
|---|---|---|
| `upper` | `upper()` | uppercase copy |
| `upcase` | `upcase()` | alias of `upper` |
| `lower` | `lower()` | lowercase copy |
| `downcase` | `downcase()` | alias of `lower` |
| `strip` | `strip()` | trimmed copy |
| `trim` | `trim()` | alias of `strip` |
| `contains` | `contains(text)` | `bool` |
| `starts_with` | `starts_with(text)` | `bool` |
| `ends_with` | `ends_with(text)` | `bool` |
| `length` | `length()` | byte length |
| `size` | `size()` | byte length |
| `reverse` | `reverse()` | reversed copy |
| `repeat` | `repeat(count)` | repeated string |
| `to_int` | `to_int()` | `int` |
| `to_float` | `to_float()` | `double` |
| `slice` | `slice(start, end)` | substring |
| `char_at` | `char_at(index)` | one-character string or `nil` |
| `split` | `split(separator)` | array |
| `replace` | `replace(from, to)` | replaced copy |

`reverse`, `upper`, `lower`, and related operations return new strings; they do not mutate the original string.

### `slice`

Returns the substring from `start` inclusive to `end` exclusive. `end` defaults to the end of the string when omitted. Negative indexes count from the end.

```lucy
text = "Lucy language"
print text.slice(0, 4)
```

### `char_at`

Returns a one-character string at an index, or `nil` when the index is invalid.

`split` rejects an empty separator.

`repeat` rejects a negative count.

`to_int` and `to_float` raise `ValueError` when conversion fails.

---

# Part IX — Numbers

## 48. Number methods

Both `int` and `double` values support these methods:

| Method | Signature | Return |
|---|---|---|
| `abs` | `abs()` | numeric absolute value |
| `floor` | `floor()` | `double` |
| `ceil` | `ceil()` | `double` |
| `round` | `round()` | `double` |
| `sqrt` | `sqrt()` | `double` |
| `sin` | `sin()` | `double` |
| `cos` | `cos()` | `double` |
| `tan` | `tan()` | `double` |
| `log` | `log()` | `double` |
| `to_int` | `to_int()` | `int` |
| `to_string` | `to_string()` | `text` |
| `pow` | `pow(exponent)` | `double` |

`sqrt()` rejects negative values.

`log()` requires a positive value.

---

# Part X — Functions as Values

## 49. First-class functions

A function can be assigned to a variable:

```lucy
def square(x)
    return x * x
end

operation = square
print operation(5)
```

Functions can be passed to methods such as `map`, `filter`, and `each`.

---

# Part XI — Classes and Objects

## 50. Defining a class

```lucy
class User
    def initialize(name)
        self.name = name
    end

    def greet()
        return "Hello " + self.name
    end
end
```

A class can contain methods. The current parser does not support arbitrary executable statements directly in a class body; class bodies contain method definitions.

---

## 51. Creating an instance

Use `.new(...)` on a class:

```lucy
user = User.new("Nima")
```

Lucy creates an instance and searches the class hierarchy for the nearest `initialize` method.

If `initialize` exists, it receives the constructor arguments with `self` bound to the new instance.

The constructor expression always returns the newly created instance. A value returned from `initialize` is not used as the constructor result.

---

## 52. Instance fields

Fields are normally assigned through `self`:

```lucy
self.name = "Lucy"
self.version = 11
```

They can then be read:

```lucy
print self.name
```

From outside a method:

```lucy
print user.name
```

Fields are dynamically created when assigned.

---

## 53. `self`

Inside an instance method, `self` refers to the current instance.

```lucy
def rename(name)
    self.name = name
end
```

Outside an instance method, `self` does not automatically refer to a global object.

---

## 54. Inheritance

A class may inherit from another class:

```lucy
class Animal
    def speak()
        return "animal"
    end
end

class Dog < Animal
    def speak()
        return "dog"
    end
end
```

Method lookup starts with the concrete class and then walks the parent chain.

Constructors also follow the inheritance chain: if the child has no `initialize`, the nearest parent initializer is used.

The `super` keyword is recognized by the lexer but full `super` call semantics are not currently part of the public runtime API.

---

# Part XII — Exceptions

## 55. Throwing an exception

```lucy
throw "something went wrong"
```

The current runtime reports this as:

```text
Exception: something went wrong
```

---

## 56. `try` and `catch`

```lucy
try
    value = 10 / 0
catch ZeroDivisionError as error
    print error
end
```

The caught variable contains the runtime error message as a string.

---

## 57. Catching a general exception

```lucy
try
    risky_operation()
catch Exception as error
    print error
end
```

An untyped `catch` also catches exceptions.

---

## 58. `finally`

`finally` runs after the protected block and after a matching catch block:

```lucy
try
    print "work"
catch Exception as error
    print error
finally
    print "cleanup"
end
```

If an exception is not caught, `finally` still runs before the exception continues outward.

---

## 59. Built-in error categories

The interpreter and standard library use error names such as:

| Error | Typical cause |
|---|---|
| `NameError` | unknown variable or constant assignment error |
| `TypeError` | incompatible value type |
| `ValueError` | invalid value or conversion |
| `ArgumentError` | wrong function argument usage |
| `IndexError` | invalid array/string index |
| `ZeroDivisionError` | division or modulo by zero |
| `AssertionError` | failed `assert` |
| `ImportError` | module cannot be loaded |
| `IOError` | filesystem or input/output failure |
| `ProcessError` | process execution failure |
| `RegexError` | invalid regular expression |
| `JSONDecodeError` | invalid JSON |
| `CSVError` | invalid CSV |
| `HTTPError` | HTTP client/curl failure |
| `SQLiteError` | SQLite failure |
| `LoopError` | loop safety limit exceeded |
| `ShellError` | backtick shell command failure |
| `OperatorError` | unsupported operation |
| `RuntimeError` | generic runtime failure |
| `Exception` | explicit `throw` or generic caught exception |

---

# Part XIII — Modules and Imports

## 60. Importing a module

```lucy
import math
print math.square(5)
```

Lucy loads a module from a `.lucy` file.

---

## 61. Import aliases

```lucy
import datetime as dt
print dt.now()
```

The alias becomes the module object visible in the importing environment.

---

## 62. Selective imports

Lucy also supports:

```lucy
from math import square
```

Multiple names are allowed:

```lucy
from math import square, cube
```

The imported names are placed directly in the current environment.

---

## 63. Module lookup order

When Lucy imports `name`, it searches in this order:

1. the directory of the current source file;
2. directories listed in `LUCY_PATH`;
3. `stdlib` under the current working directory;
4. the directory in `LUCY_STDLIB`;
5. `stdlib` next to the Lucy executable;
6. `../share/lucy/stdlib` relative to the executable on installed Unix layouts;
7. `/usr/local/share/lucy/stdlib` on non-Windows systems.

`LUCY_PATH` uses `:` on Unix-like systems and `;` on Windows.

---

## 64. Module objects

A normal `import` creates a module-like instance containing the module's exported values.

For example, `math.lucy` defines a `math` instance, so:

```lucy
import math
math.square(4)
```

works naturally.

Ordinary imports expose only public top-level functions through the module object. Classes and implementation objects stay private unless explicitly requested with `from module import Class`. Public calls are intentionally one level deep, such as `fs.read(path)` and `http.get(url)`.

Circular imports are rejected with `ImportError`.

---

# Part XIV — Shell Commands

## 65. Backticks

A command can be executed using backticks:

```lucy
output = `echo hello`
print output
```

The command's standard output is captured as a string. A trailing newline is removed.

A non-zero command status raises `ShellError`.

Backticks invoke the host shell. Do not place untrusted input into shell commands.

---

# Part XV — Built-in Functions

## 66. `print`

```lucy
print(value, ...)
```

Prints all arguments separated by spaces and adds a newline.

Returns `nil`.

```lucy
print "Lucy", 1.0, true
```

## 67. `puts`

```lucy
puts(value, ...)
```

Alias of `print`.

## 68. `input`

```lucy
input()
input(prompt)
```

Reads one line from standard input. With a prompt, the prompt is printed without an automatic newline.

Returns a string.

## 69. `len`

```lucy
len(value)
```

Returns the length of a string, array, or map.

For strings this is the current byte length.

## 70. `str`

```lucy
str(value)
```

Returns the value's display string.

## 71. `int`

```lucy
int(value)
```

Accepts an `int`, `double`, or numeric string.

Double-to-int conversion truncates toward zero using the C++ conversion semantics.

Invalid strings raise `ValueError`.

## 72. `float`

```lucy
float(value)
```

Accepts an `int`, `double`, or numeric string.

Invalid strings raise `ValueError`.

## 73. `type` and `typeof`

```lucy
type(value)
typeof(value)
```

Both return the same runtime type name.

## 74. `range`

```lucy
range(stop)
range(start, stop)
range(start, stop, step)
```

Returns an integer array.

The `stop` value is excluded.

```lucy
print range(5)
print range(2, 6)
print range(5, 0, -1)
```

A step of zero raises `ValueError`.

## 75. `sum`

```lucy
sum(array)
```

Sums numeric array elements.

The current implementation returns a numeric result using double accumulation.

## 76. `min`

```lucy
min(value, ...)
```

Returns the smallest numeric argument.

At least one argument is required.

## 77. `max`

```lucy
max(value, ...)
```

Returns the largest numeric argument.

At least one argument is required.

## 78. `abs`

```lucy
abs(number)
```

Returns the absolute value.

## 79. `sqrt`

```lucy
sqrt(number)
```

Returns the square root. Negative inputs raise `ValueError`.

## 80. `sin`, `cos`, `tan`

```lucy
sin(x)
cos(x)
tan(x)
```

Use the host C++ mathematical functions. Arguments are interpreted as floating-point numbers.

## 81. `exp`

```lucy
exp(x)
```

Returns `e` raised to the power `x`.

## 82. `floor` and `ceil`

```lucy
floor(x)
ceil(x)
```

Return floating-point results using the host math functions.

## 83. `log`

```lucy
log(x)
```

Returns the natural logarithm. `x` must be positive.

## 84. `pow`

```lucy
pow(base, exponent)
```

Returns a floating-point power result.

## 85. `assert`

```lucy
assert(condition)
assert(condition, message)
```

If the condition is falsey, `AssertionError` is raised.

## 86. Legacy filesystem helpers

These built-ins remain available for compatibility:

```lucy
read_file(path)
write_file(path, content)
exists(path)
cwd()
getenv(name)
```

The preferred public API is the `fs`, `fs`, `fs`, and `system` standard-library modules.

## 87. `sleep`

```lucy
sleep(milliseconds)
```

Pauses the current interpreter thread for the requested number of milliseconds.

## 88. `millis`

```lucy
millis()
```

Returns milliseconds elapsed from an internal monotonic clock origin created by the interpreter process.

It is not Unix time.

## 89. `random_int`

```lucy
random_int(low, high)
```

Returns a random integer in the inclusive range `[low, high]`.

The preferred public API is `random.integer`.

## 90. `help`

```lucy
from repl import help
help
help print
help "lambda"
help array.push
```

`help` is no longer a C++-owned documentation table. Its implementation lives in `stdlib/repl.lucy`, so the help text can be changed using Lucy. The parser only provides `help topic` syntax sugar; the documentation itself is not embedded in the interpreter.

The exhaustive implementation inventory is in `docs/API.md`.

---

# Part XVI — Global Constants and Runtime Values

## 91. `PI`

A read-only floating-point constant containing the mathematical constant pi.

## 92. `E`

A read-only floating-point constant containing Euler's number.

## 93. `VERSION`

A read-only string containing the Lucy interpreter version.

For this release:

```lucy
print VERSION
```

prints `1.0.1`.

## 94. `PLATFORM`

A read-only string describing the host platform.

Possible values in the current implementation include:

```text
windows
macos
linux
unix
```

## 95. `ARGV`

`ARGV` is a read-only binding to an array containing command-line arguments passed to the Lucy source file.

Example invocation:

```bash
lucy app.lucy one two 42
```

Lucy receives:

```lucy
print ARGV[0]
print ARGV[1]
print ARGV[2]
```

The values are all strings:

```text
one
two
42
```

The source filename itself is not included in `ARGV`.

## 96. `argv`

`argv` is the lowercase alias of `ARGV` and refers to the same underlying array.

Both names are provided so code can choose a conventional Ruby-style `ARGV` spelling or a lowercase `argv` spelling.

Both bindings are read-only at the variable-binding level, but the underlying array is mutable.

---

# Part XVII — Command-Line Interface

## 97. Running a script

```bash
lucy program.lucy
```

Arguments after the source file become `ARGV` values:

```bash
lucy program.lucy first second
```

## 98. REPL

Start the interactive interpreter with either:

```bash
lucy
```

or:

```bash
lucy -i
```

Lucy 1.0.1 provides a cross-platform interactive line editor on Windows, Linux, and macOS.

### Editing

The REPL supports cursor editing rather than simple line input:

- Up/Down: navigate persistent history.
- Left/Right: move the cursor.
- Home/End: move to the beginning/end of the line.
- Backspace: delete before the cursor.
- Delete: delete at the cursor.
- Tab: complete names and members.
- Ctrl-A / Ctrl-E: move to the beginning/end.
- Ctrl-K: delete from the cursor to the end.
- Ctrl-U: delete from the beginning to the cursor.
- Ctrl-D on an empty line: exit.

### Persistent history

History is stored in:

```text
Linux/macOS: ~/.lucy_history
Windows:     %USERPROFILE%\\.lucy_history
```

The editor keeps up to 1000 entries and avoids storing the same command repeatedly. History is available after restarting Lucy.

`:history` prints the entries currently loaded by the editor.

### Completion

Tab completion searches language keywords, built-in functions, globals, standard-library module names, module methods, array methods, string methods, map methods, and members of known user objects/classes.

For example:

```text
>>> pri<Tab>
>>> print

>>> import math
>>> math.sq<Tab>
>>> math.square
```

When multiple candidates have a common prefix, the first Tab expands the common prefix. A subsequent Tab prints matching candidates.

### Multiline blocks

Lucy block constructs can be entered over multiple lines. The prompt changes from `>>>` to `...` while a block is open:

```text
>>> def greet(name)
... print "Hello " + name
... end
```

The REPL recognizes `if`, `while`, `for`, `foreach`, `loop`, `function`, `def`, `class`, and `try` as block starters and `end` as the closing keyword.

### REPL commands

```text
:help
:history
:clear
:version
:quit
:exit
```

`:clear` clears the terminal and current input state. `:quit` and `:exit` leave the interactive interpreter.

When standard input is redirected, Lucy automatically uses normal line-based input instead of terminal editing, so pipelines and automated invocation remain usable.

# Part XVIII — Syntax Summary

## 99. Keywords

The lexer recognizes these language keywords:

```text
true false nil
if else while for foreach loop
function def class
return break continue end
import from as
const global in
and or not
new self super
try catch finally throw
```

Some recognized keywords are reserved for language evolution even though their complete semantics are not currently exposed. In particular, `super` does not yet provide full parent-method dispatch semantics.

---

## 100. Operators at a glance

```text
Arithmetic:
+ - * / % **

Equality:
== != === !==

Ordering:
> >= < <=

Logical:
and or not && || !

Bitwise:
& | ^ ~ << >>

Assignment:
= += -= *= /= %= **= &= |= ^= <<= >>=

Increment:
++ --

Membership:
in

Range:
.. ...

Ternary:
? :

Member/index/call:
. [] ()
```

---

# Part XIX — Programming Patterns

## 101. A small program

```lucy
import math
import fs

name = "Lucy"
numbers = [1, 2, 3, 4, 5]

def square_all(values)
    return values.map(math.square)
end

result = square_all(numbers)
file.write("result.txt", result.join(", "))

print "Hello $name"
print result
```

This example demonstrates imports, variables, arrays, functions, callbacks, a standard-library module, methods, interpolation, and file output.

---

# Part XX — Implementation Notes

## 102. Dynamic value representation

The interpreter stores runtime values as a tagged variant containing the supported value types. Arrays and maps are shared containers, so mutating an array or map through a reference changes the shared container.

## 103. Native boundary

Public standard-library modules are normally `.lucy` files. Native C++ functions are intentionally prefixed with `__` and are implementation details.

For example:

```text
regex.lucy
    -> __regex_match
    -> C++ std::regex
```

Application code should use `regex.match`, not `__regex_match`.

## 104. Current platform assumptions

The runtime uses C++17 facilities and host operating-system APIs. Some standard-library features depend on external software:

- HTTP uses the host `curl` executable.
- SQLite support is enabled when CMake finds SQLite3.
- filesystem behavior follows the host operating system.

---

# Part XXI — Recommended Learning Order

A first-time programmer should learn Lucy in this order:

1. values and variables;
2. strings and interpolation;
3. arrays and maps;
4. operators;
5. `if` and loops;
6. functions and return values;
7. default, named, and variadic arguments;
8. methods and callbacks;
9. classes and instances;
10. exceptions;
11. modules;
12. the standard library;
13. command-line applications.

A programmer coming from Python, Ruby, JavaScript, C++, or another language can usually skip the introductory examples and use the syntax tables and API sections directly.

## 1.0.1 additions

### Lambda expressions

Syntax:

```lucy
lambda(parameters) => expression
lambda parameter => expression
```

A lambda is a first-class callable value. It captures the current lexical environment and uses the same argument binding rules as named functions, including defaults.

### `switch`

Syntax:

```lucy
switch expression
case expression
    statements
default
    statements
end
```

Cases are compared using Lucy equality. Only the first matching case executes. `break` may be used to leave the selected case immediately.

### `do ... while`

Syntax:

```lucy
do
    statements
while expression
```

The body executes before the condition is evaluated.

### Semicolon separators

Statements may be separated by `;` instead of a newline:

```lucy
x = 1; y = 2; print x + y
```

### Standard-library import policy

Lucy does not use a language-level namespace feature. An ordinary `import module` creates a module object containing its public top-level functions. For example:

```lucy
import math
math.square(5)
```

Use an explicit selective import when a free function should enter the current scope:

```lucy
from math import square
square(5)
```

This prevents unrelated modules from overwriting names such as `get`, `post`, `cwd`, `parse`, and `run`.

Public classes retain their Ruby-style convenience names after import, so APIs such as `Set.new`, `Logger.new`, and `ERB.new` remain compatible.
