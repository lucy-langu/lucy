# Lucy Tooling and Automation

The official first-party toolchain entry point is **Paw**. Do not install separate `lucy-*` commands or generate repository-wide per-tool Binstubs.

See [PAW.md](PAW.md) for the command reference and installation instructions.

## Command implementation

Paw is a small executable bootstrap that starts Lucy source:

- `tools/paw/main.lucy` dispatches commands.
- `tools/paw/commands/*.lucy` implement project, build, test, package, extension, stdlib, and documentation workflows.
- `src/main.cpp` locates the sibling Lucy runtime and installed Paw/stdlib data.
- CMake builds and installs `paw` beside `lucy`.

The existing `lucy run <project-command>` and `[commands]` manifest support remain available for compatibility with existing user projects. They are not the first-party repository toolchain.

## Safety rules for tools

- Execute external programs with `system.exec(argv, options)`, not interpolated shell command strings.
- Validate project/package names before filesystem mutation.
- Keep package entry paths within the project package directory.
- Use temporary files and replacement for lock/manifest updates.
- Treat non-zero process status and timeouts as failures.
- Keep tool behavior in Lucy source; native C++ should remain limited to runtime/bootstrap/OS integration.

## Shipped feature boundaries

Paw 0.1.0 supports local project scaffolding, project command execution, CMake build/test frontends, local source packages, native extension scaffolding, standard-library manifest management, and documentation generation/verification.

Remote registry publishing/downloading, transitive dependency solving, LSP/DAP, formatter, and linter are not shipped by this toolchain snapshot.
