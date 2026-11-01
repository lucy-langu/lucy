# `system` standard library — Lucy 2.0.7

Platform/environment inspection, argv-based process execution, identity, signals, clocks, and shell helpers.

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
let result = system.exec(["cmake", "--version"], {"timeout_ms": 5000})
println result.get("stdout")
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
| `exec` | `exec(argv, options = {})` |
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

### Direct process execution

`system.exec(argv, options = {})` starts a process without a command shell. Pass the executable and each argument as a separate string. Supported options are `cwd` and `timeout_ms`.

```lucy
let result = system.exec(["cmake", "--build", "build"], {"cwd": ".", "timeout_ms": 120000})
if !result.get("success") {
    println result.get("stderr")
    exit(1)
}
println result.get("stdout")
```

The result map contains `stdout`, `stderr`, `status`, `success`, `timed_out`, and `pid`. Exit status `124` denotes a timeout. Prefer this API for package managers, build systems, test runners, and editor tooling. `system.run` remains available for compatibility and shell-specific workflows.

## Source of truth

Implementation: [`stdlib/system.lucy`](../../stdlib/system.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
