# PaintZ persistence

PaintZ uses **Community Framework (CF) ModStorage** for logical paint assignment persistence.

## Why CF

PaintZ state belongs generically to `ItemBase`, not to hard-coded target categories. A broad native `ItemBase::OnStoreSave()` append is not safe because derived DayZ classes may serialize additional data after calling `super`, which can place PaintZ bytes in the middle of an existing subclass stream.

CF already provides an isolated, versioned ModStorage context at the `ItemBase` layer. PaintZ therefore writes only its own logical finish ID through `CF_OnStoreSave` / `CF_OnStoreLoad`.

## Stored data

For a painted item, PaintZ stores the canonical Paint Pack API finish ID such as:

`PZ-C-WDL`

or:

`NCP-C-FTN`

It does not persist texture paths, materials, hidden-selection indexes, policy decisions, cached inspection data, pattern scale, registry source, stale-recovery revision, or other derived state.

Unpainted items write no PaintZ ModStorage entry.

`CfgMods.PaintZ.storageVersion` is currently `1`.

## Generic coverage

The persistence hook is on `ItemBase` once. Weapons, detachable magazines, suppressors, clothing and any future normal inventory category use the same persistence path when they carry PaintZ state.

Adding another `ItemBase`-derived category must not require another persistence hook.

Stale-finish recovery uses this same shared logical state. It does not introduce per-category migration storage or a separate entity database.

## Load behavior

When a PaintZ ModStorage entry exists:

1. the canonical finish ID is restored into the item's logical PaintZ state;
2. visual work is deferred one call-queue turn so it does not run inside the persistence serializer;
3. on the authoritative server, explicit stale-finish recovery is evaluated against the current config revision before normal visual restoration;
4. PaintZ re-inspects the current model and hidden selections;
5. if the resulting finish is registered and a safe selection is available, PaintZ resolves the currently declared surface/scale and applies it;
6. if the finish remains unresolved, PaintZ preserves the logical ID and restores the original/default visual where a selection can be resolved;
7. synchronized PaintZ state is dirtied for clients.

Current runtime domains and include/exclude rules are not consulted while restoring historical paint. Stale-finish recovery is a separate explicit persistence-repair policy.

## Runtime/network state is not persistence state

The full finish string is persistent/server logical state. PaintZ does not synchronize arbitrary finish strings on every item. Registered finishes are represented on the network by their validated integer hash, and the registry rejects registered hash collisions.

Unresolved historical finish IDs deliberately synchronize hash `0`. Sending an unresolved string's raw hash would be unsafe because it could coincidentally equal the hash of a different currently registered finish and make a client render the wrong paint.

PaintZ therefore synchronizes a separate boolean `m_PaintZHasState` in addition to the resolved selection and pattern scale. This distinguishes:

- genuinely unpainted/stripped: `hasState = false`, hash `0`;
- registered painted finish: `hasState = true`, registered finish hash;
- unresolved historical finish: `hasState = true`, hash `0`.

For an unresolved historical item, the synchronized selection may be either a valid selection index or `-1`. The explicit state boolean is what guarantees that Strip Paint can still be offered even if the current model no longer exposes a safe/recoverable paint selection.

The full unresolved finish ID remains on the server and in CF persistence unless an explicit stale-finish migration or prune operation changes/clears it.

## Missing, conflicted or unknown finishes

The default behavior remains conservative. If the stored finish ID is absent from the active Paint Pack API registry because its pack was removed, its namespace/finish conflicts, or the definition is invalid, PaintZ:

- preserves the logical finish ID;
- never substitutes a different finish implicitly;
- restores the original/default visual where possible;
- keeps the item marked as historical PaintZ state;
- keeps Strip Paint available;
- allows the finish to resolve again if the same ID becomes valid in a later session.

This behavior is independent of new-paint eligibility policy.

A server administrator may explicitly override the default for stale IDs with `$profile:PaintZ/paintz_stale_finishes.json`. That file can map an exact unregistered source ID to a currently registered replacement or enable deliberate pruning of otherwise-unmapped stale IDs. See `STALE_FINISH_RECOVERY.md`.

