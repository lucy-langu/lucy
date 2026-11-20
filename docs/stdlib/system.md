# `system` standard library — Lucy 2.0.7 · module 1.1.0

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
| `env_present` | `env_present(name)` |
| `env_or` | `env_or(name, fallback)` |
| `env_bool` | `env_bool(name, fallback = false)` |
| `env_int` | `env_int(name, fallback = 0)` |
| `env_float` | `env_float(name, fallback = 0.0)` |
| `setenv_if_missing` | `setenv_if_missing(name, value)` |
| `is_windows` | `is_windows()` |
| `is_linux` | `is_linux()` |
| `is_macos` | `is_macos()` |
| `is_unix` | `is_unix()` |
| `require_command` | `require_command(command)` |
| `exec_success` | `exec_success(argv, options = {})` |
| `exec_status` | `exec_status(argv, options = {})` |
| `exec_stdout` | `exec_stdout(argv, options = {})` |
| `exec_stderr` | `exec_stderr(argv, options = {})` |
| `exec_checked` | `exec_checked(argv, options = {})` |
| `exec_text` | `exec_text(argv, options = {})` |
| `exec_first_line` | `exec_first_line(argv, options = {})` |
| `exec_lines` | `exec_lines(argv, options = {})` |
| `exec_json` | `exec_json(argv, options = {})` |
| `exec_or` | `exec_or(argv, fallback, options = {})` |
| `process_info` | `process_info()` |

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

## Environment and process convenience helpers

These are Lucy-level compositions over the stable environment and argv-based process APIs.

| Function | Purpose |
|---|---|
| `env_present(name)` | Check whether an environment variable is set and non-empty. |
| `env_or(name, fallback)` | Return a non-empty environment value or fallback. |
| `env_bool(name, fallback = false)` | Parse common true/false environment spellings. |
| `env_int(name, fallback = 0)` | Parse an integer environment value or fallback. |
| `env_float(name, fallback = 0.0)` | Parse a floating-point environment value or fallback. |
| `setenv_if_missing(name, value)` | Set a variable only when it is currently absent/empty. |
| `is_windows()` | Check the runtime platform string for Windows. |
| `is_linux()` | Check the runtime platform string for Linux. |
| `is_macos()` | Check for Darwin/macOS platform strings. |
| `is_unix()` | True for non-Windows platforms. |
| `require_command(command)` | Throw a clear error when an executable is not on PATH. |
| `exec_success(argv, options = {})` | Return the process success flag. |
| `exec_status(argv, options = {})` | Return the process exit status. |
| `exec_stdout(argv, options = {})` | Return standard output. |
| `exec_stderr(argv, options = {})` | Return standard error. |
| `exec_checked(argv, options = {})` | Return the result or throw on non-zero/failed execution. |
| `exec_text(argv, options = {})` | Convenience alias for captured standard output. |
| `exec_first_line(argv, options = {})` | Return the first output line, or an empty string. |
| `exec_lines(argv, options = {})` | Return non-empty output lines. |
| `exec_json(argv, options = {})` | Parse successful process stdout as JSON. |
| `exec_or(argv, fallback, options = {})` | Return stdout on success or fallback on failure. |
| `process_info()` | Return pid, ppid, uid, gid, platform, and cwd. |

Prefer `exec(argv, options)` over shell-string execution. `exec_checked` reports the captured diagnostic but does not retry or suppress process failures.
