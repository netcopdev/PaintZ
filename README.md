# PaintZ

PaintZ is a generic DayZ runtime painting framework for ordinary inventory items. Paintability is determined from server policy plus the target's runtime model/hidden selections rather than per-class compatibility code.

Painting modifies the existing item instance. PaintZ does not replace the item with a painted subclass or change its classname.

## Runtime model

PaintZ separates four concerns:

- **finish availability** — registered Paint Pack API finishes;
- **server policy** — which item families may receive new paint;
- **model safety** — whether the target exposes a safe paintable hidden selection;
- **item state** — synchronization, persistence and stripping of the applied logical finish.

Weapons and detachable magazines are default policy domains, not hard-coded architectural categories. Other normal `ItemBase`-derived inventory families can be enabled through JSON when their models expose a safe paintable selection.

## Dependencies

PaintZ requires **Community Framework (CF)** and uses CF ModStorage for paint persistence.

Runtime dependency direction is:

```text
Paint Pack -> PaintZ -> CF
```

PaintZ does not depend on any individual content pack.

## Paint Pack API

PaintZ core owns the official `PZ` namespace identity through:

```text
PZ_PaintZOfficial
```

It deliberately contains no individual official finishes. Official content packs register unique `PZ-*` finishes against that owner and depend directly on PaintZ.

Third-party packs use their own non-reserved 2-3 character namespace.

Canonical finish identity is:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples:

```text
PZ-B-BLK
PZ-C-FTN
NCP-S-FDE
```

Duplicate namespace ownership and duplicate complete finish IDs are rejected rather than resolved by load order.

See:

- `docs/PAINT_PACK_API.md`
- `docs/PAINT_PACK_CONFIG_V1.md`
- `docs/PAINT_PACK_RUNTIME_ACCEPTANCE.md`

## Persistence

PaintZ stores the canonical finish ID once at the shared `ItemBase` level using CF ModStorage.

Derived values such as texture paths, selection indexes and pattern scale are not persistent identity.

If a registered finish later becomes unavailable, PaintZ preserves the stored logical finish by default. The item remains strippable and can resolve again if the same finish becomes available in a later session.

Optional stale-finish repair is configured in:

```text
$profile:PaintZ/paintz_stale_finishes.json
```

It can migrate an exact unregistered historical ID to a currently registered replacement or deliberately prune otherwise-unmapped stale PaintZ state. The bundled default is non-destructive.

See:

- `docs/PERSISTENCE_NOTES.md`
- `docs/STALE_FINISH_RECOVERY.md`

## Server item policy

New-paint eligibility is configured in:

```text
$profile:PaintZ/paintz_items.json
```

PaintZ creates it from `config/paintz_items.default.json` when missing and does not overwrite an existing administrator copy.

Domains and ordered include/exclude rules can use:

- DayZ base/config types;
- classname wildcards;
- exact declared `inventorySlot` values;
- wildcard matching over declared `inventorySlot` values.

Policy affects new painting/repainting only. Existing PaintZ state and stripping remain independent from later eligibility changes.

See `docs/item-policy.md` and `config/paintz_items_README.txt`.

## Hidden-selection safety

PaintZ inspects the actual target model at runtime.

It prefers plausible body/camo/housing selections and rejects clearly functional surfaces such as glass, lenses, reticles, displays and emissive elements. If the available selections are ambiguous, PaintZ rejects the target rather than guessing.

## Pattern scaling

Patterned finishes may register multiple scale variants. PaintZ measures the target's longest collision-box dimension and requests a configured scale from:

```text
$profile:PaintZ/paintz_pattern_scaling.json
```

Only variants explicitly registered by the active finish may be used. Pattern scale is derived state and is recalculated when needed.

See `config/paintz_pattern_scaling_README.txt`.

## Action tuning

Painting/stripping duration and consumable usage are size-dependent and configured in:

```text
$profile:PaintZ/paintz_action_tuning.json
```

See `config/paintz_action_tuning_README.txt`.

## Painted item identification

Painted items expose their PaintZ finish through DayZ's dynamic name and description hooks. For example:

```text
KA-74 [Flecktarn]
Finish: Flecktarn (PZ-C-FTN)
```

Stripping clears only PaintZ logical finish state and restores the target's original appearance where possible.

## Core responsibilities

PaintZ core contains:

- runtime finish registry and validation;
- the official `PZ` namespace owner;
- generic paint and strip actions;
- common spray-can base and stripper;
- item policy and hidden-selection inspection;
- synchronization and CF persistence;
- stale-finish recovery;
- pattern scaling and action tuning.

Finish definitions, spray-can content and finish assets belong to independent content-pack repositories. PaintZ-PackKit is offline authoring/build tooling and is not a runtime dependency.

## Project documentation

- `docs/ARCHITECTURE.md` — architectural invariants and runtime boundaries.
- `docs/PAINT_PACK_API.md` — authoritative interoperability contract.
- `docs/PAINT_PACK_CONFIG_V1.md` — concrete DayZ config representation.
- `docs/PERSISTENCE_NOTES.md` — persistence design and acceptance cases.
- `docs/STALE_FINISH_RECOVERY.md` — stale-ID migration/pruning semantics.
- `docs/item-policy.md` — runtime JSON policy contract.
- `docs/RELEASE_ACCEPTANCE.md` — release acceptance checklist.

## Universality boundary

The JSON-only category-expansion guarantee applies to ordinary `ItemBase`-derived inventory items. Static world objects, buildings, vehicles and unrelated engine hierarchies may require separate lifecycle/persistence design.

## License

MIT for original PaintZ code.
