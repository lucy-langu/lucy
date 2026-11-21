# `fs` standard library — Lucy 2.0.7 · module 1.1.0

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
| `replace` | `replace(source, destination)` |
| `touch` | `touch(path)` |
| `chmod` | `chmod(path, mode)` |
| `chown` | `chown(path, uid, gid)` |
| `chdir` | `chdir(path)` |
| `entries` | `entries(path = ".")` |
| `files` | `files(path = ".")` |
| `dirs` | `dirs(path = ".")` |
| `mkdir` | `mkdir(path, parents = false)` |
| `rmdir` | `rmdir(path, recursive = false)` |
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
| `is_dir` | `is_dir(path)` |
| `is_file` | `is_file(path)` |
| `ensure_dir` | `ensure_dir(path)` |
| `ensure_parent` | `ensure_parent(path)` |
| `read_or` | `read_or(path, fallback = "")` |
| `write_if_changed` | `write_if_changed(path, content)` |
| `append_line` | `append_line(path, line)` |
| `touch_if_missing` | `touch_if_missing(path)` |
| `remove_if_exists` | `remove_if_exists(path)` |
| `copy_if_exists` | `copy_if_exists(source, destination)` |
| `move_if_exists` | `move_if_exists(source, destination)` |
| `read_json_or` | `read_json_or(path, fallback = nil)` |
| `write_json_pretty` | `write_json_pretty(path, value)` |
| `list_files` | `list_files(path = ".")` |
| `list_dirs` | `list_dirs(path = ".")` |
| `file_count` | `file_count(path = ".")` |
| `dir_count` | `dir_count(path = ".")` |
| `with_extension` | `with_extension(path, extension_value)` |
| `is_absolute` | `is_absolute(path)` |
| `is_hidden` | `is_hidden(path)` |
| `same_path` | `same_path(left, right)` |
| `read_lines_or` | `read_lines_or(path, fallback = [])` |

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

## Project and application helpers

The following additive helpers are implemented in Lucy and do not introduce new native runtime primitives.

| Function | Purpose |
|---|---|
| `is_dir(path)` | True when the path is a directory. |
| `is_file(path)` | True when the path is a regular file. |
| `ensure_dir(path)` | Create a directory tree if absent; return its absolute path. |
| `ensure_parent(path)` | Create the parent directory for a file path; return the absolute file path. |
| `read_or(path, fallback = "")` | Read a file or return a fallback when it is absent. |
| `write_if_changed(path, content)` | Write only when contents differ; return whether a write occurred. |
| `append_line(path, line)` | Append one newline-terminated line. |
| `touch_if_missing(path)` | Create a missing file; return whether it was created. |
| `remove_if_exists(path)` | Remove a file or directory tree if present. |
| `copy_if_exists(source, destination)` | Copy an existing regular file; return false when absent. |
| `move_if_exists(source, destination)` | Move an existing entry; return false when absent. |
| `read_json_or(path, fallback = nil)` | Parse JSON or return fallback on missing/invalid input. |
| `write_json_pretty(path, value)` | Write pretty JSON followed by a newline. |
| `list_files(path = ".")` | Return sorted direct child file names. |
| `list_dirs(path = ".")` | Return sorted direct child directory names. |
| `file_count(path = ".")` | Count direct child files. |
| `dir_count(path = ".")` | Count direct child directories. |
| `with_extension(path, extension)` | Replace the filename extension. |
| `is_absolute(path)` | Detect POSIX-rooted and Windows-drive paths. |
| `is_hidden(path)` | Check whether the basename begins with `.`. |
| `same_path(left, right)` | Compare absolute path spellings. |
| `read_lines_or(path, fallback = [])` | Read lines or return fallback when absent. |

`read_json_or` intentionally treats parse failures as fallback. Use `read_json` when malformed JSON must be reported.
