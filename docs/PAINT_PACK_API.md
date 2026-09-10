# PaintZ Paint Pack Interoperability Contract v1

This document is the authoritative interoperability contract between the PaintZ runtime, paint-pack mods, and PaintZ PackKit.

The contract defines identity, namespace ownership, collision handling, finish representation, dependency direction, persistence behavior, and responsibility boundaries. Any later incompatible semantic change requires an explicit API-version change or documented migration.

## 1. Responsibility boundary

### PaintZ runtime

PaintZ owns runtime mechanics and the runtime registry:

- painting and stripping actions;
- eligibility/policy and model-safety checks;
- synchronization and persistence;
- explicit server-side stale-finish recovery for persisted unregistered IDs;
- resolution of a logical finish ID to its registered runtime surface representation;
- namespace and finish-registration validation;
- collision handling;
- the canonical spray-can base class/model/UV contract;
- Paint Pack API version compatibility;
- generic support for procedural Basic surfaces;
- the official `PZ` namespace identity.

PaintZ core deliberately owns no individual official finishes. It must not depend on any particular external paint pack.

### Paint-pack mods

A paint pack owns content, not PaintZ gameplay mechanics. It supplies:

- finish declarations;
- target-surface representations: texture assets for asset-backed finishes and procedural color descriptors for Basic finishes;
- declared pattern-scale variants where applicable;
- generated can textures;
- thin spawnable can classes inheriting the PaintZ base can;
- optional restrained branding/pack metadata.

A normal third-party standalone pack also declares one namespace owner for its own prefix.

An official PaintZ content pack does **not** declare `PZ`; it contributes unique `PZ-*` finishes to the namespace already owned by PaintZ core.

A paint pack depends on PaintZ at runtime. It must not require per-finish Enforce Script actions or a private copy of PaintZ runtime logic.

### PaintZ PackKit

PackKit is an offline authoring/build tool and reference implementation of this contract. It validates author input, generates conforming source/assets/config, enforces locally checkable namespace/ID rules, generates standardized can artwork/classes, and emits finish-registration data.

PackKit is never a runtime dependency and is not a namespace authority. A pack may be authored manually if it conforms to this contract.

## 2. Public namespace identity

Every non-official standalone paint-pack family chooses one permanent public namespace prefix.

Valid third-party prefixes:

```text
^[A-Z][A-Z0-9]{1,2}$
```

A prefix is therefore 2 or 3 uppercase alphanumeric characters and begins with a letter.

Examples:

```text
NC
NCP
AB7
```

The prefix is a PaintZ identity namespace, not a Workshop package/category identifier. Packaging boundaries must not force an otherwise unchanged finish to change identity.

There is no required UUID, secret, long reverse-domain identifier, online registry, or PackKit-generated ownership token in API v1.

Once a third-party prefix is released, changing it is a breaking identity change because persisted finish IDs contain the prefix.

## 3. Official PaintZ namespace

All valid 2-3 character prefixes beginning with `PZ` are reserved for official PaintZ use:

```text
PZ
PZA ... PZZ
PZ0 ... PZ9
```

Third-party namespace declarations beginning with `PZ` are invalid.

### `PZ` is owned by PaintZ core

PaintZ core permanently declares the active official `PZ` namespace owner:

```text
PZ_PaintZOfficial
```

Content packs must not redeclare it.

`PZ` represents the official PaintZ finish catalogue as a logical identity space. It does **not** represent one particular Workshop mod, PBO, category, or collection.

Independent official PaintZ content packs may contribute unique `PZ-*` finishes while depending directly on PaintZ. Standard, Field, Vanilla, Hunter, Pastel, and future collections are peers; none needs another official pack merely to use `PZ`.

Conceptually:

```text
CF
└── PaintZ  (owns PZ)
    ├── PaintZ Standard Pack   (contributes PZ-*)
    ├── PaintZ Field Pack      (contributes PZ-*)
    ├── PaintZ Vanilla Pack    (contributes PZ-*)
    ├── PaintZ Hunter Pack     (contributes PZ-*)
    └── PaintZ Pastel Pack     (contributes PZ-*)
```

Additional `PZ?` namespaces remain reserved but unassigned. Assigning one requires an explicit future project decision and runtime-owner support. They must not be used merely to encode content categories.

In API v1 the runtime accepts a reserved-prefix owner only when it matches a currently assigned core-owned declaration. At present the only accepted reserved owner is exactly `PZ` / `PZ_PaintZOfficial`. A content pack setting `official = 1` does not grant itself ownership; legacy/hand-authored `PZ` owners and unassigned `PZ?` owners are rejected before ordinary namespace collision resolution.

This is an interoperability gate, not cryptographic publisher authentication.

## 4. Finish identity

The canonical runtime and persistence identity of a finish is:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples:

