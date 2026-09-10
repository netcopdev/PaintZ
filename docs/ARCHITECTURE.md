# PaintZ architecture

## Core invariant

PaintZ is a generic runtime painting framework for DayZ inventory items.

**Target categories and attachment families are configuration, not code.** Weapons and detachable magazines are historical/default domains, not architectural special cases. Adding another ordinary inventory-item family must require JSON configuration only, provided the target is `ItemBase`-derived and exposes a safe paintable hidden selection.

The core must not gain a new persistence/state/network implementation for each category or slot family.

A second architecture invariant applies to official content: **PaintZ core owns the official `PZ` namespace identity, but it owns no individual `PZ-*` finishes.** Official content packs are independent contributors to that namespace and must not depend on one another merely for namespace access.

## Runtime layers

PaintZ separates five concerns:

1. **Finish availability** — the Paint Pack API registry discovers installed namespace owners and finish definitions and resolves a canonical finish ID to explicitly declared runtime surfaces.
2. **Relevance / administration** — `paintz_items.json` domains and ordered include/exclude rules decide whether a new paint application should be considered.
3. **Technical capability** — `PaintZ_PaintInspector` inspects actual runtime hidden selections and conservatively chooses a safe body-like selection.
4. **Existing PaintZ state** — logical finish assignment, synchronization and persistence belong to the physical `ItemBase` instance and remain independent from current policy or current finish-pack availability.
5. **Explicit stale-state recovery** — `paintz_stale_finishes.json` may lazily migrate or prune an unregistered persisted finish ID without changing registry identity or introducing category-specific persistence.

A domain can disappear after an item was painted. Likewise a content pack can become temporarily unavailable. Historical finish identity remains stored and the item remains strippable by default. Stale-state recovery changes that historical identity only when a server administrator explicitly configures a migration or destructive prune policy.

## Paint Pack API runtime registry

The authoritative interoperability semantics are in `PAINT_PACK_API.md`; the concrete DayZ config representation is in `PAINT_PACK_CONFIG_V1.md`.

API v1 uses one short canonical finish identity:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples include `PZ-B-BLK`, `PZ-C-FTN`, `PZ-S-FDE` and third-party IDs such as `NCP-C-FTN`.

PaintZ exposes `CfgPaintZPacks` and `CfgPaintZFinishes`. Initialization is two-phase:

1. discover and validate namespace-owner declarations;
2. validate/register finish declarations only under active namespaces.

No first-loaded-wins or last-loaded-wins behavior is permitted. Multiple owner declarations for the same prefix disable that namespace. Duplicate complete finish IDs are disabled rather than overwritten. The current integer-hash network representation also requires active finish hashes to remain unambiguous; a detected hash collision is rejected.

A finish config entry explicitly references the config child class that owns its prefix. This owner key is not a public/persisted identity or security credential; it is config-level linkage.

PaintZ owns one generic paint action. A `PaintZ_SprayCanBase` subclass identifies its finish using `paintzFinish`; runtime behavior is resolved from the registry rather than generated per-finish action subclasses.

Each finish declaration explicitly enumerates the runtime surfaces PaintZ may apply, including pattern-scale variants where applicable. Runtime code does not synthesize third-party asset paths from finish IDs.

Stale-finish migration does not alter this registry. It maps historical persisted state to an already registered destination at the item-state layer. A source ID that is still registered is therefore never eligible for stale migration.

## Core-owned official `PZ` namespace

PaintZ core declares exactly one official namespace owner:

```text
PZ_PaintZOfficial -> PZ
```

That declaration is identity infrastructure only. PaintZ core does not register individual official finishes, ship official finish textures, or ship finish-specific spray cans.

Independent official content packs register unique `PZ-*` finishes against `PZ_PaintZOfficial` and depend directly on `PaintZ_DynamicPaint`. They do not declare another `PZ` owner and do not depend on another official content pack merely for namespace access.

The intended package model is:

```text
CF
└── PaintZ                 owns PZ namespace
    ├── Standard Pack      contributes PZ-* finishes
    ├── Pastel Pack        contributes PZ-* finishes
    ├── Military Pack      contributes PZ-* finishes
    └── Hunting Pack       contributes PZ-* finishes
```

The content packs above are peers. The diagram does not imply that PaintZ requires them; each content pack requires PaintZ.

Package names/categories are not canonical identity. An unchanged finish may move between official packages while retaining its complete `PZ-*` ID. This is a packaging change, not a persistence migration.

