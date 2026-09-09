# PaintZ

PaintZ is a generic DayZ runtime painting framework. Compatible inventory items become paintable from their runtime type/model data and server JSON policy, without per-class compatibility code.

Weapons, magazines and weapon attachments are policy data, not hard-coded architectural categories. Additional ordinary `ItemBase`-derived inventory families should be enableable through `paintz_items.json` alone when their models expose a safe paintable hidden selection.

PaintZ core does not own individual paint finishes. Finish content is supplied by Paint Pack API packs such as the official [PaintZ Standard Pack](https://github.com/netcopdev/PaintZ-Standard-Pack).

## Paint Pack API

Paint packs register one namespace owner plus their finishes through `CfgPaintZPacks` and `CfgPaintZFinishes`. Each complete short ID is both the runtime identity and persisted logical identity:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples:

```text
PZ-C-FTN
PZ-S-FDE
NCP-C-FTN
```

PaintZ validates the actually loaded mod set at startup. Duplicate namespace owners disable that namespace; duplicate complete finish IDs disable that finish; there is no first-loaded-wins or last-loaded-wins overwrite behavior.

The official Standard Pack owns `PZ`. All valid 2-3 character prefixes beginning with `PZ` are reserved for official PaintZ content.

See `docs/PAINT_PACK_API.md` and `docs/PAINT_PACK_CONFIG_V1.md`.

## Persistence

PaintZ requires **Community Framework (CF)** and uses CF ModStorage for logical paint persistence.

Persistence lives once at the shared `ItemBase` level. There are no separate weapon, magazine or attachment PaintZ persistence implementations. This keeps future categories on the same state/persistence path and avoids unsafe insertion of PaintZ bytes into specialized native serializer streams.

If PaintZ is temporarily unloaded while CF remains loaded, CF preserves PaintZ's opaque ModStorage payload across saves. Removing CF as well is outside the persistence guarantee. See `docs/PERSISTENCE_NOTES.md`.

PaintZ persists only the canonical finish ID. Derived pattern scale, texture path and selection index are not persistent data.

If a paint pack is temporarily missing or a finish cannot currently resolve, the stored finish ID remains intact. The item falls back to its original visual where possible and remains strippable; if the pack later returns, the finish can resolve again.

## Painted item identification

Painted items expose their PaintZ finish through DayZ's normal dynamic name and description hooks. With Flecktarn applied, for example, a `KA-74` is displayed as:

`KA-74 [Flecktarn]`

Its existing description is preserved and receives one additional line:

`Finish: Flecktarn (PZ-C-FTN)`

PaintZ calls the previous `NameOverride` / `DescriptionOverride` implementation first and decorates that result, so compatible third-party dynamic names and descriptions remain intact. Stripping the item clears the PaintZ logical finish state, so the extra name suffix and description line disappear automatically. This changes only UI presentation; it does not change the classname or persistence identity.

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

Patterned finishes may declare multiple surface-scale variants so the apparent camouflage geometry is less dependent on the physical size of the target model.

The server configuration is:

`$profile:PaintZ/paintz_pattern_scaling.json`

PaintZ measures the target's longest collision-box dimension and maps it to a scale percentage. It then uses that scale only if the active finish actually registered a corresponding surface; otherwise it falls back to the finish's 100% surface.

The official Standard Pack currently provides:

`0.5x, 0.75x, 1x, 1.5x, 2x, 3x`

Other paint packs may declare a different valid subset. PaintZ never constructs or assumes third-party texture paths.

A config reload affects the next repaint/application only. It does not sweep through already-loaded painted items. On a later server restart/load, persisted items derive their scale again from the current mapping because only the finish ID is stored.

Physical size is only a proxy for UV density. Two similarly sized models may still need different pattern sizes because their UV layouts differ. See `config/paintz_pattern_scaling_README.txt`.

## Size-dependent action tuning

Painting time, paint consumption, stripping time and paint-stripper consumption are derived from the same physical-size measurement used by pattern scaling.

The server configuration is:

`$profile:PaintZ/paintz_action_tuning.json`

This feature does not use discrete size ranges. The target's longest collision-box dimension is clamped between configurable minimum and maximum endpoints, then action duration is linearly interpolated between the configured times. Paint and stripper usage are proportional to the same clamped physical size.

Shipped defaults are:

- `0.2 m` or smaller: `5 s` action time;
- `0.8 m` or larger: `20 s` action time;
- linear timing between those endpoints;
- a full paint can covers approximately three `0.8 m` objects;
- a `0.2 m` object therefore uses about `1/12` of a full paint can;
- stripper consumption follows the same size model and has its own applications-per-full-can setting.

The server reloads this config according to `reload_seconds` and synchronizes every valid update to clients so continuous-action progress uses the same timing on both sides. Existing administrator JSON is never overwritten. The adjacent runtime README is refreshed from the mod on every server start and documents the exact formulas and fields.

See `config/paintz_action_tuning_README.txt`.

## Runtime flow

1. Player holds a spray can supplied by a registered PaintZ paint pack or the PaintZ core stripper can.
2. The can exposes its complete finish ID through `paintzFinish`.
3. PaintZ resolves that finish from the runtime registry.
4. PaintZ resolves the targeted entity.
5. JSON domains decide whether the target is relevant for new-paint interaction.
6. Ordered policy rules decide whether new application is allowed.
7. `PaintZ_PaintInspector` inspects the actual runtime hidden selections.
8. A conservative global heuristic chooses a safe body-like selection or rejects the item.
9. The action duration is derived linearly from the target's physical size and the synchronized action-tuning config.
10. The server rechecks all conditions and required size-dependent consumable quantity when the action completes.
11. For patterned finishes, the server derives a scale from the current scaling config and uses a surface explicitly registered by that finish.
12. PaintZ updates the existing object's shared `ItemBase` PaintZ state and calls `SetObjectTexture()`.
13. The required size-dependent paint/stripper quantity is consumed from the applicator.
14. Shared synchronization publishes the finish, selected surface and transient scale to clients.
15. CF ModStorage preserves only the logical finish assignment through persistence.

No item replacement or classname change occurs.

## Project layout

- `config.cpp` — PaintZ core registration, CF dependency/storage version, Paint Pack API discovery roots, non-spawnable common can base, and paint stripper.
- `config/paintz_items.default.json` — shipped default domains/rules.
- `config/paintz_pattern_scaling.default.json` — shipped default pattern-size mapping.
- `config/paintz_pattern_scaling_README.txt` — runtime scaling configuration contract.
- `config/paintz_action_tuning.default.json` — shipped size/time/consumption defaults.
- `config/paintz_action_tuning_README.txt` — runtime action-tuning formulas and configuration contract.
- `Scripts/4_World/PaintZ/Policy/` — generic type/class/declared-slot domain and rule policy.
- `Scripts/4_World/PaintZ/Actions/PaintZ_ActionTuning.c` — size-dependent action timing/usage, runtime reload and client synchronization.
- `Scripts/4_World/PaintZ/Paint/PaintZ_ItemPaintState.c` — shared inventory-item paint state, dynamic display decoration, synchronization and CF persistence hooks.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintPackRegistry.c` — namespace/finish discovery, validation, collision handling and asset lookup.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintPersistence.c` — CF ModStorage codec and post-load restoration helpers.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PatternScaling.c` — server-side size measurement, scale mapping and live config reload.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintInspector.c` — runtime hidden-selection inspection.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintVisuals.c` — registry-backed texture application/restoration.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintTarget.c` — generic paint/strip dispatch through `ItemBase`.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintedState.c` — existing-paint lookup independent from policy.
- `docs/PAINT_PACK_API.md` — authoritative Paint Pack API identity/interoperability contract.
- `docs/PAINT_PACK_CONFIG_V1.md` — concrete DayZ config representation.
- `docs/ARCHITECTURE.md` — architectural invariants and universality boundary.
- `docs/PERSISTENCE_NOTES.md` — persistence design and acceptance matrix.
- `docs/item-policy.md` — JSON contract.

## Finish content and PackKit

PaintZ core deliberately contains no official finish catalogue, generated finish-specific spray cans, finish surface textures, or per-finish actions.

Official content lives in `netcopdev/PaintZ-Standard-Pack`. Third-party authors can build equivalent packs using `netcopdev/PaintZ-PackKit` or by implementing the documented config contract manually.

The runtime never requires PackKit. PackKit is an offline authoring/generation tool only.

## Universality boundary

The JSON-only category expansion guarantee applies to ordinary `ItemBase`-derived inventory items. Static world objects, buildings, vehicles and other unrelated engine hierarchies may require separate lifecycle/persistence design.

## License

MIT for original PaintZ code.