```text
PZ-B-BLK
PZ-C-FTN
NCP-B-ODG
NCP-S-FDE
NCP-C-FTN
```

There is no separate long canonical finish ID in API v1.

### Type code

The PaintZ one-character type namespace is part of the ID:

- `B` - **Basic**: one plain RGB color only, with no added wear, noise, scratches, grime, rust, edge treatment, or other surface character; represented at runtime by a procedural color descriptor rather than a target-surface PAA;
- `S` - **Solid**: fundamentally one color, but asset-backed and allowed to include deterministic surface treatment/detail;
- `C` - camouflage;
- `P` - generic non-camouflage pattern;
- `M` - metallic;
- `R` - rusted / oxidized;
- `W` - weathered;
- `F` - fluorescent;
- `X` - special / custom;
- `T` - transparent / tint.

`Basic` and `Solid` are deliberately distinct. Both may represent the same nominal base color, and may share the same suffix because their complete IDs differ by type letter. For example, `NCP-B-FDE` and `NCP-S-FDE` are distinct finishes.

Adding a `B` counterpart must not rename, alias, or silently repurpose an existing `S` finish. Released `S` identities remain Solid identities unless an explicit breaking migration is approved. The September 2026 retirement of `PZ-S-RGR/FDE/FGY/UGY/BLK/WHT` in favor of matching `PZ-B-*` IDs is such an explicit documented exception, handled through stale-finish migration when a server chooses to apply it.

### Finish suffix

The finish suffix is developer-authored, uppercase alphanumeric, 2-12 characters, with a short descriptive 3-character value preferred.

Changing a released complete finish ID is a breaking persistence change. Display name, branding, color/texture corrections, or moving the same logical finish between official content packs may occur without changing the finish ID.

## 5. Namespace-owner declarations

### Third-party standalone pack

A normal third-party standalone pack owns and declares its namespace once.

### Third-party multi-PBO family

A multi-PBO third-party family declares the namespace only once, normally in a core/owner PBO. Satellite content PBOs register finishes under that namespace and use ordinary DayZ dependencies on the owner/core PBO.

Example:

```text
NCP_Core.pbo
  declares namespace NCP

NCP_Solids.pbo
  depends on NCP_Core
  registers NCP-S-...

NCP_Camos.pbo
  depends on NCP_Core
  registers NCP-C-...
```

### Official PaintZ content packs

Official `PZ` content packs are different: PaintZ core itself is the namespace owner. Every official content pack references `PZ_PaintZOfficial` and depends directly on PaintZ. No official content pack owns or redeclares `PZ`.

This keeps official distribution packages independent while preserving one stable `PZ` identity namespace.

## 6. Runtime discovery is two-phase

PaintZ must not use first-loaded-wins or last-loaded-wins semantics.

### Phase 1 - discover and validate namespace owners

PaintZ discovers namespace-owner declarations before accepting finishes.

Reserved `PZ*` candidates are validated against currently assigned core-owned owner identities first. Under API v1 only `PZ_PaintZOfficial` owning `PZ` is accepted. Other reserved owner declarations are rejected and do not become competing candidates.

For ordinary non-reserved prefixes:

- zero valid owners -> namespace unresolved;
- exactly one valid owner -> namespace may proceed;
- more than one valid owner -> namespace conflicted and disabled as a whole.

No claimant wins because of mod load order.

### Phase 2 - validate finish declarations

Only finishes belonging to a valid namespace are considered.

For each finish, PaintZ validates at least:

- finish-ID syntax;
- prefix matches the active namespace;
- owner linkage matches the active namespace owner;
- type code is supported by the API version;
- `type` metadata agrees with the ID type code;
- required finish representation is valid enough for registration;
- the complete finish ID is unique in the loaded environment.

A duplicate complete finish ID is ambiguous and disabled. Other unique finishes in an otherwise valid namespace may remain available. No registration may silently overwrite another discovered finish definition.

For a `B`/`basic` finish, the required 100% surface must be a procedural color descriptor and the finish must not be pattern-scaled. Asset-backed finish types continue to reference explicitly declared texture assets.

This collision behavior also applies across independent official packs: several packs may contribute to `PZ`, but they may not define the same complete `PZ-*` ID.

## 7. Trust and authorship model

API v1 does not attempt cryptographic ownership of short third-party prefixes.

Because PaintZ, PackKit, and paint packs are open/modifiable software, a UUID, generated token, manifest secret, or copied long identifier would not prove authorship. Such mechanisms must not be presented as security.

The API protects deterministic behavior and persistence against ordinary collisions and load-order ambiguity. Server administrators remain responsible for which mods they install.

PackKit can validate local format and local uniqueness, but it cannot guarantee that a chosen third-party prefix is globally unused.

## 8. Finish declarations own their runtime representation

