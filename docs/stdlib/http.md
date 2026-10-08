# `http` standard library

HTTP, TCP socket, and DNS helpers.

## Import

```lucy
import http
```

## Public functions

- `request(method, url, body = "", headers = nil, options = nil)`
- `get(url, headers = nil, options = nil)`
- `post(url, body = "", headers = nil, options = nil)`
- `put(url, body = "", headers = nil, options = nil)`
- `patch(url, body = "", headers = nil, options = nil)`
- `delete(url, headers = nil, options = nil)`
- `head(url, headers = nil, options = nil)`
- `client(url = "", proxy = nil)`
- `connect(host, port)`
- `bind(host, port)`
- `listen(socket, backlog = 16)`
- `accept(socket)`
- `recv(socket, size = 4096)`
- `send(socket, data)`
- `close(socket)`
- `resolve(host)`
- `reverse(address)`

## Example

```lucy
import http
# Network access depends on the target environment.
println http.resolve "example.com"
```

## Source of truth

The public function list above is extracted from the current `stdlib/http.lucy` implementation. Internal runtime bridge functions are not public module functions. See `INTERNAL_API.md` and `RUNTIME_CPP.md` for the native boundary.
