#!/usr/bin/env python3
"""Generate source-derived per-module API indexes for Lucy's pure-LC standard library."""
from pathlib import Path
import re
ROOT = Path(__file__).resolve().parents[1]
STDLIB = ROOT / "stdlib"
OUT = ROOT / "docs" / "stdlib"
VERSION = (ROOT / "VERSION").read_text(encoding="utf-8").strip()
DESCRIPTIONS = {
    "app": "Command-line option parsing, logging facade, callable benchmarking, post-hoc duration checks, and templates.",
    "crypto": "Digest/HMAC utilities and cryptographic object facades. Verify backend availability before relying on a primitive.",
    "data": "Map transformations plus JSON, YAML, and CSV serialization/parsing.",
    "flow": "Small functional composition helpers for callbacks and transformations.",
    "fs": "Filesystem operations, path helpers, text/JSON convenience methods, and IO objects.",
    "http": "HTTP request helpers, client objects, sockets, DNS resolution, and reverse lookup.",
    "math": "Mathematical constants/functions and common numeric helpers.",
    "random": "Random scalar values and array selection/shuffling helpers.",
    "repl": "Plain-text REPL banner/prompt, command/topic metadata, and topic help.",
    "result": "Small success/error value conventions and unwrap helpers.",
    "runtime": "Kernel/collection facades, formatting, guarded callback helpers, and reflection/dynamic invocation.",
    "set": "Set construction, membership, and set algebra operations.",
    "sqlite": "SQLite database and prepared-statement wrappers; requires a build with SQLite enabled.",
    "system": "Platform/environment inspection, process execution, identity, signals, clocks, and shell helpers.",
    "text": "Regular expressions, Base64/hex/URL encoding, shell escaping, and string scanner.",
    "time": "Time/date/date-time objects, parsing, formatting, timestamps, and sleeping.",
}
EXAMPLES = {
    "app": 'let parser = app.parser()\nparser.on("output", "output path", "report.csv")\nlet options = parser.parse(ARGV)\nprintln options.get("output")\n\nlet elapsed = app.benchmark().realtime(lambda() => 1 + 1)\nprintln elapsed',
    "crypto": 'println crypto.hexdigest("hello")\nprintln crypto.base64digest("hello")\nprintln crypto.hmac("secret", "message")',
    "data": 'let value = data.json.parse("{\\"name\\":\\"Lucy\\"}")\nprintln data.json.pretty(value)\nlet rows = data.csv_parse("name,age\\nLucy,2")\nprintln data.csv_stringify(rows)',
    "flow": 'let answer = flow.pipe(5, [lambda x => x * 2, lambda x => x + 1])\nprintln answer\nflow.tap(answer, lambda x => println x)',
    "fs": 'fs.write("example.txt", "hello\\n")\nfs.append("example.txt", "Lucy\\n")\nprintln fs.read("example.txt")\nprintln fs.basename("src/lucy.cpp")',
    "http": 'let response = http.get("https://example.com")\nprintln response.status\nprintln response.ok',
    "math": 'println math.square(9)\nprintln math.clamp(12, 0, 10)\nprintln math.gcd(24, 18)',
    "random": 'println random.int(1, 6)\nprintln random.choice(["red", "green", "blue"])',
    "repl": 'println repl.version()\nprintln repl.commands()\nprintln repl.topics()',
    "result": 'let outcome = result.ok(42)\nif result.success(outcome) {\n    println result.unwrap(outcome)\n}',
    "runtime": 'println runtime.inspect([1, 2, 3])\nprintln runtime.methods("Lucy")\nprintln runtime.responds("Lucy", "upper")',
    "set": 'let left = set.from_values([1, 2, 3])\nlet right = set.from_values([3, 4])\nprintln set.union(left, right)',
    "sqlite": 'let db = sqlite.open("example.db")\ndb.execute("CREATE TABLE IF NOT EXISTS items (id INTEGER PRIMARY KEY, name TEXT)")\ndb.execute("INSERT INTO items (name) VALUES (?)", ["Lucy"])\nprintln db.query("SELECT id, name FROM items")\ndb.close()',
    "system": 'println system.platform()\nprintln system.cwd()\nprintln system.env("HOME")',
    "text": 'println text.match("^lucy", "lucy-lang")\nprintln text.hex_encode("Lucy")\nprintln text.url_encode("hello world")',
    "time": 'println time.now()\nprintln time.today()\nprintln time.format(time.timestamp())',
}