A finish declaration must explicitly expose the surface representation PaintZ may use. PaintZ must not invent undeclared paths or colors from naming conventions.

### Basic finishes

A `B` finish represents one predefined plain RGB color and uses a DayZ procedural texture descriptor at runtime. It does not require a target-surface `.paa`.

Conceptually, author input such as:

```json
{
  "id": "BLK",
  "name": "Black",
  "type": "basic",
  "color": "#262827"
}
```

is emitted as an API-v1 S100 surface equivalent to:

```text
#(argb,8,8,3)color(0.149020,0.156863,0.152941,1.0,CO)
```

The complete finish ID, not the procedural descriptor or RGB value separately, remains the persisted identity.

Basic is infrastructure for predefined pack-owned finishes. API v1 does not thereby define anonymous user-generated colors, paint mixing, cumulative tint state, or separate RGB persistence.

### Asset-backed finishes

`S`, `C`, `P`, `M`, `R`, and other asset-backed types expose the actual target-surface texture assets the runtime may select. Patterned/camouflage finishes expose every generated scale variant that may be selected.

Can artwork is presentation for the spawnable can class and remains separate from target-surface representation. A Basic finish still normally has a generated can texture/PAA even though its painted target surface is procedural.

## 9. Spray-can classes

PaintZ provides the non-spawnable canonical spray-can base class and common behavior.

Each content pack provides one thin spawnable can class per finish. The class identifies the finish through `paintzFinish` and supplies presentation/assets while inheriting behavior from PaintZ.

Normal packs must not require generated per-finish action subclasses. Painted target items are never represented by generated painted target subclasses.

Complete finish IDs include the type letter, so `NCP-B-FDE` and `NCP-S-FDE` may coexist. DayZ config classnames are a separate namespace and must still be unique; PackKit detects generated can-class collisions and allows an explicit distinct classname where required.

## 10. Persistence, missing packs and stale recovery

PaintZ persists only the complete logical finish ID, for example:

```text
PZ-B-FDE
PZ-C-FTN
NCP-B-ODG
NCP-C-FTN
```

PaintZ does not persist a Basic finish's RGB/procedural descriptor separately.

If a finish is not registered later because its content pack is removed, invalid, conflicted, or temporarily unavailable, the default behavior is conservative:

- the persisted finish ID remains historical logical state;
- the item is not implicitly stripped;
- the underlying item still loads;
- PaintZ restores the original/default visual where possible;
- Strip Paint remains available;
- if the same finish ID becomes valid again later, it can resolve again.

Therefore moving an official finish between independent official content packs is persistence-safe when its complete `PZ-*` ID is unchanged.

### Explicit server-side stale recovery

A server administrator may explicitly configure persistence repair through `$profile:PaintZ/paintz_stale_finishes.json`.

A stored finish is stale only when its exact ID is not currently registered. Recovery never remaps or prunes a source ID that is currently registered, regardless of whether a matching mapping exists in the JSON.

For a stale ID, PaintZ applies the following order:

1. exact configured migration to a currently registered destination;
2. if no migration exists, optional `prune_unknown` cleanup;
3. otherwise preserve the historical state.

A matching migration takes precedence over pruning. If its destination is unavailable, no safe target selection can be resolved, or the replacement cannot be applied, the stale state is preserved rather than falling through to prune.

Migration mappings are exact IDs only. Wildcards and migration chains are not supported. Successful migration changes the item's authoritative logical finish ID to the destination, which is then persisted normally by CF ModStorage. No alias or migration metadata is stored with the item.

`prune_unknown` is deliberately destructive for otherwise-unmapped stale state. It clears only PaintZ logical state; where a safe selection can be resolved PaintZ restores its configured/original texture first. It does not make policy-excluded but registered finishes stale and it does not operate on arbitrary non-PaintZ textures.

Recovery is lazy. Loading or reloading the stale-recovery JSON does not scan the world or persistence database. A successful config reload affects an item's next relevant state encounter. Invalid reloads retain the previous valid configuration. The bundled default is non-destructive: pruning is off and migrations are empty.

This facility is runtime persistence repair owned by PaintZ core. It does not change content-pack registration semantics, namespace ownership, or load-order collision rules.

The approved September 2026 Solid-to-Basic replacement set is documented by Standard Pack. Servers with historical `PZ-S-RGR/FDE/FGY/UGY/BLK/WHT` state may configure exact mappings to the corresponding currently registered `PZ-B-*` IDs. These mappings are deliberately not automatic.

## 11. Dependency direction

Normal dependency direction is:

```text
PaintZ PackKit --generates--> Paint Pack --runtime-depends-on--> PaintZ --depends-on--> CF
```

Official content packs are peers:

