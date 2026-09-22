# Lucy Language Guide

This file is the short language-oriented entry point. For the complete specification of the current interpreter, read [`REFERENCE.md`](REFERENCE.md).

## Quick syntax map

```lucy
# Variables
name = "Lucy"
count = 3

# Constants
const VERSION_NAME = "1.0.1"

# Conditions
if count > 0
    print "positive"
else
    print "zero or negative"
end

# Functions
def add(a, b = 0)
    return a + b
end

# Arrays
items = [1, 2, 3]
items.push 4

# Maps
user = {name: "Nima", age: 21}
print user.name

# Loops
for item in items
    print item
end

# Classes
class User
    def initialize(name)
        self.name = name
    end
end

user = User.new("Nima")

# Exceptions
try
    print user.name
catch Exception as error
    print error
finally
    print "done"
end

# Modules
import data
import data
data = data.parse("{\"name\":\"Lucy\"}")
```

## Core types

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

## Core control flow

```text
if / else if / else / end
while / end
for / in / end
foreach / in / end
loop / end
break
continue
```

## Functions

```text
def name(parameters)
function name(parameters)
return value
name(arguments)
name positional, arguments
name(parameter: value)
def name(required, optional = value, *rest)
```

Parenthesized calls, arrays, maps, lambda parameter lists, and function parameter lists may span multiple lines. Newlines inside these delimiters are formatting and do not terminate the expression.

```lucy
text(
    "hello",
    name,
    end
)

items = [
    1,
    2,
    3
]

user = {
    name: "Nima",
    version: 1
}

def add(
    a,
    b
)
    return a + b
end
```

## Operators

```text
+ - * / % **
== != === !==
> >= < <=
and or not && || !
& | ^ ~ << >>
= += -= *= /= %= **= &= |= ^= <<= >>=
++ --
in
.. ...
? :
```

## Collections

Arrays are zero-based and mutable. Maps use string keys. Both support member methods documented in `REFERENCE.md`.

## Strings

Both `'single quotes'` and `"double quotes"` are supported. `$name` interpolation works in both.

## Modules

```lucy
import module
import module as alias
from module import name
from module import name, other
```

## Command-line arguments

```lucy
print ARGV
print argv
```

The source filename is not included in the array.

## Where to continue

- [`TUTORIAL.md`](TUTORIAL.md) — learn by building small programs.
- [`REFERENCE.md`](REFERENCE.md) — exact language semantics and every built-in.
- [`STDLIB.md`](STDLIB.md) — every standard-library module and public API.
- [`CLI.md`](CLI.md) — command-line and REPL behavior.

## 1.0.1 language additions

### Semicolon statement separators

A semicolon separates statements on the same physical line. Newlines remain valid separators.

```lucy
x = 1; y = 2; print x + y
```

Semicolons are separators, not expression operators. They are also accepted between statements inside blocks.

### Lambda expressions

Lucy supports closure-capturing expression lambdas:

```lucy
add = lambda(x, y) => x + y
print add(2, 3)
```

A single parameter may omit parentheses:

```lucy
square = lambda x => x * x
```

Default parameters are allowed:

```lucy
increment = lambda(x, amount = 1) => x + amount
```

The lambda captures the lexical environment in which it is created and is callable like a normal Lucy function.

### switch / case / default

`switch` compares its value against each `case`. The first matching case executes. `default` executes when no case matches.

```lucy
switch status
case 200
    print "ok"
case 404
    print "not found"
default
    print "other"
end
```

`break` exits the switch. Without `break`, Lucy still executes only the selected case; it does not perform implicit fall-through.

### do ... while

`do` executes its body before checking the condition, so the body runs at least once.

```lucy
i = 0
do
    i += 1
while i < 3
```

### Time and Date operators

`Time` supports `+`, `-`, and `<=>` with millisecond offsets or another `Time`. `Date` uses day offsets for `+` and `-`, and `<=>` compares timestamps.

```lucy
tomorrow = time.now() + 86400000
difference = tomorrow - time.now()
comparison = time.now() <=> tomorrow
```

### Set operators

`Set` supports:

- `a | b` — union
- `a & b` — intersection
- `a ^ b` — symmetric difference
- `a - b` — difference

The predicate methods `subset?`, `superset?`, and `intersect?` are also available.
