# Lucy Architecture

## 1. Runtime pipeline

```text
Lucy source
    |
    v
Lexer
    |
    v
Token stream
    |
    v
Recursive-descent Parser
    |
    v
AST
    |
    v
Interpreter
    |
    +------------------+
    |                  |
    v                  v
Value/Object model   Native C++ boundary
                       |
                       +-- filesystem
                       +-- operating system
                       +-- regex
                       +-- process/shell
                       +-- HTTP via curl
                       +-- SQLite
```

## 2. Lexer

The lexer recognizes:

- identifiers;
- integer and floating-point literals;
- single- and double-quoted strings;
- backtick shell expressions;
- comments;
- keywords;
- operators;
- arrays and maps;
- ranges;
- calls, indexing, and member access.

Every token stores a source line and column.

## 3. Parser

The parser is a recursive-descent parser. Operator precedence is represented by separate parsing stages, from logical operators down to unary and postfix expressions.

Statements use newline boundaries and `end` for blocks rather than semicolons.

## 4. Values

`Value` is a tagged `std::variant` containing:

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

Arrays and maps are stored through shared pointers, which allows mutations to remain visible through multiple references.

## 5. Environments and scope

An `Environment` stores named bindings and points to a parent environment.

Control-flow blocks create child environments. Assignment searches the current environment and then its parents, so assigning an existing outer variable updates that binding while a new name becomes local to the current block.

Functions capture their defining environment, providing lexical closures.

## 6. Functions

A function contains:

- its name;
- parameter definitions;
- default expressions;
- variadic information;
- its body;
- its closure environment;
- an optional bound `self`;
- an optional native C++ callback.

Calls validate positional, named, default, and variadic arguments before executing the body.

## 7. Classes and instances

A `Class` stores methods and an optional parent class. An `Instance` stores its class and mutable fields.

`Class.new(...)` allocates an instance, finds the nearest `initialize` method in the inheritance chain, binds `self`, invokes the initializer, and returns the newly allocated instance.

Method lookup checks instance fields first and then walks the class hierarchy.

## 8. Exceptions

C++ exceptions are used internally for non-local control flow and runtime failures. Lucy exposes structured `try`, `catch`, `finally`, and `throw` statements.

Loop control also uses internal exception signals for `break` and `continue`.

## 9. Modules

Modules are `.lucy` files. They are parsed and executed in a child environment, then their exported values are exposed to the importer.

The importer supports:

```lucy
import module
import module as alias
from module import name
from module import name, other
```

The runtime detects circular imports.

## 10. Standard-library architecture

Lucy deliberately keeps public high-level APIs in `stdlib/*.lucy`.

Example:

```text
regex.match(...)
      |
      v
stdlib/regex.lucy
      |
      v
__regex_match(...)
      |
      v
C++ std::regex
```

This makes standard-library APIs easy to read and modify without moving high-level behavior into the interpreter.

## 11. Native boundary

Native functions use an internal `__name` convention. These functions are implementation details and are not the public API.

Native code is used when Lucy needs:

- operating-system APIs;
- filesystem access;
- process APIs;
- native regular-expression support;
- external `curl` transport;
- SQLite3;
- efficient low-level operations.

## 12. Command-line arguments

The command-line entry point collects arguments after the `.lucy` source filename and passes them into the `Interpreter`.

The interpreter creates one shared array and exposes it through:

```text
ARGV
argv
```

The `system` module returns the same array through `sys.argv()`.

## 13. Module resolution

Module resolution is intentionally relocatable. The runtime checks source-relative paths, `LUCY_PATH`, local `stdlib`, `LUCY_STDLIB`, executable-relative locations, and installed Unix locations.

This lets a Lucy distribution move without baking an absolute standard-library path into the executable.

## 14. Memory ownership

The current runtime relies primarily on `std::shared_ptr` ownership for AST nodes, arrays, maps, functions, classes, instances, and environments.

Closures and environments are therefore an important ownership boundary. The interpreter explicitly clears tracked environments during destruction to reduce retained reference cycles.

A tracing garbage collector is not part of the current runtime.

## 15. Development philosophy

Lucy is intentionally keeping the interpreter small while the language contracts stabilize.

The preferred implementation rule is:

> Keep high-level behavior in Lucy. Keep native C++ primitives small and focused.

That separation is especially important for the standard library and for a future runtime/VM implementation.