```text
PaintZ Standard Pack --depends-on--> PaintZ
PaintZ Field Pack    --depends-on--> PaintZ
PaintZ Vanilla Pack  --depends-on--> PaintZ
PaintZ Hunter Pack   --depends-on--> PaintZ
PaintZ Pastel Pack   --depends-on--> PaintZ
```

Forbidden assumptions:

```text
PaintZ -> specific paint pack
PaintZ -> PackKit at runtime
Paint pack -> PackKit at runtime
official content pack -> another official content pack merely to access PZ
```

PaintZ core may own the `PZ` namespace declaration, generic Basic support and stale-state recovery without owning any `PZ-*` finish content.

## 12. PackKit obligations

PackKit must follow this contract when generating API-v1 packs. At minimum it must:

- require/derive one pack prefix;
- normalize/validate prefix syntax;
- reject `PZ`/`PZ?` namespaces for ordinary third-party generation;
- provide an explicit official path for official `PZ` content;
- in that official path, reference `PZ_PaintZOfficial` instead of emitting another `CfgPaintZPacks` owner;
- make official content depend on `PaintZ_DynamicPaint`, not on Standard Pack or another official pack;
- reject unassigned reserved official namespaces;
- derive complete finish IDs from prefix + type code + suffix;
- validate local finish-ID and generated classname uniqueness;
- support `B`/`basic` as plain RGB-only finishes and emit a procedural runtime S100 surface without generating a target-surface PNG/PAA;
- keep `S` as the asset-backed one-color category;
- reject `pattern` and `appearance_profile` treatment on a Basic finish;
- continue supporting one owner plus dependent satellites for third-party multi-PBO families;
- generate thin finish can classes and finish-registration data;
- preserve the shared PaintZ can-label identity/layout, including the visible top `PaintZ` logo with its distinct red `Z`;
- keep runtime mechanics and stale-state recovery in PaintZ.

PackKit does not emit or own server stale-recovery mappings. Those mappings are an administrator/runtime concern because they act on persisted server state rather than declaring content-pack identity.

## 13. Official content-pack obligations

Every official pack contributing to `PZ`:

- uses `PZ` IDs unless a genuinely separate official namespace is explicitly assigned later;
- does not declare or own `PZ`;
- references `PZ_PaintZOfficial`;
- depends directly on PaintZ runtime;
- does not depend on another official content pack merely for namespace access;
- owns only its own finish definitions/assets/can subclasses/registrations;
- preserves released finish IDs when reorganizing catalogue contents between official packs.

The PaintZ Standard Pack is the reference/conformance pack, not the owner or mandatory base pack for other official content packs. Its current catalogue is the general-purpose Basic palette. The API still permits separate Basic and Solid identities for the same nominal color where both are intentionally released, because their complete IDs and finish semantics differ.

## 14. Packaging and category rules

A content-pack name or category must not be encoded into the canonical finish prefix merely because content is distributed separately.

For example, Standard, Field, Vanilla, Hunter and Pastel may all contain `PZ-*` finishes. Type letters describe finish semantics; package names describe distribution/collection. These are independent concerns.

Avoid parent/child pack terminology in user-facing documentation. For official content, use **independent optional PaintZ content pack**. The owner/contributor distinction is an internal API detail.

## 15. Breaking changes and versioning

Identity-breaking after public release:

- changing a third-party pack prefix;
- changing a finish's complete ID;
- reusing an old finish ID for a materially different logical finish;
- moving an official finish from `PZ-*` to another prefix rather than retaining its identity.

Normally not identity-breaking:

- display-name corrections;
- texture/color corrections that still represent the same intended finish;
- can-label/branding corrections;
- pack display-name/author metadata changes;
- moving an unchanged `PZ-*` finish between official content packs;
- adding a new `PZ-B-*` Basic counterpart alongside an existing `PZ-S-*` Solid finish.

Do not use stale migration as a routine substitute for stable IDs. When a real released identity change requires repair, an administrator may map the retired unregistered ID directly to the intended currently registered replacement through PaintZ's stale-recovery configuration. Keep such mappings only for the migration window that is operationally required. This is persistence repair, not a permanent registry alias and not permission to reuse an old ID for unrelated meaning.

## 16. Source of truth across repositories

This file in the PaintZ repository is authoritative for runtime interoperability semantics.

- `netcopdev/PaintZ` owns the runtime API contract, generic runtime behavior, official `PZ` namespace identity, persistence and stale-state recovery.
- `netcopdev/PaintZ-PackKit` must generate/validate against it.
- `netcopdev/PaintZ-Standard-Pack` conforms as the independent official general-purpose/reference content pack.
- `netcopdev/PaintZ-Field-Pack` and `netcopdev/PaintZ-Vanilla-Pack` conform as independent official peer content packs.
- Hunter, Pastel, and future official content-pack repositories must follow the same peer dependency model.

If repository-local documentation conflicts with this document, fix the local documentation rather than silently creating a divergent contract.
