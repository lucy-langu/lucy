# Lucy Language Specification 2.0.7

**Status:** normative baseline for the 2.0.7 language contract  
**Implementation:** C++17 interpreter  
**Scope:** syntax and observable language/runtime behavior implemented by this source tree

This document defines the compatibility contract for Lucy 2.0.7. The detailed examples and API inventories remain in `LANGUAGE_REFERENCE.md`, `BUILTIN_REFERENCE.md`, and `STANDARD_LIBRARY.md`. If a descriptive example conflicts with executable behavior or a regression test, the conflict must be resolved by changing either the implementation or this specification; it must not be silently treated as a new feature.

The terms **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are normative.

## 1. Language identity and compilation pipeline

Lucy source is case-sensitive. The implementation pipeline is:

```text
UTF-8 source
  -> Lexer
  -> Token stream
  -> Recursive-descent Parser
  -> AST
  -> Interpreter
  -> Runtime Values / Environments / Native boundary
```

The 2.0.7 implementation interprets an AST; it does not compile source to machine code or bytecode. A C++17 shared runtime is built separately from the command-line executable.

## 2. Source and lexical contract

- Source files are interpreted as byte sequences containing UTF-8 text. String operations documented as code-point operations do not imply grapheme-cluster segmentation.
- Identifiers are ASCII-based. An identifier begins with an ASCII letter or `_`; later characters may also contain decimal digits. A trailing `?` or `!` is accepted for supported identifier forms.
- `#` starts a line comment. The lexer also recognizes the `=begin` / `=end` multiline-comment form.
- Newline and semicolon are statement separators where the grammar permits a statement boundary. Semicolons are not required after every statement.
- A backslash immediately followed by LF or CRLF continues a logical line. String escapes include `\a`, `\b`, `\f`, `\n`, `\r`, `\t`, `\v`, `\0`, `\\`, escaped quotes, `\/`, `\xNN`, and `\uXXXX`; unknown and incomplete escapes are syntax errors.
- Both `'...'` and `"..."` produce string values. The supported escape set is documented in the language reference. Invalid or incomplete escapes are syntax errors.
- `${expression}` evaluates a full Lucy expression during interpolation. `$identifier` and supported dotted shorthand insert a resolved value; shorthand does not implicitly invoke a callable member.
- Integer literals support decimal, hexadecimal (`0x`), and binary (`0b`) forms with validated separators. Integer overflow is a syntax error.
- Backtick-delimited command expressions are supported. They are process interaction, not a shell-sandbox guarantee.

## 3. Core grammar

The following EBNF describes the accepted language shape. It is intentionally explanatory around context-sensitive details (such as command-style calls, newline handling, and declaration modifiers); `src/parser.cpp` remains the executable grammar.

