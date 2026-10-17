#!/usr/bin/env python3
"""Run source-indexed standard-library quick examples against a built Lucy executable."""
from pathlib import Path
import os
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
EXECUTABLE = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / "build" / "lucy"
QUICK_EXAMPLE = re.compile(r"## Quick example\s+```lucy\n(.*?)\n```", re.S)
NETWORK_DEPENDENT = {"http"}

if not EXECUTABLE.is_file():
    raise SystemExit(f"Lucy executable not found: {EXECUTABLE}")

env = os.environ.copy()
env["LUCY_PATH"] = str(ROOT / "stdlib")
failed = []
passed = []
skipped = []

for doc in sorted((ROOT / "docs" / "stdlib").glob("*.md")):
    name = doc.stem
    match = QUICK_EXAMPLE.search(doc.read_text(encoding="utf-8"))
    if not match:
        continue
    if name in NETWORK_DEPENDENT:
        skipped.append(name)
        continue
    with tempfile.TemporaryDirectory(prefix="lucy-doc-example-") as temporary:
        source = Path(temporary) / f"{name}.lucy"
        source.write_text(match.group(1) + "\n", encoding="utf-8")
        try:
            result = subprocess.run(
                [str(EXECUTABLE), str(source)], cwd=temporary, env=env,
                text=True, capture_output=True, timeout=15, check=False,
            )
        except subprocess.TimeoutExpired:
            failed.append((name, "timed out after 15 seconds"))
            continue
        if result.returncode:
            detail = (result.stderr or result.stdout).strip()
            failed.append((name, detail or f"exit status {result.returncode}"))
        else:
            passed.append(name)

for name in passed:
    print(f"PASS docs/stdlib/{name}.md quick example")
for name in skipped:
    print(f"SKIP docs/stdlib/{name}.md quick example (network-dependent)")
for name, detail in failed:
    print(f"FAIL docs/stdlib/{name}.md quick example: {detail}", file=sys.stderr)

builtin_example_passed = False
builtin_doc = ROOT / "docs" / "BUILTIN_REFERENCE.md"
builtin_match = re.search(r"## Examples\s+```lucy\n(.*?)\n```", builtin_doc.read_text(encoding="utf-8"), re.S)
if builtin_match:
    with tempfile.TemporaryDirectory(prefix="lucy-doc-example-") as temporary:
        source = Path(temporary) / "builtins.lucy"
        source.write_text(builtin_match.group(1) + "\n", encoding="utf-8")
        try:
            result = subprocess.run(
                [str(EXECUTABLE), str(source)], cwd=temporary, env=env,
                text=True, capture_output=True, timeout=15, check=False,
            )
            if result.returncode:
                detail = (result.stderr or result.stdout).strip() or f"exit status {result.returncode}"
                failed.append(("BUILTIN_REFERENCE", detail))
                print(f"FAIL docs/BUILTIN_REFERENCE.md example: {detail}", file=sys.stderr)
            else:
                builtin_example_passed = True
                print("PASS docs/BUILTIN_REFERENCE.md example")
        except subprocess.TimeoutExpired:
            failed.append(("BUILTIN_REFERENCE", "timed out after 15 seconds"))
            print("FAIL docs/BUILTIN_REFERENCE.md example: timed out after 15 seconds", file=sys.stderr)

passed_count = len(passed) + (1 if builtin_example_passed else 0)
print(f"Documentation examples: {passed_count} passed, {len(skipped)} skipped, {len(failed)} failed")
raise SystemExit(1 if failed else 0)
