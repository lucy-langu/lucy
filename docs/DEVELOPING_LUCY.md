# Lucy 2.0.0 — Developing, Extending, Testing, and Releasing Lucy

This is the maintainer/developer reference. It combines the implementation architecture, native extension API, build process, tests, migration/versioning policy, limits, and contribution workflow.

## 1. Source architecture

The runtime pipeline is:

```text
source
  -> lexer
  -> tokens
  -> parser
  -> AST
  -> interpreter/runtime
       |-- environments and values
       |-- builtins and object dispatch
       |-- standard-library loading
       |-- modules/packages
       |-- REPL
       `-- native extensions
```

### Important source files

| Path | Responsibility |
|---|---|
| `src/lexer.cpp` | lexical analysis and literal validation |
| `src/parser.cpp` | recursive-descent parsing and AST construction |
| `src/runtime.cpp` | interpreter, environments, calls, dispatch, builtins, modules, REPL integration |
| `src/value.cpp` | runtime value representation/operations |
| `src/phase2.cpp` | native/library support |
| `src/stdlib_native.cpp` | native standard-library facilities |
| `src/repl.cpp` | cross-platform line editor and completion |
| `src/extension.cpp` | shared-library extension loading |
| `src/main.cpp` | CLI entry point |
| `include/lucy/*.hpp` | public/runtime C++ interfaces |
| `stdlib/*.lucy` | user-facing Pure-Lucy standard library |
| `tests/*.lucy` | language/runtime regression programs |
| `tests/repl_completion_test.cpp` | runtime completion regression test |

## 2. Build requirements

Lucy is a C++17/CMake project.

Basic build:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Optional SQLite:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLUCY_ENABLE_SQLITE=ON
cmake --build build
```

On Windows with MinGW:

```text
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Use a fresh build directory when switching generators or moving the source tree. CMake caches absolute paths and should not be reused across incompatible environments.

## 3. Language implementation workflow

A language feature normally crosses several layers:

1. **Lexer:** token spelling and literal validation.
2. **Parser:** grammar/AST construction.
3. **AST/runtime:** execution semantics.
4. **Tests:** positive and negative behavior.
5. **Documentation:** syntax, semantics, examples, and limitations.
6. **REPL completion/help:** discoverability if the feature is user-facing.

Do not add documentation for a syntax form before the parser/runtime can execute it.

## 4. Runtime value model

`Value` is a tagged C++ variant containing:

```text
Nil
bool
long long
 double
string
ArrayPtr
MapPtr
FunctionPtr
ClassPtr
InstancePtr
NativePtr
```

Arrays and maps use C++ heap-backed containers. Functions/classes/instances use shared ownership. The public language does not expose a garbage-collector API.

## 5. Environments and closures

`Environment` stores named entries with:

- `Value`;
- constant flag;
- optional declared type name;
- parent environment.

Closures capture an environment chain. Assignments walk that chain according to runtime binding rules.

## 6. Built-in type extension

The runtime creates builtin class objects for:

```text
Object Nil Bool Int Double Number String Array Map
Function Class Instance Native
```

A Lucy declaration such as:

```lucy
class String {
    func shout() {
        return self.upper() + "!"
    }
}
```

extends the existing builtin class instead of replacing the primitive representation.

This is intentionally public language behavior; application code should not depend on an internal `runtime.extend_type(...)` API.

## 7. Standard-library loading

At interpreter startup, the runtime attempts to load the shipped standard modules:

```text
app crypto data flow fs http math random repl result runtime set sqlite system text time
```

SQLite is optional. Other standard-library failures are fatal because a normal Lucy installation is expected to contain the shipped library.

## 8. Module/package resolution

The runtime searches project/install locations and `LUCY_PATH` for libraries/extensions. User packages conventionally expose `src/init.lucy`.

`LUCY_PATH` is a Lucy installation/project root rather than an arbitrary list syntax. Extension search roots are derived from it and the executable/project layout.

`lucy.toml` is not currently parsed/enforced by the runtime. Do not implement documentation that implies a package registry or dependency resolver exists.

## 9. REPL architecture

`src/repl.cpp` provides:

- platform-specific terminal mode;
- line editing;
- history;
- cursor movement;
- multiline input support through runtime brace-depth tracking;
- runtime-driven completion.

Completion deliberately resolves the live environment and member structure instead of maintaining a second list of every standard-library function. This is a critical maintainability rule.

When adding a new public module function, completion should discover it automatically if it is represented as a public module member.

## 10. Native extension API

The public extension header is `include/lucy/extension.hpp`.

The exported entry point is:

```cpp
extern "C" bool lucy_extension_init(lucy::ExtensionAPI& api);
```

The current API version is `1`.

An extension can register a module and expose functions/constants/native objects through `ExtensionAPI`.

### Example shape

```cpp
#include "lucy/extension.hpp"

extern "C" bool lucy_extension_init(lucy::ExtensionAPI& api) {
    api.module("example")
        .function("answer", [](const std::vector<lucy::Value>&) {
            return lucy::Value(42);
        });
    return true;
}
```

The exact builder methods are defined by the installed `extension.hpp`; the header is authoritative if this example ever diverges.

### ABI policy

API versioning exists, but Lucy 2.0.0 does not promise that arbitrary extensions compiled against one runtime build remain binary-compatible forever. Rebuild extensions when the runtime/API changes.

### Platform names

- Windows: `lucy_<module>.dll`
- Linux: `liblucy_<module>.so` or `lucy_<module>.so`
- macOS: `liblucy_<module>.dylib` or `lucy_<module>.dylib`

## 11. Optional SQLite

CMake option:

```text
LUCY_ENABLE_SQLITE=ON|OFF
```

The default build can omit SQLite. The runtime then reports a clear `SQLiteError` rather than making the entire interpreter unbuildable.

## 12. Testing strategy

Run the complete suite after source changes:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLUCY_ENABLE_SQLITE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

Language tests are intentionally small Lucy programs. Native behavior that cannot be expressed safely in Lucy can use a C++ test.

### Regression-test rule

For every bug fix, prefer a test that fails on the old behavior and passes on the fixed behavior.

For syntax features, test at least:

- valid minimal form;
- valid nested form;
- multiline form where relevant;
- invalid form and its error category;
- interaction with existing precedence/scope rules.

For runtime features, test:

- normal path;
- boundary values;
- type errors;
- error propagation;
- interaction with modules/closures where relevant.

## 13. Current test coverage areas

The repository includes coverage for:

- legacy 1.1.1 behavior;
- 2.0 syntax;
- type errors;
- structs;
- language regressions;
- hexadecimal/binary validation;
- standard-library imports;
- builtin type extension and automatic standard modules;
- ranges and indexed `for`;
- numeric base conversions;
- lexical rules;
- semantics;
- REPL completion.

The numbered filenames are historical test organization, not a public test API.

## 14. Versioning

Lucy uses semantic versioning intent:

- patch: compatible fixes/documentation/regressions;
- minor: backward-compatible features;
- major: syntax/semantic/API breaks.

The 2.0 line is a major syntax/semantic generation. New documentation should not mix 1.x syntax into 2.0 examples.

Deprecations should be documented before removal where practical. A release must update the language reference, standard-library reference, changelog, tests, and migration notes when a user-visible feature changes.

## 15. Migration policy: 1.x to 2.0

The major migration themes are:

- `end`-terminated blocks -> brace-delimited blocks;
- older output conventions -> `print` / `println`;
- older filesystem/OS namespace layouts -> `fs` / `system`;
- expression lambdas -> `lambda ... => ...`;
- runtime-enforced type contracts;
- automatic loading of shipped standard modules;
- quoted map keys;
- ranges and modern collection operations;
- built-in type extension;
- package/native-extension support.

Old examples should be migrated semantically rather than mechanically. `HISTORY_AND_MIGRATION.md` remains useful as historical background, but the 2.0 language/reference documents are the current authority.

## 16. Release checklist

Before publishing a Lucy release:

- [ ] Update version constants and visible version strings.
- [ ] Run a clean configure/build from an empty build directory.
- [ ] Run the full CTest suite.
- [ ] Verify standard-library files are packaged.
- [ ] Verify docs are packaged.
- [ ] Verify the CLI `--version` output.
- [ ] Verify REPL startup and completion.
- [ ] Verify Windows, Linux, and macOS-specific code paths where available.
- [ ] Verify optional SQLite behavior for both enabled and disabled builds.
- [ ] Update changelog and migration information.
- [ ] Build release archives/installers without stale build caches.

## 17. Documentation maintenance rule

Documentation is organized around user tasks rather than implementation file count. The intended stable set is:

- `README.md`
- `GETTING_STARTED.md`
- `LANGUAGE_REFERENCE.md`
- `STANDARD_LIBRARY.md`
- `REPL_AND_TOOLING.md`
- `DEVELOPING_LUCY.md`
- `CHANGELOG.md`

Small topic documents should only exist when they serve a genuinely independent audience or protocol. Otherwise, merge them into the appropriate canonical document so the same API is not described in multiple conflicting places.

## 18. Architecture decision rule

When adding functionality, choose the smallest correct layer:

- language syntax -> lexer/parser/runtime;
- reusable pure language behavior -> `stdlib/*.lucy`;
- OS/external-library primitive -> native runtime/extension;
- user-facing discoverability -> standard-library docs + REPL help/completion;
- regression -> test suite.

Avoid native wrappers for behavior that can be implemented cleanly in Lucy. Avoid hidden runtime APIs when a public Lucy feature can express the same operation.