## Explicit stale finish migration

For an unregistered source ID with an exact configured mapping, migration takes precedence over pruning.

A migration is permitted only when:

- the source is currently unregistered;
- the configured destination is currently registered;
- source and destination are different exact IDs;
- the config does not form a migration chain;
- PaintZ can resolve a safe selection and apply the destination finish.

On success, the item immediately carries the destination as its authoritative logical finish ID. The next normal CF save therefore persists the destination; no second migration database or alias state is stored with the item.

If migration cannot be completed, the old logical state is preserved. A matching failed migration does not fall through to `prune_unknown`.

## Explicit stale finish pruning

When `prune_unknown` is enabled and an unregistered finish has no migration entry, PaintZ clears that item's PaintZ logical state on encounter.

If a safe selection is recoverable, PaintZ restores its configured/original texture while clearing state. If no safe selection can be recovered, logical state is still cleared without inventing a texture selection.

This is intentionally destructive persistence repair. After the item is saved, reinstalling the old content pack cannot reconstruct the removed assignment.

Registered finishes are never pruned by this subsystem. An item does not become stale merely because runtime painting policy excludes it.

## Lazy recovery and runtime reload

Stale recovery is deliberately lazy. Loading or reloading `paintz_stale_finishes.json` does not scan persistence files or walk all loaded entities.

Recovery occurs through normal shared item-state encounters, including post-load restoration and server-side paint-state-dependent checks. Each item is evaluated at most once per successfully loaded config revision. A later successful reload permits another evaluation on the next encounter.

Therefore mappings should remain active long enough for relevant persisted items to be loaded/encountered and then saved. Removing a mapping does not undo migrations already completed.

Invalid reloads leave the previous valid stale-recovery configuration active. If no valid configuration is available at startup, stale state remains preserved.

## Stripping unresolved state

If a recoverable selection exists, manual stripping restores that selection's configured/original texture and clears the logical state.

If no recoverable selection exists, manual stripping still clears the logical PaintZ state. There is no texture operation to perform in that case because the unresolved item is already using its non-PaintZ/default appearance.

## Temporary PaintZ removal

CF preserves ModStorage payloads belonging to unloaded mods. Therefore this sequence is supported:

1. save painted items with PaintZ + CF loaded;
2. stop the server;
3. remove PaintZ but leave CF loaded;
4. start/save/stop the server one or more times;
5. restore PaintZ;
6. start the server.

The PaintZ finish assignment should return because CF preserved the opaque unloaded-mod payload, unless the server then explicitly migrates or prunes that now-stale assignment through stale-finish recovery.

If CF is also removed and the world is loaded/saved, PaintZ does not guarantee recovery.

## Acceptance matrix

Persistence must be tested on at least:

- weapon;
- detachable magazine, including ammunition state;
- a non-weapon/non-magazine `ItemBase` target such as a paintable suppressor or clothing/container fixture;
- player inventory;
- persistent/nested storage;
- vehicle cargo;
- repaint A -> B;
- strip -> restart;
- policy exclusion after painting;
- temporarily missing finish definition with default recovery config preserving the old ID;
- unresolved finish with a recoverable selection;
- unresolved finish with no recoverable selection;
- stale migration OLD -> registered NEW, including persistence of NEW after restart;
- configured migration whose target is unavailable, preserving OLD;
- configured source that is still registered, leaving it untouched;
- `prune_unknown = true` clearing an unmapped stale ID;
- matching migration taking precedence over `prune_unknown`;
- failed matching migration preserving state rather than falling through to prune;
- runtime stale-config reload affecting the next encounter without a global entity scan;
- failed stale-config reload retaining the previous valid configuration;
- late client join;
- PaintZ-unloaded / CF-retained save cycle.

The important architectural assertion is that the non-weapon fixture and all stale-recovery behavior use the exact same shared PaintZ persistence/state path and require no category-specific script.