Additional valid `PZ?` prefixes remain reserved but unassigned. Do not consume them merely to encode content categories.

Third-party multi-PBO namespace families may still use the separate owner/satellite mechanism: one external owner PBO declares the namespace and its satellite PBOs depend on that owner. This mechanism is not needed for normal official `PZ` collections because PaintZ core is already their stable owner.

## Generic item state

`PaintZ_ItemPaintState.c` extends `ItemBase` once. The same state path is used by weapons, magazines, attachments and every future supported `ItemBase` descendant:

- canonical finish ID;
- resolved paint selection;
- network hash/synchronization;
- transient derived pattern-scale percentage;
- persistent logical assignment;
- transient stale-recovery config revision already evaluated for that item.

`PaintZ_PaintTarget` does not dispatch by category. It casts the target to `ItemBase` and updates the shared state.

Pattern-scale percentage is networked because clients need the same registered surface variant, but it is not persistent logical state. The stale-recovery revision is server-local transient bookkeeping and is neither persisted nor networked.

The complete finish string is retained authoritatively on server/persistence paths. Network replication currently uses the finish string's integer hash plus selection/scale because DayZ does not provide a normal `EntityAI` string net-sync variable. The registry therefore rejects active finish-hash collisions.

## Persistence

PaintZ requires Community Framework and uses **CF ModStorage** on `ItemBase`.

This choice avoids appending PaintZ bytes in a broad native `ItemBase::OnStoreSave()` stream where derived DayZ classes may serialize their own data after `super`.

PaintZ implements only `CF_OnStoreSave` / `CF_OnStoreLoad` on `ItemBase` and persists the canonical finish ID.

Consequences:

- no weapon-specific persistence hook;
- no magazine-specific persistence hook;
- no attachment-specific persistence hook;
- no persistence code change when another ordinary inventory category is enabled;
- pre-PaintZ items load normally without PaintZ context;
- when PaintZ is temporarily absent but CF remains loaded, CF can preserve PaintZ's opaque unloaded-mod payload;
- removing CF as well is outside the persistence guarantee.

Unknown/unregistered finish IDs are valid historical state and are preserved by default. If a content pack is missing, conflicted or invalid, PaintZ does not implicitly erase the stored ID. It restores/keeps the original target visual where possible and leaves Strip Paint available. Reinstalling the same valid finish later allows normal restoration.

An administrator may explicitly configure stale-state recovery. Exact migration replaces an unregistered historical ID with a currently registered destination through the same `ItemBase` state path. `prune_unknown` may clear an otherwise-unmapped stale assignment. These operations are explicit persistence repair, not implicit consequences of a pack being absent or a target being excluded by policy.

This directly supports official catalogue reorganization without recovery when IDs stay unchanged. If `PZ-C-FTN` moves from Standard Pack to Military Pack, its persisted identity remains `PZ-C-FTN`; only which package supplies the registration/assets changes. Stale recovery is needed only when identity itself is deliberately retired/changed or abandoned state is deliberately cleaned up.

Pattern scale is excluded from persistence. On post-load restoration PaintZ remeasures the target and derives the current scale before applying a registered surface. Eligibility policy is not consulted while restoring historical state.

## Stale finish recovery

`$profile:PaintZ/paintz_stale_finishes.json` is a server-side persistence-repair layer for currently unregistered PaintZ finish IDs.

Its resolution rules are intentionally narrow:

1. registered source ID -> untouched;
2. unregistered source with exact migration -> migrate only to a currently registered destination;
3. unregistered source without migration and `prune_unknown = true` -> clear PaintZ state;
4. otherwise -> preserve historical state.

Migration takes precedence over pruning. A failed matching migration preserves state and does not fall through to prune. Chains and wildcard mappings are rejected.

Recovery is lazy and centralized through shared `ItemBase` state. It runs before post-load visual restoration and before server-side operations that depend on existing PaintZ state. It does not scan persistence files or enumerate every loaded entity when configuration changes.

Each item tracks only the transient successful config revision it has already evaluated. A later successful reload permits reevaluation on the next encounter; repeated action-condition ticks under the same revision do not repeatedly execute/warn about the same stale state.

A successful migration stores only the destination canonical finish ID in normal item state. No alias, migration marker, source ID, registry path or recovery metadata is added to CF persistence.

A prune with a safe recoverable selection restores that selection's configured/original texture before clearing logical state. If no safe selection exists, PaintZ clears the logical stale assignment without guessing a texture index.

