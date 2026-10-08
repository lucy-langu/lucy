# `crypto` standard library

Digest and HMAC helpers.

## Import

```lucy
import crypto
```

## Public functions

- `digest(text)`
- `hexdigest(text)`
- `hash(text)`
- `base64digest(text)`
- `file(path)`
- `hmac(key, data)`

## Example

```lucy
import crypto
println crypto.hexdigest "hello"
```

## Source of truth

The public function list above is extracted from the current `stdlib/crypto.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
