#!/usr/bin/env python3
"""Print read-only diff context for Pintos style review."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def run_git(args: list[str], repo: Path) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=repo,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)
    return result.stdout


def repo_root() -> Path:
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)
    return Path(result.stdout.strip())


def reject_option_like(value: str, label: str) -> None:
    if value.startswith("-"):
        raise SystemExit(f"{label} must be a revision value, not an option: {value}")


def pathspec_args(args: argparse.Namespace) -> list[str]:
    if args.all_files:
        return []
    return ["--", "*.c", "*.h"]


def build_commands(args: argparse.Namespace) -> tuple[list[str], list[str], str]:
    common = ["--no-ext-diff", "--find-renames", "--find-copies"]
    unified = f"--unified={args.context}"
    pathspec = pathspec_args(args)
    scope = "all files" if args.all_files else ".c/.h files"

    if args.commit:
        reject_option_like(args.commit, "--commit")
        title = f"commit {args.commit} ({scope})"
        names = ["show", "--format=", "--name-status", *common, args.commit, *pathspec]
        patch = [
            "show",
            "--format=medium",
            "--stat",
            "--patch",
            unified,
            *common,
            args.commit,
            *pathspec,
        ]
        return names, patch, title

    if args.range:
        reject_option_like(args.range, "--range")
        title = f"range {args.range} ({scope})"
        names = ["diff", "--name-status", *common, args.range, *pathspec]
        patch = ["diff", "--stat", "--patch", unified, *common, args.range, *pathspec]
        return names, patch, title

    if args.staged:
        title = f"staged diff ({scope})"
        names = ["diff", "--cached", "--name-status", *common, *pathspec]
        patch = ["diff", "--cached", "--stat", "--patch", unified, *common, *pathspec]
        return names, patch, title

    title = f"worktree diff ({scope})"
    names = ["diff", "--name-status", *common, *pathspec]
    patch = ["diff", "--stat", "--patch", unified, *common, *pathspec]
    return names, patch, title


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Print changed files and unified diff context without mutating git state.",
    )
    target = parser.add_mutually_exclusive_group(required=True)
    target.add_argument("--commit", help="single commit to inspect, e.g. HEAD or 56f5aaa")
    target.add_argument("--range", help="revision range to inspect, e.g. main..HEAD")
    target.add_argument("--staged", action="store_true", help="inspect staged changes")
    target.add_argument("--worktree", action="store_true", help="inspect unstaged changes")
    parser.add_argument(
        "--context",
        type=int,
        default=6,
        help="unified diff context lines to print, default: 6",
    )
    parser.add_argument(
        "--all-files",
        action="store_true",
        help="include every changed file instead of the default .c/.h filter",
    )
    args = parser.parse_args(argv)
    if args.context < 0:
        parser.error("--context must be non-negative")
    return args


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    repo = repo_root()
    names_cmd, patch_cmd, title = build_commands(args)

    names = run_git(names_cmd, repo).strip()
    patch = run_git(patch_cmd, repo).rstrip()

    print(f"# Target: {title}")
    print(f"# Repository: {repo}")
    print("\n## Changed Files")
    print(names if names else "(none)")
    print("\n## Diff")
    print(patch if patch else "(no diff)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
