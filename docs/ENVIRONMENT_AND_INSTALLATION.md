# Lucy 2.0.0 Environment and Installation

## 1. Installation layout

The CMake project installs:

- the `lucy` executable;
- the runtime library where required by the platform;
- the standard library;
- documentation;
- editor integrations;
- public C++ headers;
- optional extension examples/resources;
- assets.

On Unix-style installations, standard-library resources are normally placed under `share/lucy/stdlib`. On a local prefix, this commonly becomes `$prefix/share/lucy/stdlib`.

## 2. `LUCY_PATH`

`LUCY_PATH` is the main user-configurable resource search root.

A deployment can use:

```sh
export LUCY_PATH="$HOME/.local/share/lucy"
```

The runtime considers the configured root and relevant `stdlib`/`packages` locations when resolving resources. Package entry points use `packages/<name>/src/init.lucy`.

Do not introduce separate `LUCY_STDLIB` or `LUCY_EXTENSIONS` variables into new documentation; the 2.0 resource model is centered on `LUCY_PATH`.

## 3. Relocatable installations

The installed runtime resolves standard-library resources relative to the executable and installation tree. This allows a complete Lucy installation tree to be moved when its internal layout remains intact.

## 4. Building with SQLite

SQLite support is optional:

```sh
cmake -S . -B build -DLUCY_ENABLE_SQLITE=ON
cmake --build build
```

When enabled, CMake locates SQLite and links the runtime accordingly. When disabled, the standard library still contains the SQLite-facing module surface, but actual database operations cannot succeed because the native backend is absent.

## 5. Runtime resource search

The module/resource resolver checks the relevant source and package locations, `LUCY_PATH`, executable-relative standard-library locations, and platform installation locations. This allows both development-tree and installed-tree usage.

## 6. Environment values from Lucy

Use `system` for process environment access:

```lucy
println system.cwd()
println system.env("HOME")
println system.argv()
```

Use `setenv` and `unsetenv` when a process needs to modify its own environment.

## 7. Cross-platform expectations

The language core is designed to be portable. OS-facing functionality naturally depends on the host platform. Code that uses `system.uid`, Windows-specific process behavior, signals, or filesystem permissions should account for platform differences.

The standard library is the portability boundary: prefer `fs`, `system`, `time`, and `http` APIs over hand-written host-specific shell commands where practical.
