# Lucy REPL

## Overview

Lucy 1.0.1 uses a native cross-platform line editor, while the user-facing REPL description layer is implemented in `stdlib/repl.lucy`. This split keeps terminal mechanics reliable while making the REPL's visible identity editable with Lucy itself.

## Editable REPL library

The file `stdlib/repl.lucy` defines:

| Function | Purpose |
|---|---|
| `banner()` | Startup banner text |
| `version()` | Version shown by `:version` |
| `prompt(depth)` | Primary/continuation prompt |
| `commands()` | Command metadata |
| `topics()` | Help topic list |
| `help(topic)` | Help renderer |

Changing these functions changes the corresponding REPL presentation without rebuilding the C++ interpreter.

### Example customization

```lucy
def prompt(depth = 0)
    if depth == 0
        return "lucy> "
    end
    return ".... "
end
```

The native editor still owns cursor movement, history, completion, and terminal input. The Lucy layer owns the presentation and help content.

## Starting

Run `lucy` without a source file.

## Editing

- Up/Down: history
- Left/Right: cursor movement
- Home/End: line boundaries
- Tab: completion
- Ctrl-D: exit

## History

History is stored in `~/.lucy_history` on Unix-like systems and `%USERPROFILE%\.lucy_history` on Windows. Up to 1000 entries are retained.

## Completion

Completion covers language keywords, loaded names, built-ins, module names, module members, class members, object fields, and built-in Array/String/Map methods.

## Multiline input

Blocks use `end`. `do ... while` and `switch ... case ... default ... end` are also recognized by the REPL's block-depth tracker. Delimited expressions such as `foo(`, `[`, and `{` can be formatted across multiple lines; their closing delimiter completes the expression.

## Commands

- `:help` — rendered by `stdlib/repl.lucy`
- `:clear` — clears the terminal and current input buffer
- `:history` — shows recent history
- `:version` — rendered by `stdlib/repl.lucy`
- `:quit` / `:exit` — exits

## Language help

Inside the REPL, `help print` and `help "lambda"` are Lucy syntax sugar for the Lucy-level `help` function supplied by `repl.lucy`. The documentation is not stored in the C++ runtime.
