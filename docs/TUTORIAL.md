# Learn Lucy

This tutorial is designed for someone who may be learning programming for the first time, while still moving quickly enough for a developer coming from another language.

For exact signatures and every API detail, use `docs/REFERENCE.md` and `docs/STDLIB.md`.

---

# 1. Your first program

Create `hello.lucy`:

```lucy
print "Hello, Lucy!"
```

Run:

```bash
lucy hello.lucy
```

`print` writes text to the terminal.

---

# 2. Variables

```lucy
name = "Nima"
age = 21

print name
print age
```

A variable is simply a name connected to a value.

Lucy is dynamically typed, so the same variable can later contain another kind of value:

```lucy
value = 10
value = "ten"
```

---

# 3. Strings and interpolation

```lucy
name = "Lucy"
version = 0.11

print "Language: $name"
print "Version: $version"
```

The `$name` part is replaced by the value of `name`.

---

# 4. Decisions

```lucy
age = 21

if age >= 18
    print "adult"
else
    print "minor"
end
```

Lucy uses `end` to close blocks.

Multiple conditions use `else if`:

```lucy
score = 85

if score >= 90
    print "A"
else if score >= 80
    print "B"
else
    print "C"
end
```

---

# 5. Arrays

An array stores an ordered group of values:

```lucy
languages = ["Lucy", "Python", "Ruby"]

print languages[0]
print languages.length()
```

Indexes start at zero.

Add an item:

```lucy
languages.push "C++"
```

Loop over the array:

```lucy
for language in languages
    print language
end
```

---

# 6. Maps

A map stores named values:

```lucy
user = {
    name: "Nima",
    age: 21
}

print user.name
print user["age"]
```

Change a value:

```lucy
user.age = 22
```

---

# 7. Loops

A `while` loop repeats while a condition is true:

```lucy
count = 0

while count < 5
    print count
    count += 1
end
```

Use `break` to stop early:

```lucy
for i in 1..10
    if i == 5
        break
    end
    print i
end
```

---

# 8. Functions

Functions let you package reusable logic:

```lucy
def greet(name)
    return "Hello " + name
end

print greet("Nima")
```

Default arguments:

```lucy
def greet(name, greeting = "Hello")
    return greeting + " " + name
end
```

Named arguments:

```lucy
print greet(name: "Nima", greeting: "Hi")
```

Variadic arguments:

```lucy
def total(*numbers)
    return sum(numbers)
end

print total(1, 2, 3, 4)
```

---

# 9. Methods

Lucy lets values expose useful operations through methods:

```lucy
text = "hello"
print text.upper()

items = [1, 2, 3]
items.push 4
```

Both forms are possible when the syntax is unambiguous:

```lucy
items.push(4)
items.push 4
```

---

# 10. Functions as callbacks

Functions can be passed to other functions.

```lucy
def double(value)
    return value * 2
end

numbers = [1, 2, 3]
result = numbers.map(double)

print result
```

This is one of the most useful patterns for larger programs.

---

# 11. Classes

A class defines the behavior of objects:

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

Create an object:

```lucy
user = User.new("Nima")
print user.greet()
```

Inheritance:

```lucy
class Admin < User
    def role()
        return "admin"
    end
end
```

---

# 12. Errors

Protect risky code with `try` and `catch`:

```lucy
try
    number = int("abc")
catch ValueError as error
    print error
end
```

Always use the narrowest useful error category when practical.

---

# 13. Modules

Lucy code can be split into multiple files.

Suppose `helpers.lucy` contains:

```lucy
def greet(name)
    return "Hello " + name
end
```

Another file can use:

```lucy
import helpers

print helpers.greet("Nima")
```

You can also use selective imports:

```lucy
from helpers import greet
print greet("Nima")
```

---

# 14. Reading files

```lucy
import fs

import fs
fs.write("notes.txt", "Hello from Lucy")
text = fs.read("notes.txt")
print text
```

Read lines:

```lucy
lines = fs.read_lines("notes.txt")
```

---

# 15. JSON

```lucy
import data

import data
value = data.parse("{\"name\":\"Lucy\",\"version\":11}")
print data.name
```

Serialize Lucy data:

```lucy
payload = {
    name: "Lucy",
    version: 11
}

print data.stringify(payload)
```

---

# 16. Regular expressions

```lucy
import text

if regex.match("^[a-z]+$", "lucy")
    print "valid"
end
```

Find all matches:

```lucy
numbers = regex.find_all("[0-9]+", "a12 b34")
print numbers
```

---

# 17. HTTP

```lucy
import http

response = http.get("https://example.com")

print response.status
print response.body
```

The HTTP implementation uses the host `curl` executable.

---

# 18. SQLite

```lucy
import sqlite

db = sqlite.open("app.db")
db.execute("CREATE TABLE IF NOT EXISTS users (name TEXT, age INTEGER)")
db.execute("INSERT INTO users VALUES ('Nima', 21)")

rows = db.query("SELECT name, age FROM users")
print rows[0].name
```

---

# 19. Command-line programs

Lucy programs can receive arguments:

```bash
lucy greet.lucy Nima
```

Inside `greet.lucy`:

```lucy
name = ARGV[0]
print "Hello $name"
```

`ARGV` is an array of strings. `system.argv()` provides the standard-library accessor.

---

# 20. A small real application

```lucy
import data
import fs
import fs

config_path = "config.json"

import fs
if !fs.exists(config_path)
    fs.write(config_path, "{\"name\":\"Lucy\"}")
end

import data
config = data.parse(fs.read(config_path))

print "Application: $config.name"
print "Config: " + fs.absolute(config_path)
```

This is already the beginning of the kind of structure you can use for real command-line applications.

---

# 21. Where to go next

After this tutorial, learn these sections in order:

1. `docs/REFERENCE.md` — complete language rules;
2. `docs/STDLIB.md` — complete library API;
3. `docs/CLI.md` — command-line behavior;
4. `docs/ARCHITECTURE.md` — how the interpreter works if you want to contribute to Lucy itself.
