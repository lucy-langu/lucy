# `fs` standard library

Filesystem and file IO helpers.

## Import

```lucy
import fs
```

## Public functions

- `read(path)`
- `write(path, content)`
- `append(path, content)`
- `read_lines(path)`
- `write_lines(path, lines)`
- `exists(path)`
- `size(path)`
- `remove(path)`
- `copy(source, destination)`
- `move(source, destination)`
- `touch(path)`
- `chmod(path, mode)`
- `chown(path, uid, gid)`
- `entries(path = ".")`
- `files(path = ".")`
- `dirs(path = ".")`
- `mkdir(path, parents = false)`
- `rmdir(path)`
- `empty(path = ".")`
- `glob(pattern)`
- `walk(path = ".")`
- `join(parts)`
- `absolute(path)`
- `expand(path)`
- `basename(path)`
- `dirname(path)`
- `extension(path)`
- `stem(path)`
- `link(source, destination)`
- `symlink(source, destination)`
- `open(path, mode = "r")`
- `console(prompt = "")`
- `read_json(path)`
- `write_json(path, value)`

## Example

```lucy
import fs
fs.write "hello.txt", "Hello Lucy\n"
println fs.read "hello.txt"
```

## Source of truth

The public function list above is extracted from the current `stdlib/fs.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
