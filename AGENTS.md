# PaintZ Codex Instructions

Before modifying, creating, deleting, renaming, generating, rebasing, or moving any repository work, you MUST read `docs/CODEX_PROJECT_RULES.md` completely and follow it together with this file.

For any work involving paint packs, finish registration/identity, spray-can pack integration, PackKit output, namespace handling, or cross-repository interoperability, you MUST also read and follow `docs/PAINT_PACK_API.md`. That file is the authoritative interoperability contract for PaintZ, PaintZ-PackKit, and all PaintZ content packs.

When the work touches the concrete DayZ config representation, runtime registry/discovery, generated pack config, thin can classes, finish surfaces, or conformance tests, you MUST also read `docs/PAINT_PACK_CONFIG_V1.md`. It defines the current API-v1 config representation and must remain consistent with the higher-level `docs/PAINT_PACK_API.md` contract.

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

## Branch and merge discipline

New feature, fix, refactor, experiment, tooling, documentation, or other repository work MUST be written on a separate appropriately named branch unless the user explicitly decides that the work should be performed directly on an existing branch.

Nothing goes to `main` merely because implementation is complete. Work may be integrated into `main` only after it has been demonstrated to be safe and functional to the level appropriate for the change, and the user has explicitly approved that specific work for integration.

Creating a feature branch does **not** authorize merging it. Do not merge, squash, rebase, fast-forward, or otherwise integrate a work branch into `main` or another shared integration branch unless the user explicitly instructs you to merge/integrate it after review/testing, or that exact integration was explicitly agreed beforehand.

When the implementation is complete, leave it on its work branch and report the branch name, relevant commits/PR, and test status. Treat verification and merge as separate steps. If live/runtime testing is still required, say so and keep the work branch unmerged.

Do not infer merge permission from phrases such as "implement this", "fix this", "do it", "finish it", "make it work", or from successful tests. Only an explicit merge/integration approval authorizes integration.

If branch intent or approval status is ambiguous, preserve separation: create/use a work branch and do not merge.

## Documentation progression

Documentation must progress in parallel with implementation. A feature, fix, configuration change, tooling change, workflow change, architecture change, persistence change, API/contract change, or user-/administrator-visible behavior change must update the relevant README/docs/config help on the same work branch before that work is considered complete.

Merge implementation and its documentation together. Do not knowingly merge code first and leave documentation describing an older behavior, configuration schema, architecture, or project state for a later cleanup branch.

Where a change spans PaintZ and one or more content/tooling repositories, update the authoritative documentation in the repository that owns the contract and the repository-specific documentation affected by the change as part of the same coordinated work.

A purely internal refactor with no user-, administrator-, contributor-, build-, persistence-, configuration-, or API-visible effect may require no documentation edit, but that must be a deliberate no-documentation-impact determination rather than an omission.

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
