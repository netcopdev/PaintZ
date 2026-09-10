# PaintZ

PaintZ is a generic DayZ runtime painting framework. Compatible inventory items become paintable from runtime type/model data and server JSON policy, without per-class compatibility code.

Weapons, magazines and weapon attachments are policy data, not hard-coded architectural categories. Additional ordinary `ItemBase`-derived inventory families should be enableable through `paintz_items.json` alone when their models expose a safe paintable hidden selection.

PaintZ core owns runtime mechanics and the official `PZ` namespace identity, but it deliberately owns **no individual paint finishes**. Finish content is supplied by independent Paint Pack API content packs.

## Paint Pack API

Each complete short finish ID is both runtime identity and persisted logical identity:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples:

```text
PZ-B-BLK
PZ-C-FTN
PZ-S-FDE
NCP-C-FTN
```

PaintZ exposes `CfgPaintZPacks` and `CfgPaintZFinishes`, discovers the complete loaded set, and validates namespaces before accepting finish registrations. Duplicate namespace owners disable that namespace; duplicate complete finish IDs disable that finish; there is no first-loaded-wins or last-loaded-wins overwrite behavior.

### Official `PZ` namespace

PaintZ core permanently owns the official `PZ` namespace through:

```text
PZ_PaintZOfficial
```

Official content packs do not declare `PZ`. They register unique `PZ-*` finishes against the core owner and each depends directly on PaintZ.

This permits independent peer packages such as:

```text
PaintZ Standard Pack -> PaintZ
PaintZ Pastel Pack   -> PaintZ
PaintZ Military Pack -> PaintZ
PaintZ Hunting Pack  -> PaintZ
```

No official content pack is a parent/base dependency for another. Package/category names are distribution concerns, not canonical finish namespaces. Moving an unchanged official finish between official packs is persistence-safe when its complete `PZ-*` ID remains unchanged.

All valid 2-3 character prefixes beginning with `PZ` remain reserved for official PaintZ use. Additional `PZ?` namespaces are intentionally unassigned and should not be consumed merely to represent categories.

Third-party standalone packs continue to own their own non-reserved 2-3 character namespaces. Third-party multi-PBO families may use one owner PBO plus dependent satellite content PBOs.

See `docs/PAINT_PACK_API.md` and `docs/PAINT_PACK_CONFIG_V1.md`.

## Persistence

PaintZ requires **Community Framework (CF)** and uses CF ModStorage for logical paint persistence.

Persistence lives once at the shared `ItemBase` level. There are no separate weapon, magazine or attachment PaintZ persistence implementations. This keeps future ordinary inventory categories on the same state/persistence path and avoids unsafe insertion of PaintZ bytes into specialized native serializer streams.

PaintZ persists only the canonical finish ID. Derived pattern scale, texture path and selection index are not persistent data.

If a content pack is temporarily missing or a finish cannot resolve, the stored finish ID remains intact by default. The item falls back to its original visual where possible and remains strippable; if the same finish later returns, it can resolve again.

A server may explicitly configure lazy stale-finish recovery in `$profile:PaintZ/paintz_stale_finishes.json`. Exact migrations can permanently replace an unregistered historical ID with a currently registered ID, and `prune_unknown` can deliberately clear otherwise-unmapped unregistered IDs on encounter. Registered source IDs are never affected. The bundled default is non-destructive.

This also means an unchanged `PZ-*` finish may be reorganized from one official content pack to another without changing persisted identity or requiring stale-finish recovery.

See `docs/PERSISTENCE_NOTES.md` and `docs/STALE_FINISH_RECOVERY.md`.

## Stale finish recovery

Server configuration:

```text
$profile:PaintZ/paintz_stale_finishes.json
```

A finish is stale only when an item has a persisted PaintZ finish ID that is absent from the active runtime registry. Recovery is evaluated lazily through the shared item-state path rather than by scanning the world or persistence database.

Resolution order is:

1. a currently registered ID is left untouched;
2. an exact stale-ID migration is attempted first;
3. if no migration exists, `prune_unknown = true` clears the stale PaintZ assignment;
4. otherwise the historical state is preserved.

Migration destinations must be currently registered. Migration chains and wildcards are rejected. A matching migration that cannot be completed preserves the stale state rather than falling through to pruning.

The config uses the same last-known-good runtime reload model as other PaintZ server JSON files. A successful reload makes the new policy effective on each item's next relevant encounter; it does not immediately mutate all loaded items.

See `config/paintz_stale_finishes_README.txt` and `docs/STALE_FINISH_RECOVERY.md`.

## Painted item identification

Painted items expose their PaintZ finish through DayZ's normal dynamic name and description hooks. With Flecktarn applied, for example:

```text
KA-74 [Flecktarn]
```

and the description receives:

```text
Finish: Flecktarn (PZ-C-FTN)
```

PaintZ calls the previous `NameOverride` / `DescriptionOverride` implementation first and decorates that result, preserving compatible third-party dynamic naming behavior. Stripping clears only PaintZ logical finish state and removes the extra presentation.

## Runtime policy

Server administrators configure relevance and new-paint eligibility in:

```text
$profile:PaintZ/paintz_items.json
```

PaintZ creates this file from `config/paintz_items.default.json` on first startup and never overwrites an existing administrator copy. Operational help is copied to `$profile:PaintZ/paintz_items_README.txt`.

Domains support:

