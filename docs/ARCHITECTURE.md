# PaintZ architecture

## Core invariants

PaintZ is a generic runtime painting framework for ordinary DayZ inventory items.

**Target categories and attachment families are configuration, not code.** Weapons and detachable magazines are default policy domains, not architectural special cases. Adding another ordinary `ItemBase`-derived inventory family should require JSON configuration only when the target model exposes a safe paintable hidden selection.

PaintZ state, synchronization and persistence belong once at the shared `ItemBase` level. The core must not gain separate weapon, magazine, attachment or clothing implementations for the same logical paint state.

A second invariant applies to official content: **PaintZ core owns the official `PZ` namespace identity, but owns no individual `PZ-*` finishes.** Official content packs contribute finishes to that namespace and depend directly on PaintZ rather than on one another.

## Runtime layers

PaintZ separates these concerns:

1. **Finish availability** — the Paint Pack API registry discovers namespace owners and finish definitions and resolves canonical finish IDs to explicitly declared runtime surfaces.
2. **Administration** — `paintz_items.json` decides whether a new paint application is relevant and allowed.
3. **Technical capability** — `PaintZ_PaintInspector` inspects the target model and conservatively selects a safe paintable hidden selection.
4. **Existing PaintZ state** — logical finish assignment, synchronization and persistence belong to the physical `ItemBase` instance and remain independent from current eligibility policy.
5. **Persistence repair** — `paintz_stale_finishes.json` may explicitly migrate or prune unregistered persisted finish IDs.

Changing policy after an item was painted does not erase its state. Likewise, temporarily missing content does not implicitly strip an item.

## Paint Pack API registry

The authoritative interoperability contract is `PAINT_PACK_API.md`; the concrete DayZ config representation is `PAINT_PACK_CONFIG_V1.md`.

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

PaintZ exposes `CfgPaintZPacks` and `CfgPaintZFinishes` and initializes the registry in two phases:

1. discover and validate namespace owners;
2. validate/register finish declarations only under active namespaces.

No first-loaded-wins or last-loaded-wins behavior is permitted. Duplicate namespace ownership disables that namespace. Duplicate complete finish IDs are rejected rather than overwritten. Active finish hashes must also remain unambiguous for network representation.

Each finish declaration explicitly names its owner and available runtime surfaces. PaintZ never synthesizes third-party asset paths from naming conventions.

## Official `PZ` namespace

PaintZ core declares exactly one official owner:

```text
PZ_PaintZOfficial -> PZ
```

That declaration is identity infrastructure only. PaintZ core does not register official finish content or ship finish-specific spray cans/assets.

Independent official packs register unique `PZ-*` finishes against `PZ_PaintZOfficial` and depend directly on `PaintZ_DynamicPaint`.

Package names are distribution concerns, not canonical finish identity. An unchanged official finish may move between official packages without changing its `PZ-*` identity.

Additional valid `PZ?` prefixes remain reserved but unassigned unless explicitly assigned by a future API decision.

Third-party standalone packs own their own non-reserved namespaces. Third-party multi-PBO families may use one namespace owner plus dependent satellite PBOs.

## Generic item state

`PaintZ_ItemPaintState.c` extends `ItemBase` once. The same state path is used by weapons, magazines, attachments and other supported ordinary inventory items.

Shared state includes:

- canonical finish ID;
- resolved paint selection;
- network hash/synchronization state;
- transient derived pattern scale;
- persistent logical assignment;
- transient stale-recovery revision bookkeeping.

`PaintZ_PaintTarget` updates the shared `ItemBase` state rather than dispatching by category.

The complete finish string remains authoritative on server/persistence paths. Network replication uses validated derived representation; active finish-hash collisions are rejected.

## Persistence

PaintZ requires Community Framework and uses **CF ModStorage** on `ItemBase`.

PaintZ implements its persistence path once through `CF_OnStoreSave` / `CF_OnStoreLoad` and stores the canonical finish ID. It does not append data broadly through native `ItemBase::OnStoreSave()` streams where derived DayZ classes may serialize their own state.

Consequences:

- no weapon-specific persistence hook;
- no magazine-specific persistence hook;
- no attachment-specific persistence hook;
- enabling another ordinary inventory category does not require new persistence code;
- pre-PaintZ/unpainted items load normally without PaintZ state;
- temporarily missing finish definitions do not implicitly erase persisted identity.

