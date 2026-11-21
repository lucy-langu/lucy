# Lucy 2.0 Extensions

Lucy 2.0 can load native C++ extensions as dynamic modules.

An extension exports one symbol:

```cpp
LUCY_EXTENSION_INIT {
    api.module("hello")
        .constant("version", "1.0")
        .function("greet", [](lucy::Interpreter &, const std::vector<lucy::Value> &args) {
            return lucy::Value("Hello, " + args[0].to_string());
        });
    return true;
}
```

Build the extension as `lucy_hello.dll`, `liblucy_hello.so`, or `liblucy_hello.dylib` and place it in `$LUCY_PATH/extensions/`.

`LUCY_PATH` is a single Lucy root. It controls the standard library, packages, native extensions, and other Lucy resources. Lucy also supports project-local `extensions/` directories for development.

Example:

```bash
export LUCY_PATH="$HOME/.local"
cp liblucy_hello.so "$LUCY_PATH/extensions/"
```

Then:


Then:

```lucy
import hello
print hello.greet("Lucy")
```

The host keeps loaded extensions alive for the lifetime of the interpreter.
