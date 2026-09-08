# PaintZ persistence

PaintZ uses **Community Framework (CF) ModStorage** for logical paint assignment persistence.

## Why CF

PaintZ state belongs generically to `ItemBase`, not to hard-coded target categories. A broad native `ItemBase::OnStoreSave()` append is not safe because derived DayZ classes may serialize additional data after calling `super`, which can place PaintZ bytes in the middle of an existing subclass stream.

CF already provides an isolated, versioned ModStorage context at the `ItemBase` layer. PaintZ therefore writes only its own logical finish ID through `CF_OnStoreSave` / `CF_OnStoreLoad`.

## Stored data

For a painted item, PaintZ stores the canonical finish ID such as:

`PZ-C-WDL`

It does not persist texture paths, materials, hidden-selection indexes, policy decisions, cached inspection data or other derived state.

Unpainted items write no PaintZ ModStorage entry.

`CfgMods.PaintZ.storageVersion` is currently `1`.

## Generic coverage

The persistence hook is on `ItemBase` once. Weapons, detachable magazines, suppressors, clothing and any future normal inventory category use the same persistence path when they carry PaintZ state.

Adding another `ItemBase`-derived category must not require another persistence hook.

## Load behavior

When a PaintZ ModStorage entry exists:

1. the canonical finish ID is restored into the item's logical PaintZ state;
2. visual application is deferred one call-queue turn so it does not run inside the persistence serializer;
3. PaintZ re-inspects the current model and hidden selections;
4. the finish is applied if the current catalogue contains the ID and a safe selection is available;
5. network state is dirtied so clients resolve the same finish.

Current runtime domains and include/exclude rules are not consulted while restoring historical paint.

## Missing or unknown finishes

If the stored finish ID is temporarily absent from the current PaintZ catalogue, PaintZ keeps the logical ID and logs a warning. It does not strip the item or substitute another finish. Restoring that finish definition later allows the item to render again.

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
- late client join;
- PaintZ-unloaded / CF-retained save cycle.

The important architectural assertion is that the non-weapon fixture must use the exact same PaintZ persistence path and require no category-specific script.
