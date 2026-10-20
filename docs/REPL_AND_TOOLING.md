# Lucy 2.0.3 — REPL, CLI, Tooling, Packages, and Developer Workflow

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

There is no documented `lucy -e` or `lucy -c` execution mode in 2.0.2. Do not assume those flags exist.

An uncaught script error is reported by the CLI and results in a non-zero process status. In the REPL, errors are printed and the interactive session continues when possible.

## 2. The REPL

Start it with:

```text
lucy
```

The default prompt is provided by the editable `stdlib/repl.lucy` library. The banner and prompts use plain text with no ANSI color styling to avoid inconsistent rendering in Windows consoles. The editor still uses terminal cursor-control sequences for line editing; these are not color codes. The native editor can fall back to a native prompt if that library cannot be loaded.

### REPL commands

| Command | Action |
|---|---|
| `:help` | Show REPL help |
| `:clear` | Clear the terminal and current input buffer |
| `:history` | Show recent history |
| `:version` | Show Lucy version |
| `:about` | Show the plain-text banner |
| `:reset` | Cancel a pending multiline input buffer |
| `:vars` | List user-defined global variables with values and runtime types |
| `:vars --all` | Include runtime, module, and built-in bindings in the listing |
| `:functions` | List built-in functions, user functions, and visible module methods |
| `:modules` | List discovered `.lucy` modules and filesystem paths |
| `:where <name>` | Show a module path or inspect a binding's runtime origin |
| `:methods <Type>` | List known methods for a built-in type or user class |
| `:quit`, `:exit` | Exit |
| `Ctrl-D` | Exit when the input buffer is empty |

`:where` reports exact paths for discoverable modules and source-file-defined functions/classes. Interactive definitions do not retain source-line metadata, so they are identified as interactive rather than assigned a fabricated location. `:modules` lists `.lucy` files found in configured module search paths; it is not a complete operating-system filesystem index.

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
lucy> fs.<Tab>
lucy> text.u<Tab>
lucy> values.<Tab>
lucy> point.x.to_<Tab>
```

### 3.2 Tab behavior

- If there is one candidate, the first Tab completes it immediately.
- If there are multiple candidates, the first Tab completes the longest common prefix when possible.
- The next Tab prints the sorted candidate list; later Tabs cycle through candidates.
- Editing the line or moving the cursor resets the completion cycle.
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
lucy> func greet(name) {
...     println "Hello, $name"
... }
```

Input is then parsed as one buffer. A syntax error does not permanently damage the next REPL prompt; the current buffer is reset after the attempt.

## 7. Packages

The runtime recognizes Lucy package/source layouts around `src/init.lucy`. Package resolution is part of module resolution; however, 2.0.2 does not enforce a `lucy.toml` manifest schema and does not provide a package-install command.

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

The suite currently includes language, lexical, semantics, standard-library, numeric-conversion, built-in type extension, module-path discovery, and REPL-completion coverage. The release verification script also runs every shipped `examples/*.lucy` program.

When adding a language feature, add a focused regression test and run the complete suite.

## 10. Editor integration

The repository contains editor-related files, but Lucy 2.0.3 does not ship a language server (LSP), formatter, or debugger protocol. Syntax highlighting/editor support should therefore be treated as editor configuration rather than an LSP contract.

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

### Known editor limitations

- No syntax highlighting, bracket-pair highlighting, inline diagnostics, or completion documentation popup.
- No multiline cursor editing or persistent multiline history entries; each physical line is stored separately.
- Completion is advisory and may not infer every dynamic expression.
- Terminal cursor controls and display-cell widths vary by terminal; the REPL no longer emits ANSI color styling, but Windows behavior still needs native-console testing.
- `Ctrl-C` cancels buffered source; it is not a general-purpose interrupt for a running Lucy program.
