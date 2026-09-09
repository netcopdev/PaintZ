# PaintZ Codex Instructions

Before modifying, creating, deleting, renaming, generating, rebasing, or moving any repository work, you MUST read `docs/CODEX_PROJECT_RULES.md` completely and follow it together with this file.

## Mandatory Git and dependency preflight

Before any repository change, inspect:

```text
git branch --show-current
git status --short
git log -1 --oneline
git branch -a
```

Identify the branch that the requested work actually depends on before editing. Do not assume `main` is the correct base when the task explicitly extends unmerged PaintZ work.

`main` is the authoritative shared integration base. New independent work should normally branch from current `main`. Base new work on another feature branch only when it explicitly depends on unmerged work from that branch; once that dependency is merged, `main` supersedes it as the shared base.

## Shared infrastructure must live on the dependency base

Shared repository infrastructure includes, but is not limited to:

- build, pack, release and signing helpers;
- sandbox/test launchers and common smoke-test infrastructure;
- CI/workflow support;
- shared developer tooling;
- repository-wide ignore/safety rules;
- repository-wide Codex/AGENTS instructions.

Do not leave required shared infrastructure stranded on a sibling feature branch.

If a new task needs shared tooling that currently exists only on another sibling branch:

1. stop feature work;
2. promote/integrate that shared tooling into the common dependency base first;
3. update or recreate dependent feature branches from that base;
4. only then continue feature-specific work.

Likewise, if work begun on a feature branch turns into generally required shared infrastructure, promote it to the dependency base before creating another sibling feature that needs it.

A feature branch may intentionally extend another feature branch only when that dependency is explicit. Otherwise sibling feature branches must derive from the same current dependency base.

Never solve branch dependency mistakes by silently duplicating shared files across siblings, dropping user work, or resetting unrelated changes. Reorganize history so there is one authoritative shared baseline.

## Enforce Script parser-safety rules

DayZ Enforce Script is not C, C++ or C#. Do not apply formatting conventions from those languages unless the current DayZ compiler is known to accept them.

For every `.c` file under DayZ script layers:

- Keep function and method invocations on one physical line. Do not vertically split an invocation's argument list across multiple lines. This is especially mandatory for calls with four or more arguments and for calls involving `out` / `inout` parameters.
- If a call becomes too long, assign complex expressions to local variables first and then call the function on one line.
- Do not place comments inside an invocation argument list or inside a chained method call.
- Prefer simple call arguments. Precompute arithmetic, casts, ternaries, formatting expressions and other nontrivial expressions before passing them to a function when practical.
- For diagnostic logging, prefer explicit local values plus `.ToString()` / string concatenation over multiline `string.Format(...)` calls.
- Do not infer Enforce syntax validity from brace balancing, Python parsing, JSON validation or visual inspection. Those checks do not compile Enforce Script.
- When adding or changing a nontrivial Enforce API usage, inspect current `BohemiaInteractive/DayZ-Script-Diff` examples first and mirror proven syntax.
- Before declaring Enforce work complete, inspect every changed `.c` file for invocation lines ending immediately after `(`. Any such match must be reviewed and, unless it is a function declaration rather than a call, rewritten to a single-line invocation.
- A useful review command is `rg -n '^\s*[A-Za-z_][A-Za-z0-9_\.]*\s*\(\s*$' Scripts -g '*.c'`. Treat matches as review candidates, not automatic errors, because declarations can also match.
- If a real DayZ Tools/server compile is unavailable, state that explicitly. Never describe the Enforce code as compile-tested when it has only passed static or generator checks.
- When server logs report a PaintZ syntax error, fix and explain the first PaintZ compiler error before reasoning about later cascade errors from other files/mods.

These rules are mandatory because the DayZ parser can emit misleading `Missing ';'`, `Expected ',' or ')'`, `Invalid statement`, `Syntax error`, and `Unexpected scope` cascades after one malformed invocation. Avoid parser-sensitive formatting rather than patching those downstream diagnostics one at a time.

## Scope

The detailed PaintZ architecture, persistence, policy, model-safety, networking, testing and coding rules are in `docs/CODEX_PROJECT_RULES.md`. They are mandatory for every task in this repository.
