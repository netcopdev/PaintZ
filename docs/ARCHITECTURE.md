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
- persistent logical assignment.

`PaintZ_PaintTarget` does not dispatch by category. It casts the target to `ItemBase` and updates the shared state.

## Persistence

PaintZ requires Community Framework and uses **CF ModStorage** on `ItemBase`.

This choice is intentional. Appending PaintZ bytes in a broad native `ItemBase::OnStoreSave()` override is unsafe because derived DayZ classes can serialize additional state after calling `super`; inserting PaintZ data at the base layer can therefore place it in the middle of a subclass stream and break legacy or third-party persistence.

CF already owns the safe base persistence wrapper and gives each mod an isolated versioned storage context. PaintZ implements only `CF_OnStoreSave` / `CF_OnStoreLoad` on `ItemBase` and writes the canonical finish ID into the `PaintZ` ModStorage context.

Consequences:

- no weapon-specific persistence hook;
- no magazine-specific persistence hook;
- no attachment-specific persistence hook;
- no persistence code change when another normal inventory category is enabled;
- pre-PaintZ items have no PaintZ context and load normally;
- when PaintZ is temporarily absent but CF remains loaded, CF preserves PaintZ's opaque unloaded-mod payload across subsequent saves;
- removing CF as well is outside the persistence guarantee.

Visual restoration is deferred one call-queue turn after CF load so model/hidden-selection work is not performed inside the serializer. Eligibility policy is not consulted while restoring historical state.

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

## Object preservation

Painting modifies the existing object with `SetObjectTexture()` and does not replace its classname. Health, ammunition, chamber state, attachments, cargo, inventory location and third-party state remain owned by DayZ/the original mod.

## Boundary of universality

The JSON-only guarantee applies to normal `ItemBase`-derived inventory items. Static world objects, buildings, vehicles or other non-`ItemBase` engine hierarchies may need separate lifecycle/persistence design and are not implicitly covered by adding a domain.
