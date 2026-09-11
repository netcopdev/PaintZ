# Stale finish recovery

PaintZ normally treats an unregistered persisted finish ID as historical logical state. That remains the safe default. Server administrators may opt into explicit lazy recovery through:

```text
$profile:PaintZ/paintz_stale_finishes.json
```

This facility exists for real catalogue changes such as a released finish ID being retired or renamed, and for deliberate cleanup of abandoned historical assignments. It is not a general alias system and it does not alter the runtime finish registry.

## What counts as stale

A finish assignment is stale only when all of the following are true:

1. the item has PaintZ logical state;
2. its stored finish ID is not `NONE`;
3. that exact finish ID is not registered in the active Paint Pack API registry.

A currently registered source ID is never migrated or pruned by this subsystem, even if an administrator accidentally lists it in `migrations`.

Policy exclusion is unrelated. A registered finish does not become stale merely because the target is outside `paintz_items.json` domains or excluded from new painting.

## Resolution order

For a stale item PaintZ evaluates recovery in this order:

1. Exact migration mapping.
2. `prune_unknown`, but only when no migration mapping exists.
3. Preserve stale state.

A matching migration therefore takes precedence over pruning. If that migration cannot be completed, PaintZ preserves the stale state rather than falling through to destructive pruning.

## Migrations

`migrations` is an exact-ID map:

```json
{
  "PZ-C-FLK": "PZ-C-FTN"
}
```

The source is the retired/unregistered persisted ID. The destination must be registered when an item is encountered.

Successful migration:

- uses the same shared `ItemBase` state path as normal painting;
- resolves a safe current paint selection;
- applies the destination finish through the normal PaintZ visual path;
- changes the authoritative logical finish ID to the destination;
- synchronizes the new state normally;
- persists the destination ID on the item's next normal CF ModStorage save.

Migration does not replace the item or touch unrelated item state.

A migration is skipped non-destructively when the destination is unavailable, no safe selection can be resolved, or the destination visual cannot be applied.

Migration chains are invalid. Configure every historical source directly to the final registered destination. This avoids recursion, ordering dependencies and cycles.

## Pruning

When `prune_unknown` is true and no migration exists for a stale ID, PaintZ clears the logical PaintZ assignment on encounter.

If a safe selection is available, its configured/original texture is restored before the state is cleared. If no safe selection can be recovered, PaintZ still clears the logical stale assignment without guessing a texture selection. Unresolved historical items are already expected to use their original/default appearance when visual restoration is possible.

Pruning is intentionally destructive for the affected logical assignment. Once the item is subsequently saved, reinstalling the old finish pack cannot restore that assignment.

`prune_unknown` does not strip:

- registered finishes;
- finishes merely excluded by runtime item policy;
- non-PaintZ visual modifications;
- arbitrary textures that have no PaintZ logical state.

## Lazy encounter model

PaintZ does not scan persistence files or enumerate the whole world when this config loads or reloads.

Recovery runs through the normal shared item-state lifecycle:

- after a persisted PaintZ state is loaded, before visual restoration;
- before server-side checks/operations that depend on whether an item has PaintZ state.

Each item is evaluated at most once per successfully loaded config revision. A later successful config reload permits another evaluation on the item's next relevant encounter. This prevents action-condition polling from repeatedly emitting the same recovery warning.

Consequently, a migration entry must remain configured long enough for the relevant persisted items to be loaded/encountered and saved. Removing a mapping does not undo items that have already migrated.

## Reload behavior

The config follows PaintZ's existing server-runtime JSON pattern:

- `reload_seconds = -1` means startup-only;
- a positive value schedules periodic reloads;
- `0` or values below `-1` are treated as startup-only with a warning;
- parsing and validation occur on a detached candidate;
- a failed reload keeps the previous valid configuration;
- startup without a valid stale-finish config leaves stale historical state preserved.

The runtime registry itself is not reloaded by this file. Migrations resolve against the set of finishes registered for the current server session.

## Configuration safety

The bundled default is:

```json
{
  "version": 1,
  "reload_seconds": 60,
  "prune_unknown": false,
  "migrations": {}
}
```

Therefore installing/updating PaintZ does not automatically migrate or prune any historical finish assignment.

Operational field documentation is bundled as `config/paintz_stale_finishes_README.txt` and refreshed to `$profile:PaintZ/paintz_stale_finishes_README.txt` on server startup. The administrator JSON itself is never overwritten after creation.

## Relationship to Paint Pack API identity

Canonical finish IDs remain stable developer-authored identities. Reorganizing an unchanged finish between content packs does not require migration when the ID remains the same.

Stale recovery is for a deliberate identity break or deliberate retirement cleanup. It is a server persistence-repair policy, not permission for content packs to declare aliases, override currently registered IDs, or make load order determine identity.
