# PaintZ architecture

## Core invariant

PaintZ is a generic runtime painting framework for DayZ inventory items.

**Target categories and attachment families are configuration, not code.** Weapons and detachable magazines are historical/default domains, not architectural special cases. Adding another ordinary inventory-item family must require JSON configuration only, provided the target is `ItemBase`-derived and exposes a safe paintable hidden selection.

Intended JSON-only expansion includes suppressors, muzzle devices, handguards, stocks, grips, bipods, optics, flashlights, clothing, helmets, backpacks, containers and tools.

The core must not gain a new persistence/state/network implementation for each category or slot family.

## Runtime layers

PaintZ deliberately separates four concerns:

1. **Finish availability** — the Paint Pack API runtime registry discovers installed pack namespaces and finish definitions and resolves a canonical finish ID to explicitly declared assets.
2. **Relevance / administration** — `paintz_items.json` domains and ordered include/exclude rules decide whether a new paint application should be considered.
3. **Technical capability** — `PaintZ_PaintInspector` inspects the actual runtime hidden selections and conservatively chooses a safe body-like selection.
4. **Existing PaintZ state** — the logical finish assignment, network synchronization and persistence belong to the physical `ItemBase` instance and are independent from current domains/policy or whether the finish pack is currently available.

A domain can disappear after an item was painted. Likewise a paint pack can become temporarily unavailable. The historical finish ID must remain stored and the item must remain strippable.

## Paint Pack API runtime registry

The authoritative interoperability semantics are in `PAINT_PACK_API.md`; the current DayZ config representation is in `PAINT_PACK_CONFIG_V1.md`.

API v1 uses one short canonical finish identity:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples include `PZ-C-FTN`, `PZ-S-FDE` and third-party IDs such as `NCP-C-FTN`.

PaintZ exposes `CfgPaintZPacks` and `CfgPaintZFinishes` discovery roots. Runtime initialization is two-phase:

1. discover and validate namespace-owner declarations;
2. only then validate/register finish declarations under active namespaces.

No first-loaded-wins or last-loaded-wins behavior is permitted. Multiple owner declarations for the same prefix disable that namespace. Duplicate complete finish IDs are disabled rather than overwritten. The current int-hash network representation also requires finish hashes to remain unambiguous inside the active registry; a detected hash collision is rejected rather than silently resolving to the wrong finish.

A finish config entry explicitly references the config child class that owns its prefix. This owner key is not a second public/persisted ID and is not a security credential; it is config-level linkage between finish declarations and the active namespace owner.

PaintZ owns one generic paint action. A `PaintZ_SprayCanBase` subclass identifies its finish using `paintzFinish`; runtime behavior is resolved from the registry rather than generated per-finish action subclasses.

Each finish declaration explicitly enumerates the surface assets PaintZ may apply, including any pattern-scale variants. Runtime code does not synthesize third-party texture paths from the finish ID.

During migration, the current built-in generated `PZ-*` catalogue is bridged into the registry as an internal `PZ` owner. This bridge is temporary and must be removed when `PaintZ-Standard-Pack` becomes the external authoritative owner of `PZ`.

## Generic item state

`PaintZ_ItemPaintState.c` extends `ItemBase` once. The same state path is used by weapons, magazines, attachments and every future supported `ItemBase` descendant:

- canonical finish ID;
- resolved paint selection;
- network hash/synchronization;
- transient derived pattern-scale percentage;
- persistent logical assignment.

`PaintZ_PaintTarget` does not dispatch by category. It casts the target to `ItemBase` and updates the shared state.

The pattern-scale percentage is networked because clients need to resolve the same declared surface variant, but it is not persistent logical state.

The complete finish string is retained authoritatively on the server/persistence path. Network replication currently uses the finish string's integer hash plus selection/scale because DayZ does not provide a normal `EntityAI` string net-sync variable. The runtime registry therefore rejects active finish-hash collisions. If a finish is unresolved client-side, the synchronized selection still indicates historical PaintZ state so stripping can remain available even though the unavailable finish name cannot be reconstructed from the hash alone.

