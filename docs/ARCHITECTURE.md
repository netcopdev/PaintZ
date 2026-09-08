# PaintZ architecture

## Core invariant

PaintZ is a generic runtime painting framework for DayZ inventory items.

**Target categories are configuration, not code.** Weapons and detachable magazines are only the shipped default domains. Adding another ordinary inventory-item family must require JSON configuration only, provided the target is `ItemBase`-derived and exposes a safe paintable hidden selection.

Examples of intended future JSON-only expansion include suppressors, handguards, stocks, clothing, helmets, backpacks, containers and tools.

The core must not gain a new persistence/state/network implementation for each category.

## Runtime layers

PaintZ deliberately separates three concerns:

1. **Relevance / administration** — `paintz_items.json` domains and ordered include/exclude rules decide whether a new paint application should be considered.
2. **Technical capability** — `PaintZ_PaintInspector` inspects the actual runtime hidden selections and conservatively chooses a safe body-like selection.
3. **Existing PaintZ state** — the logical finish assignment, network synchronization and persistence belong to the physical `ItemBase` instance and are independent from current domains/policy.

A domain can disappear after an item was painted. The existing finish must still restore and remain strippable.

## Generic item state

`PaintZ_ItemPaintState.c` extends `ItemBase` once. The same state path is used by weapons, magazines and every future supported `ItemBase` descendant:

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
- no persistence code change when another normal inventory category is enabled;
- pre-PaintZ items have no PaintZ context and load normally;
- when PaintZ is temporarily absent but CF remains loaded, CF preserves PaintZ's opaque unloaded-mod payload across subsequent saves;
- removing CF as well is outside the persistence guarantee.

Visual restoration is deferred one call-queue turn after CF load so model/hidden-selection work is not performed inside the serializer. Eligibility policy is not consulted while restoring historical state.

## Runtime item policy

Domains are a positive OR-list. Each domain may contain:

- `type`: a DayZ base/config class;
- `class_pattern`: a case-insensitive `*` / `?` classname glob;
- or both, in which case both must match.

The shipped defaults remain `Weapon_Base` and `Magazine_Base`, but these names are data, not special PaintZ architecture.

Rules are evaluated top-to-bottom and the last match wins. A rule's optional `type` is also generic: it may be `all`, a valid DayZ base/config class, or the legacy aliases `weapon` / `magazine` for compatibility with existing version-1 configs. Rules use exactly one selector: `class_pattern` or `inherits`.

## Selection safety

PaintZ discovers compatibility from the live target object. It never needs a registry of supported classnames.

Preferred globally plausible selections include `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag` and `magazine`. Obvious glass, lens, optic, reticle, display, screen, LED, light, emissive, glow and flame selections are blocked. If several selections remain ambiguous, PaintZ rejects the target instead of guessing.

## Object preservation

Painting modifies the existing object with `SetObjectTexture()` and does not replace its classname. Health, ammunition, chamber state, attachments, cargo, inventory location and third-party state remain owned by DayZ/the original mod.

## Boundary of universality

The JSON-only guarantee applies to normal `ItemBase`-derived inventory items. Static world objects, buildings, vehicles or other non-`ItemBase` engine hierarchies may need separate lifecycle/persistence design and are not implicitly covered by adding a domain.
