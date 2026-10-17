# Lucy 2.0.1 — REPL, CLI, Tooling, Packages, and Developer Workflow

This document is the operational manual for using Lucy every day. It combines the former REPL, CLI, package, testing, editor, limits, and tooling references so users do not have to search through small disconnected files.

## 1. CLI

The current CLI forms are:

```text
lucy <file.lucy> [arguments...]
lucy -i
lucy --interactive
lucy -h
lucy --help
lucy -v
lucy --version
```

With no program argument, Lucy starts the REPL.

Command-line arguments are available as both `ARGV` and `argv` in Lucy code.

There is no documented `lucy -e` or `lucy -c` execution mode in 2.0.1. Do not assume those flags exist.

An uncaught script error is reported by the CLI and results in a non-zero process status. In the REPL, errors are printed and the interactive session continues when possible.

## 2. The REPL

Start it with:

```text
lucy
```

The default prompt is provided by the editable `stdlib/repl.lucy` library. The native editor can fall back to a native prompt if that library cannot be loaded.

### REPL commands

| Command | Action |
|---|---|
| `:help` | Show REPL help |
| `:clear` | Clear the terminal and current input buffer |
| `:history` | Show recent history |
| `:version` | Show Lucy version |
| `:quit` | Exit |
| `:exit` | Exit |
| `Ctrl-D` | Exit when the input buffer is empty |

There is no separate `:history` topic parser or large colon-command language. The commands above are handled by the native REPL.

### Language help

The REPL loads `repl.lucy` and exposes:

```lucy
help()
help "array"
help "modules"
help "exceptions"
repl.commands()
repl.topics()
```

The help system is itself Lucy code, which means its user-facing wording can evolve without changing the C++ interpreter.

## 3. Completion

Completion is a runtime feature, not a static list of documentation names.

### 3.1 What can be completed

At the top level, completion includes:

- language keywords;
- global builtins;
- builtin type classes;
- variables/functions/classes in the current environment and parent environments;
- module names exposed in the environment.

After `.`, completion resolves the actual runtime object and lists:

- module fields and public functions;
- class methods and inherited methods;
- instance fields and methods;
- array methods;
- string methods;
- map methods;
- number methods;
- members of chained expressions when the intermediate value can be resolved without executing arbitrary user code.

Examples:

```text
Lucy$ fs.<Tab>
Lucy$ text.u<Tab>
Lucy$ values.<Tab>
Lucy$ point.x.to_<Tab>
```

### 3.2 Tab behavior

- If there is one candidate, the first Tab completes it immediately.
- If there are multiple candidates, the first Tab completes the longest common prefix and prints the candidate list.
- Subsequent Tabs cycle through candidates.
- Candidates are sorted and deduplicated.
- Completion never intentionally executes arbitrary user expressions.
- Moving the cursor, editing the line, or navigating history resets the completion cycle.

### 3.3 Why completion is runtime-driven

Lucy ships a sizeable standard library. A manually maintained completion list inevitably becomes stale. The runtime now treats module objects and live environments as the source of completion candidates. This means a new public function added to a Pure-Lucy module becomes discoverable without adding another C++ completion entry.

Public classes/constants/helper objects exported by a module are also completable. Native extensions that expose members through the normal module/instance representation participate in the same completion path.

## 4. Line editing

The interactive editor supports:

| Key | Behavior |
|---|---|
| Up / Down | history navigation |
| Left / Right | cursor movement |
| Home / End | line boundaries |
| Delete / Backspace | character editing |
| Ctrl-A / Ctrl-E | beginning/end |
| Ctrl-K | delete to end |
| Ctrl-U | delete to beginning |
| Tab | completion |
| Ctrl-D | exit on empty input |

The editor stores up to 1000 history entries.

## 5. History files and platforms

Windows uses:

```text
%USERPROFILE%\\.lucy_history
```

Unix-like systems use:

```text
$HOME/.lucy_history
```

The editor uses platform-native terminal APIs:

- Windows: console input and Win32 console configuration.
- Linux/macOS and other POSIX systems: `termios`/TTY handling.

When stdin is not an interactive terminal, Lucy falls back to ordinary line-oriented input. This is important for scripts, CI, pipes, and redirected input.

## 6. Multiline REPL input

The REPL tracks brace depth and changes from the primary prompt to the continuation prompt while a block remains open:

```text
Lucy$ func greet(name) {
...     println "Hello, $name"
... }
```

Input is then parsed as one buffer. A syntax error does not permanently damage the next REPL prompt; the current buffer is reset after the attempt.

## 7. Packages

The runtime recognizes Lucy package/source layouts around `src/init.lucy`. Package resolution is part of module resolution; however, 2.0.1 does not enforce a `lucy.toml` manifest schema and does not provide a package-install command.

A practical source package can look like:

```text
my_project/
├── src/
│   ├── init.lucy
│   ├── parser.lucy
│   └── utils.lucy
├── examples/
└── tests/
```

Use normal Lucy imports from the project environment. Do not assume registry/package-manager behavior exists.

## 8. Native extensions

