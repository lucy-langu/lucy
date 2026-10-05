# Lucy 2.0.0 Packages and Native Extensions

Lucy has two extension layers: Pure-Lucy packages for portable language-level code and native C++ extensions for capabilities that need the host runtime or external libraries.

## 1. Pure-Lucy package layout

A package can use the following structure:

```text
packages/
  mypackage/
    lucy.toml
    src/
      init.lucy
```

`src/init.lucy` is the package entry point. Package resolution is integrated with the Lucy module lookup system.

## 2. Importing packages

A package exposes the names defined by its entry module. User imports remain explicit even though shipped standard modules are loaded automatically.

```lucy
import mypackage
```

Keep package APIs flat and intentional. Do not require callers to understand internal directory layout.

## 3. Native extensions

Native extensions are shared libraries loaded by the Lucy runtime. The public C++ boundary is declared in `include/lucy/extension.hpp`.

The extension API supports:

- modules;
- functions;
- constants;
- native resource-backed objects;
- dynamic loading through the platform extension mechanism.

The exported initialization contract is represented by `LUCY_EXTENSION_INIT`.

## 4. Extension naming

The runtime recognizes platform-appropriate shared-library names, including:

```text
Windows: lucy_<module>.dll
Linux:   liblucy_<module>.so or lucy_<module>.so
macOS:   liblucy_<module>.dylib or lucy_<module>.dylib
```

## 5. Why use a native extension?

Use Pure Lucy when the capability can be expressed using the language and standard library. Use C++ when the feature requires:

- a platform API not exposed by the standard library;
- an external native library;
- performance-sensitive native code;
- a resource with native lifetime semantics.

Do not write a C++ extension merely to wrap a function that Lucy can already implement cleanly.

## 6. NativeObject model

`NativeObject` represents a native resource that can participate in Lucy member dispatch. The runtime can bind methods to the native object without pretending the resource is a normal Lucy map.

This is the boundary used by facilities such as sockets, IO, cryptographic resources, and optional SQLite support.

## 7. Extension API design

A good extension should expose a small, stable public module:

```text
module.open(...)
module.read(...)
module.close(...)
```

rather than leaking implementation helpers.

Return Lucy values where possible. Use native objects only when identity/lifetime/resource state matters.

## 8. Package versus extension

| Requirement | Pure-Lucy package | Native extension |
|---|---:|---:|
| Portable Lucy code | Yes | No |
| Uses only Lucy APIs | Yes | Not necessarily |
| Host OS API | Limited | Yes |
| External C/C++ library | No | Yes |
| Easy source distribution | Yes | Platform-dependent |
| Native resource lifetime | Limited | Strong |

## 9. Extension safety and stability

The extension ABI is a runtime boundary. Extensions should target the public header/API rather than including private implementation details. Internal runtime classes are not a substitute for the public extension interface.
