#!/usr/bin/env python3
"""Inspect and safely update Lucy's per-module stdlib version metadata."""
import argparse
import json
import re
import sys
import shutil
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "stdlib" / "manifest.json"
SEMVER = re.compile(r"^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$")


def version_tuple(version):
    if not SEMVER.fullmatch(version):
        raise ValueError(f"invalid semantic version {version!r}")
    return tuple(int(part) for part in version.split("."))


def satisfies(version, expression):
    current = version_tuple(version)
    for raw in expression.split(","):
        raw = raw.strip()
        match = re.fullmatch(r"(>=|<=|>|<|==|=)?\s*(\d+\.\d+\.\d+)", raw)
        if not match:
            raise ValueError(f"unsupported version constraint {raw!r}")
        operator, wanted_text = match.groups()
        wanted = version_tuple(wanted_text)
        operator = operator or "=="
        if not {">=": current >= wanted, "<=": current <= wanted, ">": current > wanted,
                "<": current < wanted, "==": current == wanted, "=": current == wanted}[operator]:
            return False
    return True

def read_manifest():
    try:
        data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise SystemExit(f"stdlib manifest error: {exc}")
    if data.get("schema_version") != 1 or not isinstance(data.get("modules"), dict):
        raise SystemExit("stdlib manifest error: unsupported schema or missing modules")
    return data


def verify(data):
    failures = []
    for name, item in sorted(data["modules"].items()):
        version = item.get("version", "")
        entry = item.get("entry", "")
        if not SEMVER.fullmatch(version):
            failures.append(f"{name}: invalid semantic version {version!r}")
        if Path(entry).name != entry or not entry.endswith(".lucy"):
            failures.append(f"{name}: entry must be a simple .lucy filename")
        elif not (MANIFEST.parent / entry).is_file():
            failures.append(f"{name}: missing entry file {entry}")
        if item.get("stability") not in {"stable", "preview", "optional", "deprecated"}:
            failures.append(f"{name}: invalid stability label")
        constraints = item.get("dependency_constraints", {})
        if not isinstance(constraints, dict):
            failures.append(f"{name}: dependency_constraints must be an object")
        else:
            for dep, constraint in constraints.items():
                if dep not in data["modules"]:
                    failures.append(f"{name}: dependency constraint references unknown module {dep!r}")
                elif not isinstance(constraint, str) or not constraint.strip():
                    failures.append(f"{name}: dependency constraint for {dep!r} must be a non-empty string")
                else:
                    try:
                        if not satisfies(data["modules"][dep]["version"], constraint):
                            failures.append(f"{name}: {dep} {data['modules'][dep]['version']} does not satisfy {constraint!r}")
                    except ValueError as exc:
                        failures.append(f"{name}: invalid dependency constraint for {dep!r}: {exc}")
    if failures:
        print("Standard-library manifest verification failed:", file=sys.stderr)
        print("\n".join(f" - {failure}" for failure in failures), file=sys.stderr)
        return 1
    print(f"Verified {len(data['modules'])} standard-library module records.")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("list", help="list module versions and stability")
    sub.add_parser("verify", help="verify manifest schema, versions, and files")
    bump = sub.add_parser("set-version", help="set one module's independent version")
    bump.add_argument("module")
    bump.add_argument("version")
    install = sub.add_parser("install-source", help="atomically update one module from a validated Lucy source file")
    install.add_argument("module")
    install.add_argument("version")
    install.add_argument("source", type=Path)
    install.add_argument("--allow-downgrade", action="store_true")
    args = parser.parse_args()
    data = read_manifest()
    if args.command == "list":
        for name, item in sorted(data["modules"].items()):
            print(f"{name:10} {item['version']:8} {item['stability']:10} {item['entry']}")
        return 0
    if args.command == "verify":
        return verify(data)
    if args.command == "install-source":
        if args.module not in data["modules"]:
            parser.error(f"unknown stdlib module: {args.module}")
        if not SEMVER.fullmatch(args.version):
            parser.error("version must use semantic versioning: MAJOR.MINOR.PATCH")
        item = data["modules"][args.module]
        old_version = item["version"]
        if version_tuple(args.version) < version_tuple(old_version) and not args.allow_downgrade:
            parser.error(f"refusing downgrade from {old_version} to {args.version}; pass --allow-downgrade explicitly")
        if not args.source.is_file():
            parser.error(f"source file does not exist: {args.source}")
        entry_path = MANIFEST.parent / item["entry"]
        new_bytes = args.source.read_bytes()
        if not new_bytes.strip():
            parser.error("refusing to install an empty module source")
        manifest_bytes = MANIFEST.read_bytes()
        old_bytes = entry_path.read_bytes()
        item["version"] = args.version
        try:
            with tempfile.NamedTemporaryFile(dir=entry_path.parent, delete=False) as staged:
                staged.write(new_bytes); staged_path = Path(staged.name)
            staged_path.replace(entry_path)
            MANIFEST.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
            if verify(data) != 0:
                raise RuntimeError("post-install manifest verification failed")
        except Exception as exc:
            entry_path.write_bytes(old_bytes)
            MANIFEST.write_bytes(manifest_bytes)
            print(f"stdlib update rolled back: {exc}", file=sys.stderr)
            return 1
        print(f"Installed {args.module} {old_version} -> {args.version} at {entry_path}.")
        print("Note: this transaction validates manifest/file integrity, not Lucy source semantics or API compatibility.")
        return 0
    if args.command == "set-version":
        if args.module not in data["modules"]:
            parser.error(f"unknown stdlib module: {args.module}")
        if not SEMVER.fullmatch(args.version):
            parser.error("version must use semantic versioning: MAJOR.MINOR.PATCH")
        data["modules"][args.module]["version"] = args.version
        MANIFEST.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"Updated {args.module} to {args.version}; language version remains {data['language_version']}.")
        print("Review the module's API contract, tests, and changelog before publishing.")
        return verify(data)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