- DayZ base/config types;
- classname wildcards;
- exact declared `inventorySlot` values;
- wildcard matching over declared `inventorySlot` values.

Slot matching uses the target class's declared compatible slots, not its current attachment state. Loose stocks, handguards, suppressors, optics and flashlights can therefore be selected by policy while on the ground.

Ordered include/exclude rules support the same generic selector model. Policy affects new painting/repainting only; already-painted items remain painted and strippable after exclusion or domain removal. Stale-finish recovery is a separate persistence-repair policy and does not reinterpret eligibility exclusions as stale state.

## Hidden-selection safety

PaintZ inspects the actual target model at runtime rather than maintaining a classname compatibility table.

It prefers plausible body/camo/housing selections and rejects clearly functional surfaces such as glass, lenses, reticles, displays and emissive elements. If several candidate selections remain ambiguous, PaintZ rejects the target rather than guessing.

## Pattern scale normalization

Patterned finishes may declare multiple surface-scale variants so camouflage geometry is less dependent on target physical size.

Server configuration:

```text
$profile:PaintZ/paintz_pattern_scaling.json
```

PaintZ measures the target's longest collision-box dimension, maps it to a requested scale, and uses that scale only if the active finish explicitly registered it. Otherwise it falls back to the finish's mandatory 100% representation.

PaintZ never constructs third-party texture paths from naming conventions. Different packs may provide different valid scale subsets.

Pattern scale is derived state, not persistent identity. A later server load re-derives scale from the current mapping and currently available finish variants while retaining the same persisted finish ID.

## Size-dependent action tuning

Painting time, paint consumption, stripping time and stripper consumption derive from the same physical-size measurement used by pattern scaling.

Server configuration:

```text
$profile:PaintZ/paintz_action_tuning.json
```

The target's longest dimension is clamped between configurable endpoints; action duration is linearly interpolated and consumable usage scales proportionally. The server owns/reloads this config and synchronizes valid settings to clients so continuous-action timing agrees.

See `config/paintz_action_tuning_README.txt`.

## Runtime flow

1. Player holds a spray can supplied by a registered content pack or the PaintZ core stripper can.
2. The can exposes its complete finish ID through `paintzFinish`.
3. PaintZ resolves the finish through the runtime registry.
4. Persisted historical state passes through explicit stale-finish recovery when applicable; registered state remains untouched.
5. JSON domains/rules decide whether the target is relevant and eligible for a new application.
6. `PaintZ_PaintInspector` selects a safe paintable hidden selection or rejects the item.
7. The server derives action duration/consumption from target size and revalidates conditions at completion.
8. Patterned finishes resolve only an explicitly registered scale variant.
9. PaintZ updates the existing object's shared `ItemBase` paint state and visual without replacing the object/classname.
10. Shared synchronization publishes current visual state to clients.
11. CF ModStorage persists only the logical finish assignment.

No item replacement or classname change occurs.

## Core/content separation

PaintZ core contains:

- runtime registry and validation;
- the canonical official `PZ` namespace owner;
- generic paint/strip actions;
- common non-spawnable spray-can base and stripper;
- policy/model inspection;
- synchronization/persistence and stale-finish recovery;
- pattern scaling and action tuning.

PaintZ core deliberately contains no official finish catalogue, finish-specific spray cans, finish textures/assets, or per-finish actions.

Official content lives in independent content-pack repositories such as `netcopdev/PaintZ-Standard-Pack`; future official Standard/Pastel/Military/Hunting collections may all contribute to the same core-owned `PZ` namespace without depending on one another.

Third-party authors can build conforming packs using `netcopdev/PaintZ-PackKit` or implement the documented config contract manually. PackKit is an offline authoring/generation tool and is never a runtime dependency.

## Project layout

- `config.cpp` — PaintZ runtime registration, core-owned `PZ` namespace owner, common can base and stripper.
- `config/` — bundled runtime policy/scaling/action-tuning/stale-recovery defaults and operational help.
- `Scripts/4_World/PaintZ/Policy/` — generic type/class/declared-slot policy.
- `Scripts/4_World/PaintZ/Paint/PaintZ_ItemPaintState.c` — shared `ItemBase` paint state, display decoration, synchronization and CF persistence hooks.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintPackRegistry.c` — namespace/finish discovery, validation, collision handling and runtime-surface lookup.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintPersistence.c` — CF ModStorage codec and restoration helpers.
- `Scripts/4_World/PaintZ/Paint/PaintZ_StaleFinishRecovery.c` — runtime-reloadable lazy migration/pruning for unregistered persisted finish IDs.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PatternScaling.c` — target-size measurement and pattern-scale resolution.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintInspector.c` — runtime hidden-selection inspection.
- `docs/PAINT_PACK_API.md` — authoritative identity/interoperability contract.
- `docs/PAINT_PACK_CONFIG_V1.md` — concrete DayZ config representation.
- `docs/ARCHITECTURE.md` — architectural invariants.
- `docs/PERSISTENCE_NOTES.md` — persistence design and acceptance matrix.
- `docs/STALE_FINISH_RECOVERY.md` — explicit stale-ID migration/pruning semantics.
- `docs/item-policy.md` — runtime JSON policy contract.

## Universality boundary

The JSON-only category expansion guarantee applies to ordinary `ItemBase`-derived inventory items. Static world objects, buildings, vehicles and unrelated engine hierarchies may require separate lifecycle/persistence design.

## License

MIT for original PaintZ code.