If a stored finish cannot resolve, PaintZ preserves the logical ID by default, restores the original/default visual where possible and leaves the item strippable.

See `PERSISTENCE_NOTES.md`.

## Stale finish recovery

`$profile:PaintZ/paintz_stale_finishes.json` is an explicit server-side persistence-repair layer for currently unregistered PaintZ finish IDs.

Resolution is intentionally narrow:

1. registered source ID -> untouched;
2. unregistered source with exact migration -> migrate only to a currently registered destination;
3. unregistered source without a migration and `prune_unknown = true` -> clear PaintZ state;
4. otherwise -> preserve historical state.

Migration takes precedence over pruning. A failed matching migration preserves state and does not fall through to prune. Migration chains and wildcard mappings are rejected.

Recovery is lazy and runs through shared item state. Reloading the JSON does not scan persistence files or enumerate every loaded entity.

A successful migration stores only the destination canonical finish ID. No alias or migration metadata is persisted with the item.

See `STALE_FINISH_RECOVERY.md`.

## Runtime item policy

`$profile:PaintZ/paintz_items.json` governs **new painting/repainting**.

Domains are a positive OR-list. Shared selector families are:

- `type` / `types`;
- `class_pattern` / `class_patterns`;
- `inventory_slot` / `inventory_slots`;
- `inventory_slot_pattern` / `inventory_slot_patterns`.

Within one selector family, singular and plural values form one OR group. Different selector families inside the same domain or rule are AND. Separate domains are OR.

Slot matching uses the target class's declared compatible `inventorySlot` values, not its current attachment state. A loose stock, optic, suppressor or flashlight can therefore match the slots its config declares.

Rules are evaluated top-to-bottom and the last matching rule wins. Type scope may use valid DayZ base/config classes plus legacy `weapon` / `magazine` aliases for compatibility.

Policy affects new painting/repainting only. Existing PaintZ state and stripping remain independent from later exclusion/domain removal.

See `item-policy.md`.

## Hidden-selection safety

Compatibility comes from the actual target model, not a supported-classname registry.

Plausible body selections include names such as `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag`, `magazine`, `housing`, `shell` and `frame`.

Clearly functional surfaces involving glass, lenses, reticles, displays, screens, LEDs, emissive/glow or flame are protected.

Broad category words such as `optic` and `light` are not themselves grounds for rejection because names such as `optic_body` and `flashlight_body` may be legitimate housings.

If several candidate selections remain ambiguous, PaintZ rejects the target rather than guessing.

The same selection-safety rules apply to stale-finish migration.

## Pattern-scale normalization

Pattern scale is derived visual state, not finish identity.

For a patterned finish, PaintZ measures the target collision box and uses the longest local dimension as a physical-size proxy. `$profile:PaintZ/paintz_pattern_scaling.json` maps ascending dimensions to requested scale values.

Scale availability is per finish. Content packs explicitly declare every available scale percentage. PaintZ never invents an undeclared variant path and falls back to the finish's required 100% representation when necessary.

Scale is derived on application/repaint, stale migration and post-load restoration. Reloading scale configuration does not enumerate and mutate all already-loaded items.

## Size-dependent action tuning

Painting and stripping reuse the same physical-size measurement.

`$profile:PaintZ/paintz_action_tuning.json` defines size endpoints, action-time endpoints and paint/stripper consumption rates.

The server owns/reloads this configuration and synchronizes valid settings to clients so continuous-action timing agrees. Server completion recalculates/revalidates required quantity before modifying the target or consuming the applicator.

## Object preservation

Painting and stale migration modify the existing object and do not replace its classname.

Preserve, as applicable:

- object/network identity;
- persistence identity;
- health and quantity;
- ammunition and chamber/FSM state;
- attachments and cargo;
- inventory location and parent/container relationships;
- unrelated third-party state.

The normal visual operation is `SetObjectTexture()` or the verified current equivalent. PaintZ should preserve the target's existing material unless a separately designed finish explicitly requires material behavior.

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
PaintZ -> specific content pack
PaintZ -> PackKit
content pack -> PackKit
official content pack -> another official content pack merely for PZ access
```

## Universality boundary

The JSON-only category-expansion guarantee applies to ordinary `ItemBase`-derived inventory items. Static world objects, buildings, vehicles and unrelated engine hierarchies may require separate lifecycle/persistence design.
