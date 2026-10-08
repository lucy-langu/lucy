# `system` standard library

Host process, environment, identity, signal, and shell helpers.

## Import

```lucy
import system
```

## Public functions

- `platform()`
- `version()`
- `argv()`
- `cwd()`
- `env(name)`
- `setenv(name, value)`
- `unsetenv(name)`
- `home()`
- `temp_dir()`
- `command_exists(command)`
- `pid()`
- `ppid()`
- `run(command)`
- `capture(command)`
- `success(command)`
- `output(command)`
- `spawn(command)`
- `wait(pid)`
- `waitpid(pid)`
- `kill(signal_number, pid)`
- `uid()`
- `gid()`
- `euid()`
- `egid()`
- `groups()`
- `clock_gettime(clock = "monotonic")`
- `trap(signal_number, callback)`
- `signals()`
- `signal_name(signal_number)`
- `login()`
- `user(name)`
- `user_id(uid_value)`
- `shell_split(text)`
- `shell_escape(text)`
- `shell_join(items)`

## Example

```lucy
import system
println system.platform()
println system.cwd()
```

## Source of truth

The public function list above is extracted from the current `stdlib/system.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
