# PaintZ

PaintZ is a generic DayZ runtime painting framework. Compatible inventory items become paintable from their runtime type/model data and server JSON policy, without per-class compatibility code.

Weapons, magazines and weapon attachments are policy data, not hard-coded architectural categories. Additional ordinary `ItemBase`-derived inventory families should be enableable through `paintz_items.json` alone when their models expose a safe paintable hidden selection.

## Persistence

PaintZ requires **Community Framework (CF)** and uses CF ModStorage for logical paint persistence.

Persistence lives once at the shared `ItemBase` level. There are no separate weapon, magazine or attachment PaintZ persistence implementations. This keeps future categories on the same state/persistence path and avoids unsafe insertion of PaintZ bytes into specialized native serializer streams.

If PaintZ is temporarily unloaded while CF remains loaded, CF preserves PaintZ's opaque ModStorage payload across saves. Removing CF as well is outside the persistence guarantee. See `docs/PERSISTENCE_NOTES.md`.

PaintZ persists only the canonical finish ID. Derived pattern scale, texture path and selection index are not persistent data.

## Runtime policy

Server administrators configure relevance and new-paint eligibility in:

`$profile:PaintZ/paintz_items.json`

PaintZ creates this file from `config/paintz_items.default.json` on first startup and never overwrites an existing administrator copy. Operational help is copied to `$profile:PaintZ/paintz_items_README.txt`.

Domains support:

- DayZ base/config types;
- classname wildcards;
- exact declared `inventorySlot` values;
- wildcard matching over declared `inventorySlot` values.

Slot matching uses the target class's declared compatible slots, not the item's current attachment state, so loose stocks, handguards, suppressors, optics and flashlights can be selected by policy while lying on the ground.

Ordered include/exclude rules support the same generic model: type is optional scope and rules may select by classname, inheritance, exact slot or slot wildcard. Legacy version-1 `weapon` / `magazine` rule aliases remain accepted.

The bundled default includes weapons, detachable magazines, common weapon/pistol/suppressor attachment slot families, and `SmallProtectorCase`.

PaintZ does not hard-exclude sensible item families such as optics or flashlights. If an eligible model exposes a safe body/camo/housing selection, it may paint. Clearly functional surfaces such as glass, lenses, reticles, displays and emissive elements remain protected by the model-safety heuristic.

Policy affects new painting/repainting only. Existing painted items remain painted and strippable even after exclusion or domain removal.

## Pattern scale normalization

Patterned finishes can use generated scale variants so the apparent camouflage geometry is less dependent on the physical size of the target model.

The server configuration is:

`$profile:PaintZ/paintz_pattern_scaling.json`

PaintZ measures the target's longest collision-box dimension and maps it to a generated scale variant. The bundled configuration defaults to 1x for every item; administrators can add size ranges and reload the file while the server is running.

The current generated scale options are:

`0.5x, 0.75x, 1x, 1.5x, 2x, 3x`

A config reload affects the next repaint/application only. It does not sweep through already-loaded painted items. On a later server restart/load, persisted items derive their scale again from the current mapping because only the finish ID is stored.

Physical size is only a proxy for UV density. Two similarly sized models may still need different pattern sizes because their UV layouts differ. See `config/paintz_pattern_scaling_README.txt`.

## Runtime flow

1. Player holds a PaintZ spray can.
2. PaintZ resolves the targeted entity.
3. JSON domains decide whether the target is relevant for new-paint interaction.
4. Ordered policy rules decide whether new application is allowed.
5. `PaintZ_PaintInspector` inspects the actual runtime hidden selections.
6. A conservative global heuristic chooses a safe body-like selection or rejects the item.
7. The server rechecks all conditions when the action completes.
8. For patterned finishes, the server derives a scale from the current scaling config and target collision-box size.
9. PaintZ updates the existing object's shared `ItemBase` PaintZ state and calls `SetObjectTexture()`.
10. Shared synchronization publishes the finish, selected surface and transient scale to clients.
11. CF ModStorage preserves only the logical finish assignment through persistence.

No item replacement or classname change occurs.

## Project layout

- `config.cpp` — mod registration, CF dependency/storage version, generated paint declarations.
- `config/paintz_items.default.json` — shipped default domains/rules.
- `config/paintz_pattern_scaling.default.json` — shipped default pattern-size mapping.
- `config/paintz_pattern_scaling_README.txt` — runtime scaling configuration contract.
- `Scripts/4_World/PaintZ/Policy/` — generic type/class/declared-slot domain and rule policy.
- `Scripts/4_World/PaintZ/Paint/PaintZ_ItemPaintState.c` — shared inventory-item paint state, synchronization and CF persistence hooks.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintPersistence.c` — CF ModStorage codec and post-load restoration helpers.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PatternScaling.c` — server-side size measurement, scale mapping and live config reload.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintInspector.c` — runtime hidden-selection inspection.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintVisuals.c` — texture application/restoration.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintTarget.c` — generic paint/strip dispatch through `ItemBase`.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintedState.c` — existing-paint lookup independent from policy.
- `docs/ARCHITECTURE.md` — architectural invariants and universality boundary.
- `docs/PERSISTENCE_NOTES.md` — persistence design and acceptance matrix.
- `docs/item-policy.md` — JSON contract.

## Texture strategy

Painted surfaces use clean generated color/pattern assets under `data/surfaces`; spray-can labels are separate assets under `data/cans`. PaintZ normally changes only the texture and leaves the target's material/RVMat behavior intact.

`tools/paintzgen/paints.json` is the source of truth for finishes and generated pattern-scale variants. The 1x pattern surface retains the normal historical filename while non-1x variants use `_sNNN` percentage suffixes. Regenerate outputs with the supplied tooling rather than hand-editing generated files.

## Universality boundary

The JSON-only category expansion guarantee applies to ordinary `ItemBase`-derived inventory items. Static world objects, buildings, vehicles and other unrelated engine hierarchies may require separate lifecycle/persistence design.

## License

MIT for original PaintZ code.
