# `crypto` standard library — Lucy 2.0.2

Digest/HMAC utilities and cryptographic object facades. Verify backend availability before relying on a primitive.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import crypto
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
println crypto.hexdigest("hello")
println crypto.base64digest("hello")
println crypto.hmac("secret", "message")
```

## Exported module-level objects

`_digest`, `openssl`, `pkey`, `x509`

## Module-level functions

| Function | Signature |
|---|---|
| `digest` | `digest(text)` |
| `hexdigest` | `hexdigest(text)` |
| `hash` | `hash(text)` |
| `base64digest` | `base64digest(text)` |
| `file` | `file(path)` |
| `hmac` | `hmac(key, data)` |

## Classes and methods

### `Digest`

| Method | Signature |
|---|---|
| `digest` | `digest(text)` |
| `hexdigest` | `hexdigest(text)` |
| `base64digest` | `base64digest(text)` |
| `file` | `file(path)` |

### `HMAC`

| Method | Signature |
|---|---|
| `digest` | `digest(key, data)` |
| `hexdigest` | `hexdigest(key, data)` |

### `SSLContext`

| Method | Signature |
|---|---|
| `initialize` | `initialize(options = {})` |

### `SSLSocket`

| Method | Signature |
|---|---|
| `initialize` | `initialize(socket = nil, context = nil)` |
| `connect` | `connect(host, port)` |
| `close` | `close()` |

### `Certificate`

| Method | Signature |
|---|---|
| `initialize` | `initialize(data = "")` |

### `RSA`

| Method | Signature |
|---|---|
| `initialize` | `initialize(pem = "")` |

### `Cipher`

| Method | Signature |
|---|---|
| `initialize` | `initialize(name = "")` |

### `X509`

| Method | Signature |
|---|---|
| `certificate` | `certificate(data = "")` |

### `PKey`

| Method | Signature |
|---|---|
| `rsa` | `rsa(pem = "")` |

### `OpenSSL`

| Method | Signature |
|---|---|
| `hmac` | `hmac()` |
| `ssl_context` | `ssl_context(options = {})` |
| `ssl_socket` | `ssl_socket(socket = nil, context = nil)` |
| `certificate` | `certificate(data = "")` |
| `rsa` | `rsa(pem = "")` |
| `x509` | `x509()` |
| `pkey` | `pkey()` |
| `cipher` | `cipher(name = "")` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

## Source of truth

Implementation: [`stdlib/crypto.lucy`](../../stdlib/crypto.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