See `STALE_FINISH_RECOVERY.md`.

## Runtime item policy

Domains are a positive OR-list. Shared selector families use the same singular/plural schema in both domains and rules:

- `type` / `types`;
- `class_pattern` / `class_patterns`;
- `inventory_slot` / `inventory_slots`;
- `inventory_slot_pattern` / `inventory_slot_patterns`.

Within one selector family, singular and plural values form one OR group. Different selector families inside the same domain/rule object are AND. Separate domain objects are OR.

Slot matching uses the target class's **declared compatible slot data**, not its current attachment state. A stock, optic, suppressor or flashlight lying loose on the ground still matches the slots its config declares.

Rules are evaluated top-to-bottom and last matching rule wins. Type scope is optional and may use valid DayZ base/config classes plus legacy `weapon` / `magazine` aliases for compatibility.

Policy affects new painting/repainting only. Existing PaintZ state remains independent and stripping remains available after exclusion/domain removal. Policy exclusion does not make a registered finish stale.

## Selection safety

PaintZ discovers compatibility from the actual target object/model. It does not maintain a supported-classname registry and does not hard-exclude sensible inventory families such as optics or flashlights.

Preferred globally plausible selections include `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag`, `magazine`, `housing`, `shell` and `frame`.

Functional surfaces indicating glass, lens, reticle, display, screen, LED, emissive/glow or flame are blocked. Broad category words such as `optic` and `light` are not blocked by themselves because names such as `optic_body` or `flashlight_body` may be legitimate housings.

If several selections remain ambiguous, PaintZ rejects the target instead of guessing.

The same inspection safety applies to stale migration. An unresolved historical ID is not an excuse to guess which selection should receive a replacement finish.

## Pattern scale normalization

Pattern scale is derived visual state, not finish identity.

For a patterned finish, the server uses the target collision box and takes the longest local dimension as a generic physical-size proxy. `$profile:PaintZ/paintz_pattern_scaling.json` maps ascending dimensions to requested scale values.

Scale availability is per finish. The active content pack explicitly declares every available scale percentage. PaintZ never invents an undeclared variant path.

A configured scale must be a positive whole percentage up to 1000%. If the requested scale is unavailable, PaintZ falls back to the finish's configured/default 100% representation.

Scale is derived at application/repaint, stale migration and post-load restoration. Reloading scale configuration does not enumerate and mutate all already-loaded painted objects.

Longest collision-box size is intentionally an approximation rather than a promise of real-world texel density. Different UV layouts can produce different apparent pattern scales on similarly sized objects.

## Size-dependent action tuning

Painting and stripping reuse the longest-collision-box measurement exposed by `PaintZ_PatternScaling.GetMaxDimensionMeters()`.

`$profile:PaintZ/paintz_action_tuning.json` defines physical-size endpoints, action-time endpoints and independent paint/stripper applications-per-full-can values.

Measured size is clamped between configured endpoints. Action duration is linearly interpolated and consumable usage is proportional to the same clamped physical size.

The server owns/reloads this configuration and synchronizes valid settings to clients because continuous-action progress must use consistent timing. Server completion recalculates/revalidates required quantity before modifying the target or consuming the applicator.

## Object preservation

Painting and stale migration modify the existing object with `SetObjectTexture()` or the verified current equivalent and do not replace its classname.

Preserve health, ammunition, chamber/FSM state, attachments, cargo, inventory location, parent/container relationships and third-party state owned by the original item/mod.

Pattern scaling changes only which registered diffuse/coating surface is selected. PaintZ does not replace the target RVMat/material unless a separately designed finish explicitly requires such behavior.

## Dependency boundary

Runtime dependency direction is:

```text
Official/third-party content pack -> PaintZ -> CF
```

PackKit is offline tooling:

```text
PackKit --generates--> content pack
```

Forbidden runtime dependencies include:

```text
PaintZ -> Standard/Military/Pastel/Hunting pack
PaintZ -> PackKit
official content pack -> another official content pack merely for PZ access
```

Stale migration does not create a dependency on the pack that originally supplied the stale ID. It only requires that the configured destination be present in the current PaintZ registry when encountered.

## Boundary of universality

The JSON-only category expansion guarantee applies to ordinary `ItemBase`-derived inventory items. Static world objects, buildings, vehicles or other unrelated engine hierarchies may need separate lifecycle/persistence design and are not implicitly covered by adding a domain or stale-recovery rule.