## Persistence

PaintZ requires Community Framework and uses **CF ModStorage** on `ItemBase`.

This choice is intentional. Appending PaintZ bytes in a broad native `ItemBase::OnStoreSave()` override is unsafe because derived DayZ classes can serialize additional data after calling `super`; inserting PaintZ data at the base layer can therefore place it in the middle of a subclass stream and break legacy or third-party persistence.

CF already owns the safe base persistence wrapper and gives each mod an isolated versioned storage context. PaintZ implements only `CF_OnStoreSave` / `CF_OnStoreLoad` on `ItemBase` and writes the canonical finish ID into the `PaintZ` ModStorage context.

Consequences:

- no weapon-specific persistence hook;
- no magazine-specific persistence hook;
- no attachment-specific persistence hook;
- no persistence code change when another normal inventory category is enabled;
- pre-PaintZ items have no PaintZ context and load normally;
- when PaintZ is temporarily absent but CF remains loaded, CF preserves PaintZ's opaque unloaded-mod payload across subsequent saves;
- removing CF as well is outside the persistence guarantee.

Unknown/unregistered finish IDs are valid historical PaintZ state. If the owning pack is missing, conflicted or otherwise unavailable, PaintZ does not erase the stored string. It restores/keeps the original target visual where possible, keeps the logical ID on the server, and leaves Strip Paint available. Reinstalling the same valid finish later allows normal restoration again.

Pattern scale is deliberately excluded from persistence. The stored finish ID remains stable while the visual scale can evolve with configuration and with the variants supplied by the currently installed pack. Visual restoration is deferred one call-queue turn after CF load so model/hidden-selection work is not performed inside the serializer. During that restore PaintZ remeasures the target and derives the current scale before applying the texture. Eligibility policy is not consulted while restoring historical state.

## Runtime item policy

Domains are a positive OR-list. Shared selector families use the same singular/plural schema in both domains and rules:

- `type` / `types`;
- `class_pattern` / `class_patterns`;
- `inventory_slot` / `inventory_slots`;
- `inventory_slot_pattern` / `inventory_slot_patterns`.

Within one selector family, singular and plural values form one OR group. Different selector families inside the same domain/rule object are AND. This naming and matching convention is an architecture invariant for future shared selectors.

A domain may contain any combination of those selector families. Separate domain objects are OR.

Slot matching uses the target class's **declared compatible slot data**, not the item's current attachment state. That means a stock, optic, suppressor or flashlight lying loose on the ground still matches the slots its config says it can occupy. PaintZ reads inherited slot data from the target's actual config root and does not maintain a classname-to-slot registry.

The bundled default config includes weapon/magazine type alternatives, common weapon/pistol/suppressor slot-pattern alternatives, and `SmallProtectorCase`. These values are data, not special PaintZ architecture.

Rules are evaluated top-to-bottom and the last match wins. A rule's optional type scope uses `type`, `types`, or both. Scope may be `all`, valid DayZ base/config classes, or legacy aliases `weapon` / `magazine` for compatibility with existing version-1 configs. If type scope is omitted, it normalizes to `all`.

A rule must contain at least one non-type selector family. Supported rule selectors are:

- `class_pattern` / `class_patterns`;
- `inherits` / `inherits_any`;
- `inventory_slot` / `inventory_slots`;
- `inventory_slot_pattern` / `inventory_slot_patterns`.

Multiple selector families in one rule are valid and are ANDed. Alternatives within one family are ORed. No new rule/category enum should be needed when a third-party mod introduces another slot family.

Every policy field that affects client-visible matching is synchronized in both singular and plural form so client and server evaluate the same policy shape.

## Selection safety

PaintZ discovers compatibility from the actual target object/model. It never needs a registry of supported classnames and it does not hard-exclude item families such as optics or flashlights.

Preferred globally plausible selections include `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag`, `magazine`, `housing`, `shell` and `frame`.

