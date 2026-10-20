# Lucy object model and diagnostics (2.0.3)

This document describes the runtime contract implemented in this source tree. It intentionally distinguishes supported behavior from Ruby features that Lucy does not yet implement.

## Classes and method kinds

```lucy
class Animal {
    func speak() { return "animal" }
    protected func family() { return "animal" }
}

class Dog < Animal {
    static func species() { return "dog" }
    private func secret() { return "internal" }
    func speak() { return super.speak() + "-dog" }
    func reveal() { return self.secret() }
}

assert(Dog.species() == "dog")
assert(Dog.new().speak() == "animal-dog")
assert(Dog.new().reveal() == "internal")
```

- `class Child < Parent` establishes one parent class. Lookup walks the child then its parent chain.
- `func method(...)` defines an instance method. An instance call binds `self` to the receiver.
- `static func method(...)` defines a class method, called as `Type.method(...)`; it does not receive an instance `self`.
- `super.method(...)` starts lookup at the parent of the class that defined the currently executing method, and binds the current instance.
- `private func` restricts method access to methods executing in the declaring class. `protected func` permits access from the declaring class and its subclasses. Unmodified methods are public.
- Modifiers may precede a method declaration. Duplicate `static` modifiers are rejected.
- Existing `File`, `Dir`, and `Path` facade APIs remain class-callable; `class File { ... }` extends the shared global class rather than creating a second class.

### Current boundaries

This is not a claim of full Ruby compatibility. Lucy currently supports single inheritance, explicit `super.method(...)`, static methods and method visibility. It does not implement Ruby's implicit `super`, mixins/modules as traits, refinements, singleton classes, property visibility metadata, or a full method-resolution protocol. Visibility is enforced for methods, not arbitrary instance fields. These differences are part of the contract, not undocumented promises.

## Source locations and runtime errors

Every parsed statement and expression root records a 1-based line and column. When a runtime error crosses one of these nodes, Lucy annotates the first relevant location as:

```text
NameError: undefined variable 'usrname' [Lucy source: app.lucy:8:9]
 --> app.lucy:8:9
  |
8 | println usrname
  |         ^
  at render (app.lucy:4:1)
  at main (app.lucy:1:1)
```

The exact error text depends on the runtime operation. Function boundaries append frames as an exception unwinds. Parser and lexer diagnostics retain their own token-based coordinates.

### Diagnostic rules

- Lines and columns are 1-based.
- A location is printed only when it originates from parser/token metadata; Lucy does not fabricate source coordinates for native operations that provide none.
- A call frame records function name and, when known, source file and declaration line/column.
- Native C++ frames and OS-level stack frames are not currently available.
- Column positions are lexer character positions, not terminal display-cell widths; wide Unicode glyphs may make a caret visually offset in some terminals.

## Method visibility errors

An inaccessible method raises `AccessError`, while an unknown method raises `NoMethodError`. Calling a non-static instance method through a class object is rejected. Use an explicit `static func` for class-level behavior.

## API contract expectations

Each public standard-library API should document: canonical name, signature, accepted input types, return type, error conditions, platform constraints, and one executable example. `docs/contracts/core_api.json` is the machine-readable contract for the core `File`, `Dir`, and `Path` facade subset. A contract entry is not a promise that an undocumented feature exists; implementation and regression tests remain authoritative.
