# `fs` standard library — Lucy 2.0.1

Filesystem operations, path helpers, text/JSON convenience methods, and IO objects.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import fs
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
fs.write("example.txt", "hello\n")
fs.append("example.txt", "Lucy\n")
println fs.read("example.txt")
println fs.basename("src/lucy.cpp")
```

## Module-level functions

| Function | Signature |
|---|---|
| `read` | `read(path)` |
| `write` | `write(path, content)` |
| `append` | `append(path, content)` |
| `read_lines` | `read_lines(path)` |
| `write_lines` | `write_lines(path, lines)` |
| `exists` | `exists(path)` |
| `size` | `size(path)` |
| `remove` | `remove(path)` |
| `copy` | `copy(source, destination)` |
| `move` | `move(source, destination)` |
| `touch` | `touch(path)` |
| `chmod` | `chmod(path, mode)` |
| `chown` | `chown(path, uid, gid)` |
| `entries` | `entries(path = ".")` |
| `files` | `files(path = ".")` |
| `dirs` | `dirs(path = ".")` |
| `mkdir` | `mkdir(path, parents = false)` |
| `rmdir` | `rmdir(path)` |
| `empty` | `empty(path = ".")` |
| `glob` | `glob(pattern)` |
| `walk` | `walk(path = ".")` |
| `join` | `join(parts)` |
| `absolute` | `absolute(path)` |
| `expand` | `expand(path)` |
| `basename` | `basename(path)` |
| `dirname` | `dirname(path)` |
| `extension` | `extension(path)` |
| `stem` | `stem(path)` |
| `link` | `link(source, destination)` |
| `symlink` | `symlink(source, destination)` |
| `open` | `open(path, mode = "r")` |
| `console` | `console(prompt = "")` |
| `read_json` | `read_json(path)` |
| `write_json` | `write_json(path, value)` |

## Classes and methods

### `IO`

| Method | Signature |
|---|---|
| `initialize` | `initialize(path = nil, mode = "r")` |
| `print` | `print(values)` |
| `write` | `write(text)` |
| `read` | `read(prompt = "")` |
| `ask` | `ask(prompt)` |
| `seek` | `seek(offset, whence = 0)` |
| `pos` | `pos()` |
| `rewind` | `rewind()` |
| `eof` | `eof()` |
| `fileno` | `fileno()` |
| `close` | `close()` |
| `pipe` | `pipe()` |
| `popen` | `popen(command)` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/fs.lucy`](../../stdlib/fs.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
