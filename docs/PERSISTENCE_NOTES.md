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

It does not persist texture paths, materials, hidden-selection indexes, policy decisions, cached inspection data, pattern scale, registry source, or other derived state.

Unpainted items write no PaintZ ModStorage entry.

`CfgMods.PaintZ.storageVersion` is currently `1`.

## Generic coverage

The persistence hook is on `ItemBase` once. Weapons, detachable magazines, suppressors, clothing and any future normal inventory category use the same persistence path when they carry PaintZ state.

Adding another `ItemBase`-derived category must not require another persistence hook.

## Load behavior

When a PaintZ ModStorage entry exists:

1. the canonical finish ID is restored into the item's logical PaintZ state;
2. visual work is deferred one call-queue turn so it does not run inside the persistence serializer;
3. PaintZ re-inspects the current model and hidden selections;
4. if the finish is registered and a safe selection is available, PaintZ resolves the currently declared surface/scale and applies it;
5. if the finish is unresolved, PaintZ preserves the logical ID and restores the original/default visual where a selection can be resolved;
6. synchronized PaintZ state is dirtied for clients.

Current runtime domains and include/exclude rules are not consulted while restoring historical paint.

## Runtime/network state is not persistence state

The full finish string is persistent/server logical state. PaintZ does not synchronize arbitrary finish strings on every item. Registered finishes are represented on the network by their validated integer hash, and the registry rejects registered hash collisions.

Unresolved historical finish IDs deliberately synchronize hash `0`. Sending an unresolved string's raw hash would be unsafe because it could coincidentally equal the hash of a different currently registered finish and make a client render the wrong paint.

PaintZ therefore synchronizes a separate boolean `m_PaintZHasState` in addition to the resolved selection and pattern scale. This distinguishes:

- genuinely unpainted/stripped: `hasState = false`, hash `0`;
- registered painted finish: `hasState = true`, registered finish hash;
- unresolved historical finish: `hasState = true`, hash `0`.

For an unresolved historical item, the synchronized selection may be either a valid selection index or `-1`. The explicit state boolean is what guarantees that Strip Paint can still be offered even if the current model no longer exposes a safe/recoverable paint selection.

The full unresolved finish ID remains on the server and in CF persistence. It is not discarded merely because the client cannot resolve/display it.

## Missing, conflicted or unknown finishes

If the stored finish ID is absent from the active Paint Pack API registry because its pack was removed, its namespace/finish conflicts, or the definition is invalid, PaintZ:

- preserves the logical finish ID;
- never substitutes a different finish;
- restores the original/default visual where possible;
- keeps the item marked as historical PaintZ state;
- keeps Strip Paint available;
- allows the finish to resolve again if the same ID becomes valid in a later session.

This behavior is independent of new-paint eligibility policy.

## Stripping unresolved state

If a recoverable selection exists, stripping restores that selection's configured/original texture and clears the logical state.

If no recoverable selection exists, stripping still clears the logical PaintZ state. There is no texture operation to perform in that case because the unresolved item is already using its non-PaintZ/default appearance.

## Temporary PaintZ removal

CF preserves ModStorage payloads belonging to unloaded mods. Therefore this sequence is supported:

1. save painted items with PaintZ + CF loaded;
2. stop the server;
3. remove PaintZ but leave CF loaded;
4. start/save/stop the server one or more times;
5. restore PaintZ;
6. start the server.

The PaintZ finish assignment should return because CF preserved the opaque unloaded-mod payload.

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
- temporarily missing finish definition;
- unresolved finish with a recoverable selection;
- unresolved finish with no recoverable selection;
- late client join;
- PaintZ-unloaded / CF-retained save cycle.

The important architectural assertion is that the non-weapon fixture must use the exact same PaintZ persistence path and require no category-specific script.
