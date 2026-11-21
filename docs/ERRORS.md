# Lucy Errors

Lucy uses categorized syntax and runtime errors. The category is normally the first part of the message and is intended to tell you what kind of problem occurred.

## Error categories

| Category | Meaning |
|---|---|
| `SyntaxError` | The source code does not match Lucy syntax. |
| `NameError` | A variable, constant, or referenced name is unavailable or cannot be assigned. |
| `TypeError` | An operation received an incompatible value type. |
| `ArgumentError` | A function or method received invalid arguments. |
| `IndexError` | An array or string index is outside the valid range. |
| `ValueError` | A value is invalid for the requested operation or conversion. |
| `ZeroDivisionError` | Division or modulo used zero as the divisor. |
| `ImportError` | A module could not be resolved or a circular import was detected. |
| `IOError` | A filesystem or input/output operation failed. |
| `ShellError` | A backtick shell command failed. |
| `ProcessError` | An external process could not be started or returned a failure where capture requires success. |
| `AssertionError` | An `assert` condition was falsey. |
| `LoopError` | A loop exceeded the interpreter's safety iteration limit. |
| `NoMethodError` | A requested member or method does not exist. |
| `OperatorError` | An operator is not supported for the supplied values. |
| `RegexError` | A regular-expression pattern is invalid. |
| `JSONDecodeError` | JSON input is malformed. |
| `CSVError` | CSV input is malformed. |
| `HTTPError` | The HTTP transport failed. |
| `SQLiteError` | SQLite could not execute an operation or is unavailable in the build. |
| `RuntimeError` | A generic runtime failure occurred. |
| `Exception` | A language-level `throw` or generic exception category. |

## Syntax errors

Parser errors include a line and column when that information is available:

```text
SyntaxError: expected ')' after arguments at line 8, column 12
```

When debugging syntax, start at the reported location and then inspect the preceding expression or block delimiter. A missing `end`, `)`, `]`, or `}` often causes the visible error to appear later than the original mistake.

## Argument errors

Common causes include:

- missing required parameters;
- too many positional arguments;
- unknown named arguments;
- duplicate arguments;
- positional arguments after named arguments;
- a variadic parameter in the wrong position;
- a variadic parameter supplied by name.

## Runtime type errors

Use `type(value)` when unsure about a value:

```lucy
print type(value)
```

The result is one of Lucy's documented runtime type names.

## Exceptions and cleanup

Use `finally` for cleanup that must happen whether an operation succeeds or fails:

```lucy
try
    work()
finally
    cleanup()
end
```

## External commands

Backticks and `system` invoke host commands. Their behavior therefore depends on the operating system and installed command-line tools.

Never place untrusted data directly into shell command strings without appropriate validation and escaping.

## SQLite

SQLite support is optional at build time. If CMake cannot find SQLite3, the module remains present but its native operations report `SQLiteError` instead of silently pretending that a database exists.
