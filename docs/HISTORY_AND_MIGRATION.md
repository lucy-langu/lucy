# Lucy History and Migration

This document separates Lucy's historical development from the current 2.0 API. Older behavior is described here so that existing users can migrate without confusing old syntax with current syntax.

## 1. Development direction

Lucy began as a smaller DSL/runtime and progressively moved toward a general-purpose language. The older documentation emphasized a compact core, collections, classes, modules, filesystem/OS facilities, a REPL, and a growing standard library. fileciteturn32file3

The 1.0.1 documentation also established an important API principle: public standard-library functions should be flat and readable, while `__*` and `_name` helpers remain implementation details. fileciteturn32file1

The 2.0 line consolidates those ideas around a brace-delimited language, runtime type contracts, built-in type extension, automatic shipped-standard-module loading, package support, and a native C++ extension boundary.

## 2. 0.9.x → 1.x

The older language surface used `end`-terminated blocks and older module names. Examples from the historical language reference included forms such as:

```text
if condition
    ...
end
```

and older module layouts such as `file`, `dir`, `path`, and `os`. Those names should not be copied into new 2.0 code. The historical reference itself pointed readers to the then-current standard-library document. fileciteturn32file3

## 3. 1.0.1 standard-library direction

Lucy 1.0.1 explicitly described a flat module API such as:

```lucy
fs.read "notes.txt"
http.get url
data.parse json_text
```

and discouraged deep namespace chains. fileciteturn31file1

That design principle remains in 2.0. The implementation has evolved, but the public surface should stay shallow.

## 4. 1.1.1 → 2.0

The 2.0 language surface introduced or consolidated:

- brace-delimited blocks;
- runtime-enforced type annotations;
- binary and hexadecimal literals with separators;
- quoted map keys;
- expression lambdas;
- empty blocks;
- typed structs;
- `??` fallback behavior for missing names on the left side;
- `println` and explicit `exit(code)`;
- native REPL completion;
- automatic loading of shipped standard modules;
- built-in type extension through ordinary `class Type { ... }` declarations;
- Pure-Lucy packages;
- native C++ extensions;
- optional SQLite support.

## 5. Migrating blocks

Old:

```text
if ready
    print "yes"
end
```

Current:

```lucy
if ready {
    println "yes"
}
```

Do not retain `end` in new 2.0 examples.

## 6. Migrating output

Old code may use `echo` or older output conventions. Current code uses:

```lucy
print "same line"
println "new line"
```

`echo` is not a current 2.0 builtin.

## 7. Migrating functions

Current functions use `func` and brace bodies:

```lucy
func add(a: Int, b: Int) -> Int {
    return a + b
}
```

The runtime now enforces the declared parameter and return types.

## 8. Migrating lambdas

Current expression-lambda forms are:

```lucy
lambda x => x * 2
lambda(x, y) => x + y
```

Do not document block-bodied lambdas as a 2.0 feature unless the parser/runtime is extended and tested for them.

## 9. Migrating maps

Current quoted keys are explicit and valid:

```lucy
let user = {
    "nickname": "Nima",
    'age': 20
}
```

## 10. Migrating types

In earlier versions, annotations could be treated mainly as interface information. In 2.0 they are runtime contracts. Code that relied on returning a value of a different type must be corrected rather than expecting silent acceptance.

## 11. Migrating standard-library imports

Shipped standard modules are automatically loaded in 2.0. This means new code does not need:

```lucy
import fs
import text
import http
```

merely to access the built-in modules. Imports are still required for user modules and packages.

## 12. Migrating old filesystem/OS names

The current API is organized around modules such as `fs` and `system`:

```lucy
fs.read "file.txt"
system.cwd()
system.platform()
```

Do not copy historical `file.*`, `dir.*`, `path.*`, or `os.*` examples into 2.0 code.

## 13. Migrating hardware references

The 2.0 core documentation does not expose the earlier hardware simulator concept. Hardware work can be implemented later through an appropriate library or extension boundary without making a simulator part of the language core.

## 14. Migrating numeric work

2.0 supports numeric literals:

```lucy
let mask = 0b1010_1010
let address = 0xFF00
```

and conversion helpers:

```lucy
hex(255)
bin(255)
oct(255)
```

For zero-padded protocol/register formatting:

```lucy
hex(15, 4)       # 000F
255.to_binary(8) # 11111111
```

## 15. Migration principle

When moving a project, migrate behavior rather than mechanically replacing tokens. The old documentation should help explain intent, but current 2.0 reference pages are the authority for actual syntax and APIs. This avoids the partial-update problem where a guide contains a mixture of old and new instructions. citeturn0search5
