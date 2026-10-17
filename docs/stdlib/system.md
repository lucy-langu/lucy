# `system` standard library — Lucy 2.0.1

Platform/environment inspection, process execution, identity, signals, clocks, and shell helpers.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import system
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println system.platform()
println system.cwd()
println system.env("HOME")
```

## Module-level functions

| Function | Signature |
|---|---|
| `platform` | `platform()` |
| `version` | `version()` |
| `argv` | `argv()` |
| `cwd` | `cwd()` |
| `env` | `env(name)` |
| `setenv` | `setenv(name, value)` |
| `unsetenv` | `unsetenv(name)` |
| `home` | `home()` |
| `temp_dir` | `temp_dir()` |
| `command_exists` | `command_exists(command)` |
| `pid` | `pid()` |
| `ppid` | `ppid()` |
| `run` | `run(command)` |
| `capture` | `capture(command)` |
| `success` | `success(command)` |
| `output` | `output(command)` |
| `spawn` | `spawn(command)` |
| `wait` | `wait(pid)` |
| `waitpid` | `waitpid(pid)` |
| `kill` | `kill(signal_number, pid)` |
| `uid` | `uid()` |
| `gid` | `gid()` |
| `euid` | `euid()` |
| `egid` | `egid()` |
| `groups` | `groups()` |
| `clock_gettime` | `clock_gettime(clock = "monotonic")` |
| `trap` | `trap(signal_number, callback)` |
| `signals` | `signals()` |
| `signal_name` | `signal_name(signal_number)` |
| `login` | `login()` |
| `user` | `user(name)` |
| `user_id` | `user_id(uid_value)` |
| `shell_split` | `shell_split(text)` |
| `shell_escape` | `shell_escape(text)` |
| `shell_join` | `shell_join(items)` |

## Classes and methods

No public class declarations were found in this module's pure-Lucy source.

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/system.lucy`](../../stdlib/system.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
