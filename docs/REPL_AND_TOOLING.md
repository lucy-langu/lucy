# Lucy 2.0.0 REPL and Tooling

## 1. Starting the REPL

Run `lucy` without a source file. Expressions are evaluated interactively and non-nil results are displayed.

```text
Lucy$ 2 + 2
4
```

The REPL is also a practical way to inspect methods and test standard-library calls before moving them into a source file.

## 2. Interactive commands

The current REPL command layer includes the following documented commands:

- `:help` — show help.
- `:clear` — clear the terminal.
- `:version` — show Lucy version.
- `:history` — show command history.
- `:exit` — leave the REPL.
- `:quit` — exit alias.

The `repl` standard module exposes programmatic helpers such as `commands()`, `topics()`, and `help(topic)`.

## 3. History and editing

The native REPL editor supports interactive line editing, history navigation, and member completion. Repeated Tab presses cycle through multiple candidates instead of merely printing an unordered list.

For example:

```text
Lucy$ text.<Tab>
base64_decode  base64_encode  find_all  hex_decode ...
```

After a unique candidate is selected, the completion inserts the member name while retaining the object prefix.

## 4. Member completion

Completion is aware of dotted member paths and does not need to execute arbitrary user code just to discover candidates. This matters for safety and for avoiding side effects during interactive completion.

Examples:

```text
text.<Tab>
math.<Tab>
runtime.<Tab>
```

The current completion inventory includes built-ins, standard modules, and known members for arrays, strings, maps, and numbers.

## 5. File execution

Use:

```sh
lucy program.lucy
```

The process exit status is non-zero when an uncaught runtime error terminates a script.

## 6. REPL versus script behavior

The same runtime semantics are used in both modes, but error handling differs at the user-interface level:

- Script mode reports uncaught diagnostics and exits non-zero.
- REPL mode reports the diagnostic and continues the interactive session where recovery is possible.

## 7. Debugging workflow

A useful workflow is:

1. Reproduce a small expression in the REPL.
2. Inspect the value with `runtime.inspect` or `runtime.type`.
3. Use Tab completion to verify the available public member names.
4. Move the smallest working expression into a test file.
5. Add the regression test before changing the runtime.

## 8. Editor support

The source tree installs editor integrations under the configured installation prefix. The project currently contains Vim, Sublime, and VS Code-related editor resources. These are tooling integrations; they do not define Lucy's grammar independently of the lexer/parser.

## 9. Help as part of the language

The parser recognizes the `help` convenience form for documented topics. The `repl` module provides the programmable side of that system.

Keep the reference docs and help output synchronized. If an API changes, update both the source-level inventory and the user-facing documentation.
