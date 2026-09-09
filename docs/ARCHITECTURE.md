# PaintZ architecture

## Core invariant

PaintZ is a generic runtime painting framework for DayZ inventory items.

**Target categories and attachment families are configuration, not code.** Weapons and detachable magazines are historical/default domains, not architectural special cases. Adding another ordinary inventory-item family must require JSON configuration only, provided the target is `ItemBase`-derived and exposes a safe paintable hidden selection.

Intended JSON-only expansion includes suppressors, muzzle devices, handguards, stocks, grips, bipods, optics, flashlights, clothing, helmets, backpacks, containers and tools.

The core must not gain a new persistence/state/network implementation for each category or slot family.

## Runtime layers

PaintZ deliberately separates three concerns:

1. **Relevance / administration** — `paintz_items.json` domains and ordered include/exclude rules decide whether a new paint application should be considered.
2. **Technical capability** — `PaintZ_PaintInspector` inspects the actual runtime hidden selections and conservatively chooses a safe body-like selection.
3. **Existing PaintZ state** — the logical finish assignment, network synchronization and persistence belong to the physical `ItemBase` instance and are independent from current domains/policy.

A domain can disappear after an item was painted. The existing finish must still restore and remain strippable.

## Generic item state

`PaintZ_ItemPaintState.c` extends `ItemBase` once. The same state path is used by weapons, magazines, attachments and every future supported `ItemBase` descendant:

- canonical finish ID;
- resolved paint selection;
- network hash/synchronization;
- transient derived pattern-scale percentage;
- persistent logical assignment.

`PaintZ_PaintTarget` does not dispatch by category. It casts the target to `ItemBase` and updates the shared state.

The pattern-scale percentage is networked because clients need to resolve the same derived surface texture, but it is not persistent logical state.

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

Pattern scale is deliberately excluded from persistence. The stored finish ID remains stable while the visual scale can evolve with configuration. Visual restoration is deferred one call-queue turn after CF load so model/hidden-selection work is not performed inside the serializer. During that restore PaintZ remeasures the target and derives the current scale before applying the texture. Eligibility policy is not consulted while restoring historical state.

## Runtime item policy

Domains are a positive OR-list. Each domain may contain any combination of:

- `type`: a DayZ base/config class;
- `class_pattern`: a case-insensitive `*` / `?` classname glob;
- `inventory_slot`: an exact declared compatible `inventorySlot`;
- `inventory_slot_pattern`: a case-insensitive `*` / `?` glob over declared compatible `inventorySlot` values.

All supplied fields inside one domain are AND. Separate domain objects are OR.

Slot matching uses the target class's **declared compatible slot data**, not the item's current attachment state. That means a stock, optic, suppressor or flashlight lying loose on the ground still matches the slots its config says it can occupy. PaintZ reads inherited slot data from the target's actual config root and does not maintain a classname-to-slot registry.

The bundled default config now includes weapon/magazine domains, common weapon/pistol/suppressor slot families, and `SmallProtectorCase`. These values are data, not special PaintZ architecture.

Rules are evaluated top-to-bottom and the last match wins. A rule's optional `type` is generic: it may be `all`, a valid DayZ base/config class, or legacy aliases `weapon` / `magazine` for compatibility with existing version-1 configs.

Each rule uses exactly one selector from:

- `class_pattern`;
- `inherits`;
- `inventory_slot`;
- `inventory_slot_pattern`.

No new rule/category enum should be needed when a third-party mod introduces another slot family.

## Selection safety

PaintZ discovers compatibility from the actual target object/model. It never needs a registry of supported classnames and it does not hard-exclude item families such as optics or flashlights.

Preferred globally plausible selections include `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag`, `magazine`, `housing`, `shell` and `frame`.

Functional surfaces remain protected. Selection names indicating glass, lens, reticle, display, screen, LED, emissive/glow or flame surfaces are blocked. Broad category words such as `optic` and `light` are **not** blocked by themselves, because names such as `optic_body` or `flashlight_body` may represent legitimate housings.

If several selections remain ambiguous, PaintZ rejects the target instead of guessing. Consequently, two optics from different mods may behave differently: one can paint because it exposes a safe body/camo/housing selection while another remains unsupported because it exposes only lens/reticle surfaces.

## Pattern scale normalization

Pattern scale is derived visual state, not finish identity.

For a patterned finish, the server uses `GetCollisionBox()` on the actual target and takes the longest local collision-box dimension as a generic physical-size proxy. `$profile:PaintZ/paintz_pattern_scaling.json` maps ascending maximum dimensions to scale values backed by generated texture variants.

The generator owns the allowed scale set. Runtime config validation rejects any scale that has no generated asset. The existing 1x filename remains stable; additional variants use percentage suffixes such as `_s050`, `_s150` and `_s200`.

Scale is derived only at two lifecycle points:

1. a new paint/repaint application;
2. post-load restoration of a persisted finish.

Reloading the scale config does not enumerate or mutate already-loaded painted objects. The next application uses the new mapping immediately, and a later server restart/load re-derives the scale from the same persisted finish ID.

The server synchronizes the chosen scale percentage with the existing ItemBase paint state so clients resolve the same texture path. Clients do not need a copy of the server scaling config.

Longest collision-box size is intentionally an approximation rather than a promise of real-world texel density. Model UV layouts can differ significantly, including between similarly sized magazines, stocks, suppressors and optics. The design therefore improves consistency without introducing per-class compatibility tables.

Solid paints always resolve to the normal 1x texture.

## Size-dependent action tuning

Painting and stripping reuse the same longest-collision-box measurement exposed by `PaintZ_PatternScaling.GetMaxDimensionMeters()`. Size measurement is therefore implemented once and consumed by both pattern normalization and action tuning.

`$profile:PaintZ/paintz_action_tuning.json` defines two physical-size endpoints, two action-time endpoints, and independent paint/stripper applications-per-full-can values. There are no discrete size buckets.

For action time, the measured dimension is clamped to the configured minimum/maximum and linearly interpolated between the configured minimum/maximum times. For paint and stripper consumption, the same clamped dimension is proportional to `max_dimension_m`; the max-size cost is one full can divided by the configured applications-per-full-can value.

With shipped defaults, 0.2 m maps to 5 seconds and 1/12 of a full can, while 0.8 m maps to 20 seconds and 1/3 of a full can. An unmeasurable target conservatively falls back to the max-size endpoint.

The server owns and reloads this config. Every valid load/reload is synchronized to clients because `CAContinuousTime` is created on both sides and client/server action duration must agree. Server completion still recalculates/revalidates required quantity before modifying the target or consuming the applicator.

The active duration for an already-running action is fixed when its action component is created. A later config reload affects newly started actions; the server's completion-time quantity check always uses the currently active authoritative settings.

## Object preservation

Painting modifies the existing object with `SetObjectTexture()` and does not replace its classname. Health, ammunition, chamber state, attachments, cargo, inventory location and third-party state remain owned by DayZ/the original mod.

Pattern scaling changes only which generated diffuse/coating texture path is selected. PaintZ still does not replace the target RVMat/material.

## Boundary of universality

The JSON-only guarantee applies to normal `ItemBase`-derived inventory items. Static world objects, buildings, vehicles or other non-`ItemBase` engine hierarchies may need separate lifecycle/persistence design and are not implicitly covered by adding a domain.
