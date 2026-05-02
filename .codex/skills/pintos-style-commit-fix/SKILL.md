---
name: pintos-style-commit-fix
description: Review and safely fix Pintos style issues in changed .c and .h files from a user-specified commit, commit range, staged diff, or worktree diff using docs/reference/code/CODING_CONVENTIONS_CHECKLIST.md. Use when asked to inspect recent Pintos C/header changes, check coding conventions, or apply style-only fixes while avoiding assignment implementation changes unless the user explicitly approves them.
---

# Pintos Style Commit Fix

## Overview

Use this skill to review changed Pintos C source/header files against the
repository's coding convention checklist and apply only safe style/layout
fixes. Treat Pintos assignment logic as user-owned work: do not supply or
rewrite implementation logic.

The default edit scope is only files ending in `.c` or `.h`. Do not edit
Markdown, docs, scripts, skill files, config files, or other non-C/header files
unless the user explicitly asks for those files.

## Workflow

1. Determine the target diff before reviewing:
   - Use an explicit commit hash, revision range, staged diff, or worktree diff
     if the user provided one.
   - If the user says only "recent commit" or "latest change" without a target,
     ask which commit/range/diff to inspect.
2. Read `docs/reference/code/CODING_CONVENTIONS_CHECKLIST.md`.
3. Build the review file set by filtering the target diff to paths ending in
   `.c` or `.h`. If no `.c` or `.h` files changed, report that there is no
   in-scope style target.
4. Inspect only changed `.c`/`.h` hunks plus nearby context. Use the helper when
   useful, but ignore non-C/header files in its output:
   `python3 .codex/skills/pintos-style-commit-fix/scripts/style_diff_context.py --commit HEAD`
5. Classify each style issue as `layout-only` or `code-changing`.
6. Apply layout-only fixes directly when they are confined to in-scope `.c`/`.h`
   hunks in the target diff.
7. For every code-changing fix, pause and ask the user before editing. Use the
   available Ask/user-input tool when present; otherwise ask in chat and wait.
8. Summarize what changed and name any remaining issues that require user
   approval.

## Safe Auto-Fixes

Apply these without asking when they preserve tokens and behavior:

- Replace leading spaces with tabs for indentation.
- Adjust whitespace around function calls, declarations, and control keywords.
- Move `.c` function definition return types to the line above the function
  name.
- Adjust pointer declaration spacing, brace placement, and `} else {` layout.
- Rewrap comments or code to stay near 80 columns when no words, tokens,
  string contents, or semantics change.
- Remove trailing whitespace or add a final newline in `.c`/`.h` files only.

## Ask Before Editing

Ask every time before making a change that could alter code meaning or Pintos
assignment implementation, including:

- Rename identifiers, macros, enum values, fields, or functions.
- Change types, signatures, constants, string literals, macros, or include
  directives.
- Add, remove, reorder, or move statements.
- Change conditionals, loops, expressions, pointer arithmetic, memory layout, or
  synchronization behavior.
- Add substantive comments, remove meaningful comments, or rewrite comments in
  a way that changes their meaning.
- Touch files outside the target diff.
- Touch files that do not end in `.c` or `.h`, including Markdown files, docs,
  scripts, skill files, and config files.

If a style issue cannot be fixed without one of these changes, report the issue
and ask for approval with a concise proposed edit.

## Pintos Repository Rules

- Prefer local reference files and current repository code over external
  sources.
- Follow nearby style first, then the checklist.
- Keep formatting-only edits separate from logic edits.
- Run `git diff --check` path-limited to `.c`/`.h` files when possible. If a
  whole-diff check reports whitespace issues in non-C/header files, leave those
  files unchanged and mention they are outside this skill's default scope.
- Do not run Pintos tests directly. If validation is needed, point the user to
  the relevant command in `TESTING.md`.
- Do not look up external Pintos solutions or copy implementation code.

## Helper Script

`scripts/style_diff_context.py` is read-only. By default, it prints changed
`.c`/`.h` files and patch context for exactly one target mode:

- `--commit <hash>`
- `--range <rev-range>`
- `--staged`
- `--worktree`

Use `--all-files` only when the user explicitly asks to inspect non-C/header
files.

The script must never write files, run formatters, run tests, fetch network
data, or mutate git state.
