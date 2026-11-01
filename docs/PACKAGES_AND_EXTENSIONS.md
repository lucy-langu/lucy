# Lucy 2.0.2 Packages and Native Extensions

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

## Standard-library module release metadata

Lucy ships its source standard library under `stdlib/`. Each module is independently versioned in `stdlib/manifest.json`; this is distinct from the interpreter's `VERSION`. The manifest currently provides module identity, semantic version, entry point, stability, and declared dependencies. Validate it with `./build/lucy tools/stdlib_manager.lucy verify` and inspect available module versions with `./build/lucy tools/stdlib_manager.lucy list`.

This mechanism establishes independent version metadata and release discipline, not a remote package registry or automatic updater. A true remote registry, dependency solver, integrity/signature verification, and transactional update/rollback remain future ecosystem work and are intentionally not represented as already implemented.


## Lucy-native package management

The repository includes `tools/lucy_pkg.lucy` for local source package workflows:

```sh
lucy tools/lucy_pkg.lucy init
lucy tools/lucy_pkg.lucy add my_package ./my_package.lucy 0.1.0
lucy tools/lucy_pkg.lucy update my_package ./my_package_v2.lucy 0.2.0
lucy tools/lucy_pkg.lucy verify
lucy tools/lucy_pkg.lucy list
lucy tools/lucy_pkg.lucy remove my_package
```

The current manager installs a single local `.lucy` entry source and maintains `lucy.lock` plus package metadata. It does not yet fetch remote packages, solve transitive dependencies, or verify signed registry artifacts. See [Ecosystem Tooling](ECOSYSTEM_TOOLING.md) for the exact current contract and roadmap.

## Native extension scaffolding

Create a starter extension with `lucy tools/lucy_ext_new.lucy telemetry`. The generated project uses the public extension header and CMake. Build it against the same Lucy source tree and runtime build that will load it; extension ABI compatibility is not guaranteed across arbitrary interpreter builds.


## Project-local package discovery

When Lucy executes a source file under a directory tree containing `lucy.toml`, it walks upward to the nearest project root and adds that root's `packages/` directory to import lookup. A package installed at `packages/<name>/src/init.lucy` is imported with `import <name>` and is bound under the requested module name, not the implementation filename `init`. Package names are also included in REPL module completion. This lookup is additive to `LUCY_PATH`, so the shared standard library remains available without copying it into every project. The project's `extensions/` directory is added to native extension lookup by the same project-root discovery.


## Remote registry and dependency resolution boundary (2.0.7)

The package manager described above remains local-source oriented. Lucy 2.0.7 adds formal architecture specifications for a remote package registry and a transitive dependency resolver:

- [Package Registry Specification](PACKAGE_REGISTRY_SPEC.md)
- [Dependency Resolver Specification](DEPENDENCY_RESOLVER_SPEC.md)

Neither remote publication/download nor transitive solving is implemented by `tools/lucy_pkg.lucy` in this source archive. Do not infer support from the existence of these design documents. The current `lucy.lock` behavior is local package metadata/integrity bookkeeping, not a registry-signed supply-chain attestation.