Functional surfaces remain protected. Selection names indicating glass, lens, reticle, display, screen, LED, emissive/glow or flame surfaces are blocked. Broad category words such as `optic` and `light` are **not** blocked by themselves, because names such as `optic_body` or `flashlight_body` may represent legitimate housings.

If several selections remain ambiguous, PaintZ rejects the target instead of guessing. Consequently, two optics from different mods may behave differently: one can paint because it exposes a safe body/camo/housing selection while another remains unsupported because it exposes only lens/reticle surfaces.

## Pattern scale normalization

Pattern scale is derived visual state, not finish identity.

For a patterned finish, the server uses `GetCollisionBox()` on the actual target and takes the longest local collision-box dimension as a generic physical-size proxy. `$profile:PaintZ/paintz_pattern_scaling.json` maps ascending maximum dimensions to requested scale values.

Scale availability is **per finish**. The active paint pack explicitly declares every scale percentage available for each patterned finish. The runtime no longer assumes one global generated scale set for all packs and never invents an undeclared variant path.

A configured scale must be a positive whole percentage up to 1000%. On application/restore, PaintZ asks whether the selected finish actually provides the requested percentage. If not, it falls back to that finish's configured default when available and finally to the mandatory 100% surface.

The current legacy/Standard finishes expose the familiar 50%, 75%, 100%, 150%, 200% and 300% set, but third-party packs may legitimately expose a different subset as long as 100% exists.

Scale is derived only at two lifecycle points:

1. a new paint/repaint application;
2. post-load restoration of a persisted finish.

Reloading the scale config does not enumerate or mutate already-loaded painted objects. The next application uses the new mapping immediately, and a later server restart/load re-derives the scale from the same persisted finish ID.

The server synchronizes the chosen scale percentage with the existing ItemBase paint state so clients resolve the same registered texture. Clients do not need a copy of the server scaling config.

Longest collision-box size is intentionally an approximation rather than a promise of real-world texel density. Model UV layouts can differ significantly, including between similarly sized magazines, stocks, suppressors and optics. The design therefore improves consistency without introducing per-class compatibility tables.

Solid/non-pattern finishes always resolve to their normal 100% surface.

## Size-dependent action tuning

Painting and stripping reuse the same longest-collision-box measurement exposed by `PaintZ_PatternScaling.GetMaxDimensionMeters()`. Size measurement is therefore implemented once and consumed by both pattern normalization and action tuning.

`$profile:PaintZ/paintz_action_tuning.json` defines two physical-size endpoints, two action-time endpoints, and independent paint/stripper applications-per-full-can values. There are no discrete size buckets.

For action time, the measured dimension is clamped to the configured minimum/maximum and linearly interpolated between the configured minimum/maximum times. For paint and stripper consumption, the same clamped dimension is proportional to `max_dimension_m`; the max-size cost is one full can divided by the configured applications-per-full-can value.

With shipped defaults, 0.2 m maps to 5 seconds and 1/12 of a full can, while 0.8 m maps to 20 seconds and 1/3 of a full can. An unmeasurable target conservatively falls back to the max-size endpoint.

The server owns and reloads this config. Every valid load/reload is synchronized to clients because `CAContinuousTime` is created on both sides and client/server action duration must agree. Server completion still recalculates/revalidates required quantity before modifying the target or consuming the applicator.

The active duration for an already-running action is fixed when its action component is created. A later config reload affects newly started actions; the server's completion-time quantity check always uses the currently active authoritative settings.

## Object preservation

Painting modifies the existing object with `SetObjectTexture()` and does not replace its classname. Health, ammunition, chamber state, attachments, cargo, inventory location and third-party state remain owned by DayZ/the original mod.

Pattern scaling changes only which registered diffuse/coating texture is selected. PaintZ still does not replace the target RVMat/material.

## Boundary of universality

The JSON-only guarantee applies to normal `ItemBase`-derived inventory items. Static world objects, buildings, vehicles or other non-`ItemBase` engine hierarchies may need separate lifecycle/persistence design and are not implicitly covered by adding a domain.
