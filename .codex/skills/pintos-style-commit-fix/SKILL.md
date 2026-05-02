---
name: pintos-style-commit-fix
description: Review and safely fix Pintos style issues in a user-specified commit, commit range, staged diff, or worktree diff using docs/reference/code/CODING_CONVENTIONS_CHECKLIST.md. Use when asked to inspect recent Pintos changes, check coding conventions, or apply style-only fixes while avoiding assignment implementation changes unless the user explicitly approves them.
---

# Pintos Style Commit Fix

## Overview

Use this skill to review changed Pintos code against the repository's coding
convention checklist and apply only safe style/layout fixes. Treat Pintos
assignment logic as user-owned work: do not supply or rewrite implementation
logic.

## Workflow

1. Determine the target diff before reviewing:
   - Use an explicit commit hash, revision range, staged diff, or worktree diff
     if the user provided one.
   - If the user says only "recent commit" or "latest change" without a target,
     ask which commit/range/diff to inspect.
2. Read `docs/reference/code/CODING_CONVENTIONS_CHECKLIST.md`.
3. Inspect only changed hunks plus nearby context. Use the helper when useful:
   `python3 .codex/skills/pintos-style-commit-fix/scripts/style_diff_context.py --commit HEAD`
4. Classify each style issue as `layout-only` or `code-changing`.
5. Apply layout-only fixes directly when they are confined to the target diff.
6. For every code-changing fix, pause and ask the user before editing. Use the
   available Ask/user-input tool when present; otherwise ask in chat and wait.
7. Summarize what changed and name any remaining issues that require user
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
- Remove trailing whitespace or add a final newline.

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

If a style issue cannot be fixed without one of these changes, report the issue
and ask for approval with a concise proposed edit.

## Pintos Repository Rules

- Prefer local reference files and current repository code over external
  sources.
- Follow nearby style first, then the checklist.
- Keep formatting-only edits separate from logic edits.
- Do not run Pintos tests directly. If validation is needed, point the user to
  the relevant command in `TESTING.md`.
- Do not look up external Pintos solutions or copy implementation code.

## Helper Script

`scripts/style_diff_context.py` is read-only. It prints changed files and patch
context for exactly one target mode:

- `--commit <hash>`
- `--range <rev-range>`
- `--staged`
- `--worktree`

The script must never write files, run formatters, run tests, fetch network
data, or mutate git state.