```ebnf
program          = { separator | statement } ;
separator        = newline | ";" ;

statement        = declaration | assignment | control_statement
                 | function_decl | class_decl | struct_decl
                 | import_decl | exception_statement | expression ;

declaration      = ( "let" | "var" | "const" | "global" )
                   identifier [ ":" type_name ] [ "=" expression ] ;

assignment       = assignable assign_operator expression ;
assignable       = identifier | postfix_expression ;
assign_operator  = "=" | "+=" | "-=" | "*=" | "/=" | "%="
                 | "**=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ;

function_decl    = [ visibility ] [ "static" ] ( "func" | "function" | "def" )
                   identifier "(" [ parameters ] ")" [ "->" type_name ] block ;
parameters       = parameter { "," parameter } ;
parameter        = [ "*" ] identifier [ ":" type_name ] [ "=" expression ] ;

class_decl       = "class" identifier [ "<" identifier ] "{"
                   { method_decl | separator } "}" ;
method_decl      = [ visibility ] [ "static" ] function_decl ;
visibility       = "public" | "protected" | "private" ;

struct_decl      = "struct" identifier "{"
                   { struct_field | method_decl | separator } "}" ;
struct_field     = identifier [ ":" type_name ] [ "=" expression ] ;

import_decl      = "import" module_name [ "as" identifier ]
                 | "from" module_name "import" identifier
                   { "," identifier } ;

control_statement = if_statement | unless_statement | while_statement
                  | post_test_loop | for_statement | switch_statement
                  | "break" | "continue" | "return" [ expression ] ;
if_statement     = "if" expression block
                   { "else" "if" expression block }
                   [ "else" block ] ;
unless_statement = "unless" expression block [ "else" block ] ;
while_statement  = "while" expression block ;
post_test_loop   = ( "repeat" | "do" ) block "while" expression ;
for_statement    = ( "for" | "foreach" ) identifier [ "," identifier ]
                   "in" expression block ;
switch_statement = "switch" expression "{"
                   { "case" expression ":" statement }
                   [ "default" ":" statement ] "}" ;

exception_statement = "throw" expression
                    | "try" block
                      [ "catch" [ type_name ] [ "as" identifier ] block ]
                      [ "finally" block ] ;

block            = "{" { separator | statement } "}" ;
expression       = ternary_expression ;
ternary_expression = coalesce_expression
                     [ "?" expression ":" expression ] ;
coalesce_expression = logical_or_expression [ "??" coalesce_expression ] ;
logical_or_expression = logical_and_expression
                        { ( "or" | "||" ) logical_and_expression } ;
logical_and_expression = bit_or_expression
                         { ( "and" | "&&" ) bit_or_expression } ;
bit_or_expression  = bit_xor_expression { "|" bit_xor_expression } ;
bit_xor_expression = bit_and_expression { "^" bit_and_expression } ;
bit_and_expression = equality_expression { "&" equality_expression } ;
equality_expression = comparison_expression
                      { ( "==" | "!=" | "===" | "!==" ) comparison_expression } ;
comparison_expression = shift_expression
                         { ( "<" | "<=" | ">" | ">=" | "<=>" | "in" )
                           shift_expression } ;
shift_expression = range_expression { ( "<<" | ">>" ) range_expression } ;
range_expression = additive_expression [ ( ".." | "..." ) additive_expression ] ;
additive_expression = multiplicative_expression
                      { ( "+" | "-" ) multiplicative_expression } ;
multiplicative_expression = power_expression
                            { ( "*" | "/" | "%" ) power_expression } ;
power_expression = unary_expression [ "**" power_expression ] ;
unary_expression = { "!" | "not" | "~" | "+" | "-" | "++" | "--" }
                   postfix_expression ;
postfix_expression = primary
                     { call_suffix | member_suffix | index_suffix
                       | "++" | "--" } ;
call_suffix      = "(" [ arguments ] ")" ;
member_suffix    = "." identifier ;
index_suffix     = "[" expression "]" ;
primary          = literal | identifier | "self" | "super" "." identifier
                 | "(" expression ")" | array_literal | map_literal
                 | lambda_expression | backtick_command ;
lambda_expression = "lambda" [ "(" [ parameters ] ")" | identifier ]
                    "=>" expression ;
```

The parser accepts additional convenience forms (including command-style calls) where they can be parsed unambiguously. Do not infer arbitrary whitespace-insensitive syntax from this grammar.

## 4. Operator precedence and evaluation

From lower to higher precedence:

1. Ternary `?:`
2. Null coalescing `??`
3. Logical OR `or`, `||`
4. Logical AND `and`, `&&`
5. Bitwise OR `|`
6. Bitwise XOR `^`
7. Bitwise AND `&`
8. Equality `==`, `!=`, `===`, `!==`
9. Comparison `<`, `<=`, `>`, `>=`, `<=>`, `in`
10. Shift `<<`, `>>`
11. Range `..`, `...`
12. Addition/subtraction `+`, `-`
13. Multiplication/division/modulo `*`, `/`, `%`
14. Exponentiation `**`
15. Unary operators
16. Calls, member access, indexing, postfix increment/decrement

`and`/`or` and their symbolic aliases short-circuit. `??` evaluates its right operand only when the left side is nil or the supported missing-name case. Function arguments are evaluated before the call. Evaluation order for independent map entries is not a public sequencing guarantee.

## 5. Values, bindings, and type contracts

The runtime value categories are nil, boolean, integer, floating-point number, string, array, map, function, class, instance, and native object.

