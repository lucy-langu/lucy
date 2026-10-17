# `http` standard library — Lucy 2.0.1

HTTP request helpers, client objects, sockets, DNS resolution, and reverse lookup.

## Availability and imports

The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:

```lucy
import http
```

Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.

## Quick example

```lucy
let response = http.get("https://example.com")
println response.status
println response.ok
```

## Exported module-level objects

`http`, `resolv`, `socket`

## Module-level functions

| Function | Signature |
|---|---|
| `request` | `request(method, url, body = "", headers = nil, options = nil)` |
| `get` | `get(url, headers = nil, options = nil)` |
| `post` | `post(url, body = "", headers = nil, options = nil)` |
| `put` | `put(url, body = "", headers = nil, options = nil)` |
| `patch` | `patch(url, body = "", headers = nil, options = nil)` |
| `delete` | `delete(url, headers = nil, options = nil)` |
| `head` | `head(url, headers = nil, options = nil)` |
| `client` | `client(url = "", proxy = nil)` |
| `connect` | `connect(host, port)` |
| `bind` | `bind(host, port)` |
| `listen` | `listen(socket, backlog = 16)` |
| `accept` | `accept(socket)` |
| `recv` | `recv(socket, size = 4096)` |
| `send` | `send(socket, data)` |
| `close` | `close(socket)` |
| `resolve` | `resolve(host)` |
| `reverse` | `reverse(address)` |

## Classes and methods

### `HTTPRequest`

| Method | Signature |
|---|---|
| `initialize` | `initialize(method, url, body = "", headers = nil, options = nil)` |
| `execute` | `execute()` |

### `HTTP`

| Method | Signature |
|---|---|
| `initialize` | `initialize(url = "", proxy = nil)` |
| `headers` | `headers(value)` |
| `timeout` | `timeout(seconds)` |
| `connect_timeout` | `connect_timeout(seconds)` |
| `proxy` | `proxy(proxy_url)` |
| `user_agent` | `user_agent(value)` |
| `follow_redirects` | `follow_redirects(value)` |
| `insecure` | `insecure(value = true)` |
| `get` | `get(path = "")` |
| `post` | `post(path = "", body = "")` |
| `put` | `put(path = "", body = "")` |
| `patch` | `patch(path = "", body = "")` |
| `delete` | `delete(path = "")` |
| `head` | `head(path = "")` |
| `request` | `request(method, path = "", body = "", headers = nil)` |

### `Socket`

| Method | Signature |
|---|---|
| `initialize` | `initialize()` |
| `connect` | `connect(host, port)` |
| `bind` | `bind(host, port)` |
| `listen` | `listen(backlog = 16)` |
| `accept` | `accept()` |
| `recv` | `recv(size = 4096)` |
| `send` | `send(data)` |
| `close` | `close()` |

### `SocketAPI`

| Method | Signature |
|---|---|
| `new` | `new()` |

### `Resolv`

| Method | Signature |
|---|---|
| `getaddress` | `getaddress(host)` |
| `getname` | `getname(address)` |

## Contract and error handling

Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.

### Network cautions

HTTP, sockets, DNS, and TLS depend on the host environment. Set explicit request timeouts where supported, validate response status/content, and do not treat examples using public websites as deterministic tests.

## Source of truth

Implementation: [`stdlib/http.lucy`](../../stdlib/http.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.
