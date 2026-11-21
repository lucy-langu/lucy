# Legacy project command metadata

Lucy retains `[commands]` in `lucy.toml` for backward compatibility with existing projects. First-party Lucy development workflows are now managed by Paw; the root Lucy repository no longer installs one global binstub per tool.

See [Paw](PAW.md) for installation, commands, architecture, and current feature boundaries.

The low-level `lucy run <project-command>` interface remains supported for projects that already use `[commands]`. It is not the recommended interface for Lucy's own ecosystem tools.
