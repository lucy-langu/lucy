# Paw — Lucy's official toolchain

**Paw version:** 0.1.0  
**Lucy compatibility:** 2.0.7  
**Implementation:** Lucy command modules, with a small C++ bootstrap executable.

Paw is the single supported command-line entry point for first-party Lucy project workflows. Its command implementations live in `tools/paw/commands/` as `.lucy` programs. The `paw` executable only locates the Lucy runtime, standard library, and Paw source tree, then starts `tools/paw/main.lucy`.

## Installation

Install Lucy and Paw together:

```sh
cmake -S . -B build -DLUCY_ENABLE_SQLITE=OFF
cmake --build build --parallel
cmake --install build --prefix "$HOME/.local"
```

On Unix-like systems, add `$HOME/.local/bin` to `PATH`. On Windows, add the installation directory containing `lucy.exe` and `paw.exe` to `PATH`.

Paw expects the Lucy runtime beside its executable. Its source modules and standard library are installed under the Lucy data directory. A development build can also locate the source tree used at configure time.

## Commands

```sh
paw --help
paw --version
paw new my_app
cd my_app
paw run start
paw run test
```

Repository maintenance:

```sh
paw build all
paw test
paw stdlib verify
paw docs
paw docs-check
paw extension sample_extension
```

Local source packages:

```sh
paw init
paw add greeting ./greeting.lucy 1.0.0
paw list
paw verify
paw update greeting ./greeting-v2.lucy 1.1.0
paw remove greeting
```

`paw install <name> <source.lucy> [version]` is an alias for `paw add` in this release. Package installation is local-source only.

## Command ownership

| Command | Lucy implementation |
|---|---|
| `new` | `commands/new.lucy` |
| `build` | `commands/build.lucy` |
| `test` | `commands/test.lucy` |
| `init`, `add`, `install`, `update`, `remove`, `list`, `verify` | `commands/package.lucy` |
| `extension` | `commands/extension.lucy` |
| `stdlib` | `commands/stdlib.lucy` |
| `docs` | `commands/docs.lucy` |
| `docs-check` | `commands/docs-check.lucy` |
| command dispatcher | `main.lucy` |

No per-command PATH binstubs are generated. There is one installed `paw` executable.

## Current boundaries

The following are not claimed as implemented in Paw 0.1.0:

- Remote registry search, download, authentication, and publishing.
- Transitive dependency resolution and version-conflict solving.
- Workspace-wide dependency graph operations.
- Production Language Server Protocol or Debug Adapter Protocol.
- A package cache or signed package verification.

Those require dedicated designs and tests; the local lockfile workflow is not a substitute for a remote package manager.
