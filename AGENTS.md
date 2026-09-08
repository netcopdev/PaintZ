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

For the current persistence-era PaintZ work, `feature/universal-paint-persistence` is the shared dependency base until that work is merged or deliberately replaced by a newer integration base.

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

## Scope

The detailed PaintZ architecture, persistence, policy, model-safety, networking, testing and coding rules are in `docs/CODEX_PROJECT_RULES.md`. They are mandatory for every task in this repository.