def parse_api(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    classes = []
    funcs = []
    exports = []
    current = None
    for line in lines:
        m = re.match(r'^class\s+([A-Za-z_][\w]*)\s*\{', line)
        if m:
            current = {"name": m.group(1), "methods": []}
            classes.append(current)
            continue
        m = re.match(r'^func\s+([A-Za-z_][\w?!]*)\s*(\([^)]*\))', line)
        if m:
            funcs.append((m.group(1), m.group(2)))
            current = None
            continue
        m = re.match(r'^\s{4}func\s+([A-Za-z_][\w?!]*)\s*(\([^)]*\))', line)
        if m and current:
            current["methods"].append((m.group(1), m.group(2)))
        if re.match(r'^[A-Za-z_][\w]*\s*=\s*', line):
            exports.append(line.split("=",1)[0].strip())
    return classes, funcs, sorted(set(exports))

for path in sorted(STDLIB.glob("*.lucy")):
    name = path.stem
    if name.startswith("_"):
        continue
    classes, funcs, exports = parse_api(path)
    title = f"# `{name}` standard library — Lucy {VERSION}"
    content = [title, "", DESCRIPTIONS.get(name, "Standard-library module."), "", 
        "## Availability and imports", "", 
        "The shipped module is loaded automatically in Lucy 2.x. Explicit `import` remains available when you want to make dependencies visible:", "", 
        f"```lucy\nimport {name}\n```", "", 
        "Names beginning with `_` or `__` are implementation details unless a public API explicitly documents them.", ""]
    content += ["## Quick example", "", "```lucy", EXAMPLES.get(name, f"println {name}"), "```", ""]
    if exports:
        content += ["## Exported module-level objects", "", ", ".join(f"`{x}`" for x in exports), ""]
    content += ["## Module-level functions", ""]
    if funcs:
        content += ["| Function | Signature |", "|---|---|"]
        content += [f"| `{fn}` | `{fn}{args}` |" for fn,args in funcs]
    else:
        content += ["No top-level functions are declared in the pure-Lucy source. The module may export class instances or rely on native runtime operations."]
    content += ["", "## Classes and methods", ""]
    if classes:
        for cls in classes:
            content += [f"### `{cls['name']}`", ""]
            if cls["methods"]:
                content += ["| Method | Signature |", "|---|---|"]
                content += [f"| `{fn}` | `{fn}{args}` |" for fn,args in cls["methods"]]
            else:
                content += ["No methods were found by the source indexer."]
            content.append("")
    else:
        content += ["No public class declarations were found in this module's pure-Lucy source.", ""]
    content += ["## Contract and error handling", "", 
        "Validate inputs at the application boundary. Runtime errors are reported as Lucy exceptions or native errors; exact messages may vary by platform and build. Do not assume network access, process privileges, filesystem permissions, or optional native dependencies are available.", ""]
    if name == "app":
        content += ["### Important behavioral contracts", "", 
          "- `OptionParser` accepts `--name value` and `--name=value`; boolean options are declared by setting a boolean default. Unknown flags, missing values, and positional arguments are errors. Short aliases, automatic type conversion, and required-option metadata are not implemented.",
          "- `Benchmark.realtime(callback)` and `Benchmark.measure(callback)` time a Lucy callback without launching a shell. The timer has millisecond resolution.",
          "- `Timeout.timeout(seconds, callback)` checks elapsed time only after the callback returns. It cannot cancel a blocked or infinite callback.",
          "- `Logger` is a lightweight facade and should not be assumed to provide a full logging backend or structured sink without inspecting the current implementation.", ""]
    if name == "sqlite":
        content += ["### Build requirement", "", "Configure with `-DLUCY_ENABLE_SQLITE=ON`. A build without this option cannot open databases. Use prepared-statement placeholders for values and close connections/statements explicitly.", ""]
    if name == "http":
        content += ["### Network cautions", "", "HTTP, sockets, DNS, and TLS depend on the host environment. Set explicit request timeouts where supported, validate response status/content, and do not treat examples using public websites as deterministic tests.", ""]
    content += ["## Source of truth", "", f"Implementation: [`stdlib/{name}.lucy`](../../stdlib/{name}.lucy). The signatures above are indexed from the current source file. Behavior is ultimately defined by implementation and regression tests; if you change the source API, regenerate this page and add tests for the new contract.", ""]
    (OUT / f"{name}.md").write_text("\n".join(content), encoding="utf-8")
