---
name: pintos-style-commit-fix
description: Review and safely fix Pintos style issues in changed .c and .h files from a user-specified commit, commit range, staged diff, or worktree diff using docs/reference/code/CODING_CONVENTIONS_CHECKLIST.md. Use when asked to inspect recent Pintos C/header changes, check coding conventions, apply layout-only fixes, or propose/apply explicitly approved code-style fixes while avoiding assignment implementation changes.
---

# Pintos Style Commit Fix

## Overview

Use this skill to review changed Pintos C source/header files against the
repository's coding convention checklist. Apply safe layout-only fixes
directly. Also detect code-style issues that require code edits, such as
identifier renames or comment rewrites, and either ask for approval or apply
them when the user has explicitly authorized that category. Treat Pintos
assignment logic as user-owned work: do not supply or rewrite implementation
logic.

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
5. Classify each style issue as `layout-only`, `code-style`, or
   `implementation-changing`.
6. Apply layout-only fixes directly when they are confined to in-scope `.c`/`.h`
   hunks in the target diff.
7. Do not silently skip `code-style` issues. If the user already gave explicit
   approval for code-style edits, apply them within the target diff. Otherwise
   ask with a concise proposed edit, wait, then apply approved fixes.
8. For every `implementation-changing` fix, pause and ask the user before
   editing. Use chat and wait for approval.
9. Summarize what changed and name any remaining issues that require user
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

## Code-Style Fixes

Treat these as style issues, but ask before editing unless the user explicitly
authorized code-style edits in the current request:

- Rename unclear changed identifiers to clearer Pintos-style names, then update
  all in-scope uses in changed `.c`/`.h` files. Examples: avoid ad hoc
  abbreviations such as `rtn_val`; prefer `retval` when it matches nearby
  Pintos style, or `return_value` when clarity is better.
- Rewrite changed comments to match Pintos comment style without changing the
  technical meaning.
- Remove or revise TODO markers, temporary comments, or redundant comments only
  when the user approves and the meaning is preserved.

When asking, include the exact issue code when useful, the affected identifier
or comment, and the proposed replacement. Example: `NAME-010: rename rtn_val to
return_value and update its local uses?`

## Comment Details

- Use `/* ... */` block comments for public function, struct, and longer
  comments.
- For changed multi-line comments, avoid leading `*` on continuation lines when
  applying Pintos style; align continuation text with the first line's comment
  text.
- Keep one-line trailing comments only when they are short and match nearby
  style; otherwise move them to a block comment near the code.

## Ask Before Implementation Changes

Ask every time before making a change that could alter code meaning or Pintos
assignment implementation, including:

- Change types, signatures, constants, string literals, macros, or include
  directives.
- Add, remove, reorder, or move statements.
- Change conditionals, loops, expressions, pointer arithmetic, memory layout, or
  synchronization behavior.
- Add substantive comments, remove meaningful comments, or rewrite comments in a
  way that changes their meaning.
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
