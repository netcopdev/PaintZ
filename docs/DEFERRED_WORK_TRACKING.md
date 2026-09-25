# Deferred work and integration tracking

This policy exists to prevent valid fixes or other required work from being completed on a branch, deferred for testing, and then silently forgotten while development continues on `main`.

It applies to every unmerged branch containing implementation, fixes, documentation, tooling, migration work, or other changes that may still be relevant to the project.

## Core rule

An unmerged work branch must never become invisible project state.

Before switching away from a branch to unrelated work, the branch must have an explicit tracked disposition in GitHub. Use either:

- an open pull request, normally draft while verification is incomplete; or
- an open issue when a PR would be misleading because the branch must first be rebased, ported, redesigned, or otherwise reworked.

A branch name by itself is not tracking.

## Required tracker contents

The PR or issue must record enough information for another contributor or agent to resume the work without reconstructing the conversation:

- branch name;
- relevant commit SHA or current branch head;
- what problem the work solves;
- current implementation status;
- tests already performed;
- tests or review still required;
- the exact blocker preventing integration, if any;
- the next action;
- acceptance criteria for integration;
- whether the branch should be merged, ported/cherry-picked, rebased, superseded, or rejected.

When later commits materially change the candidate, update the tracker.

## Verification and integration

Keeping work separate for testing is correct. Forgetting it afterward is not.

If a candidate fix is waiting for live DayZ/server testing, keep it unmerged but tracked. Once the required testing passes and the user approves integration, integrate the fix promptly or record a new concrete blocker before moving on.

Do not continue release preparation, repository cleanup, or a claim that `main` is clean/release-ready while a verified and approved relevant fix exists only on another branch.

A proper fix may remain unmerged only for a stated reason such as pending verification, unresolved conflict, redesign, or explicit user decision. "Forgotten branch" is never an acceptable state.

## Superseded or rejected work

If investigation shows that a candidate is wrong, obsolete, or superseded:

1. record that decision in its PR/issue;
2. state why it is no longer required;
3. identify the replacement commit/PR when applicable;
4. close the tracker;
5. remove or retain the branch according to normal repository hygiene.

Do not silently abandon a branch whose purpose is still relevant.

## Mandatory outstanding-work audit

Before any release, release candidate, "clean main" declaration, broad repository cleanup, or deletion of old branches, perform an outstanding-work audit.

The audit must inspect at minimum:

1. remote/local work branches visible through the available repository interface;
2. open pull requests;
3. open issues tracking deferred or integration work;
4. release-acceptance requirements related to those branches;
5. branches with commits not represented on current `main`.

For every relevant outstanding branch, determine one of these dispositions:

- already integrated/equivalent on `main`;
- intentionally pending with an active tracker and concrete blocker;
- ready and approved for integration;
- superseded by identified work;
- rejected/obsolete with recorded reason;
- experiment that was never intended for integration.

If a branch contains a plausible unresolved fix for a currently accepted defect and its changes are absent from `main`, the audit is not complete until that discrepancy is resolved or explicitly tracked.

## Release gate

`main` must not be described as clean, complete, release-ready, or fully reconciled when:

- a relevant fix exists only on an untracked branch;
- a tracker says verification passed and integration is approved but the fix is still absent from `main` without a new blocker;
- release acceptance contains a requirement known to fail on `main` while a candidate correction exists elsewhere;
- an old work branch has not been classified during the outstanding-work audit.

The purpose of this gate is not to force untested code into `main`. It is to ensure deferred work remains visible until it is either integrated or deliberately closed.