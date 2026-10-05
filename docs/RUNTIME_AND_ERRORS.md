# Lucy 2.0.0 Runtime, Types, and Errors

## 1. Runtime model

Lucy uses a tree of environments and runtime `Value` objects. Source code is lexed into tokens, parsed into an AST, and interpreted by the runtime. The runtime also hosts the standard-library bridge, REPL behavior, and native extension boundary.

## 2. Runtime type contracts

Type annotations are executable contracts.

```lucy
let count: Int = 10

func add(a: Int, b: Int) -> Int {
    return a + b
}
```

The runtime checks:

- typed variable initialization;
- typed assignment;
- function arguments;
- function return values;
- typed struct fields.

Example failure:

```text
TypeError: variable 'count' expects Int, got string
TypeError: parameter 'a' expects Int, got string
TypeError: return value of function 'bad' expects Int, got string
```

## 3. Runtime type names

The current contract system recognizes built-in names such as `Int`, `Double`, `Float`, `Number`, `String`, `Bool`, `Nil`, `Array`, `Map`, `Function`, `Class`, `Instance`, `Native`, and `Any`, as well as user-defined class names.

Use `Any` when an API intentionally accepts any runtime value. Do not annotate everything as `Any`; precise contracts are more useful.

## 4. Undefined values and `??`

Normal missing-variable access raises `NameError`. The coalescing operator gives missing-name access a special fallback behavior:

```lucy
let name = nickname ?? "anonymous"
```

Only the left operand of `??` receives this missing-value behavior.

## 5. Error classes

Common diagnostics include:

- `SyntaxError` — source cannot be parsed.
- `NameError` — a normal variable/name lookup failed.
- `TypeError` — a value has the wrong runtime type.
- `ArgumentError` — a call has an invalid argument count or shape.
- `IndexError` — an index is invalid for the requested operation.
- `ValueError` — a value is outside the accepted domain or cannot be converted.
- `IOError` — filesystem or stream access failed.
- `ImportError` — module loading failed.
- library-specific socket, HTTP, SQLite, or native errors where appropriate.

## 6. Import errors

Module failures are normalized through an `ImportError` wrapper:

```text
ImportError: module 'example': ...
```

This gives callers a stable category while retaining the underlying reason in the message.

## 7. Script versus REPL failure

In script mode, an uncaught runtime error is reported and the process exits with a non-zero status. In the REPL, the diagnostic is printed and the interactive environment attempts to continue.

## 8. Exceptions

Use `try`, `catch`, `finally`, and `throw` for exceptional control flow:

```lucy
try {
    risky()
} catch error {
    println error
} finally {
    cleanup()
}
```

The `runtime` module also contains callback-oriented exception helpers such as `catch`, `rescue`, and `ensure`.

## 9. Exit status

`exit(code)` terminates the process. Use zero for successful explicit termination and a non-zero code for failure conditions.

```lucy
if invalid {
    exit 2
}
```

## 10. Numeric conversion errors

Base conversion functions accept an optional non-negative width:

```lucy
hex(15, 4)       # 000F
```

A negative width raises `ValueError`. Invalid numeric strings passed to conversion APIs raise `ValueError` rather than returning arbitrary data.

## 11. Indexing rules

Array and string indexes support negative values. Out-of-range direct indexing raises the implementation's index error behavior. Range slicing clamps its endpoints according to the current runtime rules.

For arrays:

```lucy
let values = [1, 2, 3]
println values[-1]
println values[0..2]
```

## 12. Error-writing rules for libraries

A public library should:

1. validate argument count early;
2. report the operation name in `ArgumentError`/`ValueError` messages;
3. preserve useful native failure information;
4. avoid exposing internal C++ stack details;
5. document whether failure is an exception or a `result` value.

This makes errors useful to both the kernel/runtime and application developers.