- `let` and `const` bindings are immutable bindings. `var` bindings are assignable.
- Type annotations are runtime contracts, not static compile-time type checking.
- Type contracts are checked for typed variable initialization/assignment, function parameters and return values, and typed struct fields.
- Built-in type names include `Any`, `Nil`, `Bool`, `Int`, `Double`/`Float`, `Number`, `String`, `Array`, `Map`, `Function`, `Class`, `Instance`, and `Native`; user-defined class names are accepted for instance contracts.
- `nil` is a value. A missing ordinary name raises `NameError`; the left operand of `??` has the documented missing-name fallback behavior.
- Truthiness, equality, indexing, iteration, and collection semantics are defined in `LANGUAGE_REFERENCE.md` and are part of this contract.

## 6. Functions and scope

Functions are first-class values. Calls support positional arguments, named arguments, defaults, and a final variadic parameter. Functions capture lexical environments by reference. Recursion is supported, but tail-call optimization is not guaranteed.

A block introduces a nested lexical environment according to the interpreter's block execution rules. `global` declarations target the interpreter's global environment. Function parameters and local declarations are scoped to the function environment.

## 7. Object model

- Classes support single inheritance.
- `Type.new(...)` creates an instance and invokes the available `initialize` constructor convention.
- Instance methods bind `self` to the receiver.
- `static func` declares class-level behavior.
- `super.method(...)` begins lookup at the parent of the class that defined the currently executing method.
- `public`, `protected`, and `private` visibility modifiers are supported for methods.
- Built-in types are represented by runtime classes that can be reopened to add methods. Reopening a built-in class adds methods; it does not add per-instance fields or replace primitive storage.
- Structs are class-like runtime objects with struct metadata; they are not a separate C++-style value representation.

Multiple inheritance, interfaces, traits/mixins, method overloading, abstract classes, and implicit `super` forwarding are not part of 2.0.7.

## 8. Modules, exceptions, and host interaction

Standard modules are loaded automatically by the interpreter; explicit imports remain supported. User modules are Lucy source files. Package resolution can locate project-local packages, but the runtime's package resolver does not imply a complete remote package manager or security sandbox.

Exceptions use textual error categories. `try/catch/finally` and `throw` are supported. The caught value is currently a string containing the diagnostic message; Lucy 2.0.7 does not expose a public custom exception-class hierarchy or structured exception object.

Backticks and `system` process APIs invoke host process facilities. Their availability and exact behavior depend on the operating system and configured runtime. They MUST NOT be treated as a security boundary. The interpreter is not generally thread-safe; no language-level async/await, scheduler, generator, or concurrency contract is provided.

## 9. Runtime and native extension boundary

The interpreter is implemented in C++17. The runtime uses C++ standard-library containers and smart pointers for values, classes, instances, functions, and environments. There is no language-level garbage collector or deterministic resource-management syntax in 2.0.7. The interpreter breaks registered environment references during shutdown; reference cycles and long-lived allocation behavior remain subjects for future memory-model work.

The C++ extension API exposes `ExtensionAPI`, `ExtensionModuleBuilder`, `NativeFunction`, and `NativeObject`. Its API version is currently `1`. This version identifies the declared extension interface; it is not a promise that arbitrary binaries remain ABI-compatible across all runtime builds. Extensions SHOULD be rebuilt against the exact Lucy runtime release they target.

Native extensions execute inside the host process and inherit its privileges. Loading an extension is equivalent to loading native code; only load trusted extensions.

## 10. Compatibility and versioning policy

For the 2.x line:

- Patch releases MUST preserve documented language semantics and public API behavior, except for correcting behavior that is demonstrably inconsistent with the published contract.
- Minor releases MAY add backward-compatible syntax, runtime APIs, and tooling.
- A breaking change to established syntax, semantics, runtime value representation, or native extension contract requires a major version or an explicitly documented migration path.
- A bug fix that changes behavior relied upon by existing programs MUST be recorded in the changelog and migration notes.
- Historical verification reports are immutable evidence; they MUST NOT be rewritten to claim validation on platforms that were not tested.

A release is not considered verified for an operating system/compiler combination until its build and test suite have actually run there.

## 11. Conformance

The conformance baseline consists of:

1. This specification.
2. The executable regression suite under `tests/`.
3. The standard-library manifest and API inventory.
4. The implementation and platform-specific verification records.

Every newly documented language feature MUST have at least one positive test. Every intentionally rejected syntax or type contract SHOULD have a negative test. Documentation examples MUST be labelled as executable, illustrative, expected-error, or environment-dependent.