Native extensions are shared libraries loaded through the extension API.

Platform naming:

| Platform | Typical extension name |
|---|---|
| Windows | `lucy_<name>.dll` |
| Linux | `liblucy_<name>.so` or `lucy_<name>.so` |
| macOS | `liblucy_<name>.dylib` or `lucy_<name>.dylib` |

The runtime searches project, installation, and `LUCY_PATH` extension roots.

The exported entry point is:

```cpp
extern "C" bool lucy_extension_init(lucy::ExtensionAPI& api);
```

The current public API version is `1`. Extensions should be rebuilt when the runtime changes in ways that affect the API/ABI; there is no promise of permanent ABI stability.

## 9. Testing

Lucy uses CTest for the regression suite. Tests are ordinary `.lucy` programs plus native tests where C++ behavior is being exercised.

From a fresh build directory:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLUCY_ENABLE_SQLITE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

The suite currently includes language, lexical, semantics, standard-library, numeric-conversion, and REPL-completion coverage.

When adding a language feature, add a focused regression test and run the complete suite.

## 10. Editor integration

The repository contains editor-related files, but Lucy 2.0.1 does not ship a language server (LSP), formatter, or debugger protocol. Syntax highlighting/editor support should therefore be treated as editor configuration rather than an LSP contract.

## 11. Troubleshooting by symptom

### `SQLiteError: Lucy was built without SQLite support`

Reconfigure with:

```text
-DLUCY_ENABLE_SQLITE=ON
```

and make sure SQLite3 development libraries are available.

### Completion shows nothing

Check whether the expression before `.` can be resolved in the current environment. Completion is intentionally advisory and will not execute arbitrary expressions to guess their result.

### REPL behaves like ordinary stdin

Lucy falls back to line input when stdin is not a TTY. This is expected when piping input or running in some CI/terminal environments.

### A standard module is missing

The shipped standard modules are loaded automatically, but optional SQLite support may be absent. Use `modules_info()` to inspect resolved module paths.

## 12. Cross-platform expectations

Lucy aims to keep language semantics identical across Windows, Linux, and macOS. Platform-specific differences are confined primarily to:

- terminal input implementation;
- shared-library extension suffixes;
- OS/process functions in `system`;
- filesystem path representation;
- availability of optional native dependencies such as SQLite.

Application code should prefer `fs`, `system`, `text`, and other standard modules instead of embedding OS-specific shell assumptions.

## 8. 2.0.1 interactive workflow

The default prompt and banner are defined in `stdlib/repl.lucy`; they use ANSI styling in an interactive terminal. ANSI control sequences are stripped when output is redirected or piped. This remains ordinary Lucy code so downstream builds can change the visual identity without recompiling the native editor. The editor has a plain-stdin fallback when input is piped or a terminal cannot be configured.

### Session commands

| Command | Effect |
|---|---|
| `:help` | Print the topic index and editor shortcuts |
| `:help string` | Ask the Lucy help library for a topic (use `help "string"` if the shell command form is unavailable) |
| `:clear` | Clear the terminal display and the pending input buffer |
| `:reset` | Cancel a pending multiline expression without exiting |
| `:about` | Redisplay the styled banner |
| `:history` | Print recent in-memory/persisted history entries |
| `:version` | Print the runtime version |
| `:quit`, `:exit` | Leave the session |

`Ctrl-C` cancels the current line or pending multiline block. `Ctrl-D` exits only when the editor buffer is empty. `Ctrl-L` clears and redraws the terminal. `Ctrl-W` deletes the previous whitespace-delimited word. The editor supports UTF-8 byte input and code-point boundary movement for basic editing; terminal column width for East Asian wide characters and combining sequences is approximate, and the editor is not a full Readline replacement.

### Multiline input and evaluation

The REPL buffers input while braces are unbalanced, then parses and evaluates the accumulated source. This is intentionally lightweight: it is not a full incremental parser, so braces in comments and unusual quoting contexts can confuse the continuation prompt. When an expression produces a non-`nil` value, the REPL prints that value. Statements whose result is `nil` do not produce an extra result line.

### Completion and history

Tab completion reads candidates from the active interpreter environment and object members. First Tab completes a unique match; for ambiguous matches it completes a shared prefix and displays candidates; repeated Tab cycles through the available candidates. History is stored in `.lucy_history` under the user's home directory, capped at 1,000 entries, and duplicate entries are moved toward the newest position. History is local plaintext; do not enter secrets into the REPL.

### Known editor limitations

- No syntax highlighting of the editable input buffer, bracket-pair highlighting, inline diagnostics, or completion documentation popup.
- No multiline cursor editing or persistent multiline history entries; each physical line is stored separately.
- Completion may evaluate a limited safe subset of expressions to infer the receiver type; it cannot reliably infer every dynamic expression.
- Terminal escape sequences, true display-cell widths, and Windows console behavior vary by terminal. Verify styling in the actual terminal used for release.
- `Ctrl-C` cancels buffered source; it is not a general-purpose interrupt mechanism for a running Lucy program.
