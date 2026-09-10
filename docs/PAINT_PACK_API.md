# PaintZ Paint Pack Interoperability Contract v1

This document is the authoritative interoperability contract between the PaintZ runtime, paint-pack mods, and PaintZ PackKit.

The contract defines identity, namespace ownership, collision handling, dependency direction, persistence behavior, and responsibility boundaries. Exact config-class/property names may evolve during the first implementation, but they must preserve these semantics. Any later incompatible semantic change requires an explicit API-version change or documented migration.

## 1. Responsibility boundary

### PaintZ runtime

PaintZ owns runtime mechanics and the runtime registry:

- painting and stripping actions;
- eligibility/policy and model-safety checks;
- synchronization and persistence;
- resolution of a logical finish ID to its registered runtime surface;
- namespace and finish-registration validation;
- collision handling;
- the canonical spray-can base class/model/UV contract;
- Paint Pack API version compatibility;
- the official `PZ` namespace identity.

PaintZ core deliberately owns no individual official finishes. It must not depend on any particular external paint pack.

### Paint-pack mods

A paint pack owns content, not PaintZ gameplay mechanics. It supplies:

- finish declarations;
- target surface textures/procedural surface declarations and declared pattern-scale variants as applicable;
- generated can textures;
- thin spawnable can classes inheriting the PaintZ base can;
- optional restrained branding/pack metadata.

A normal third-party standalone pack also declares one namespace owner for its own prefix.

An official PaintZ content pack does **not** declare `PZ`; it contributes `PZ-*` finishes to the `PZ` namespace already owned by PaintZ core.

A paint pack depends on PaintZ at runtime. It should not need per-finish Enforce Script actions or a private copy of PaintZ runtime logic.

### PaintZ PackKit

PackKit is an offline authoring/build tool and reference implementation of this contract. It validates author input, generates conforming source/assets, enforces locally checkable namespace/ID rules, generates standardized can artwork/classes, and emits finish-registration/config data.

PackKit is never a runtime dependency and is not a namespace authority. A pack may be authored manually if it conforms to this contract.

## 2. Public namespace identity

Every non-official standalone paint-pack family chooses one permanent public namespace prefix.

Valid third-party prefixes:

```text
^[A-Z][A-Z0-9]{1,2}$
```

Therefore a prefix is 2 or 3 uppercase alphanumeric characters and begins with a letter.

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

Any number of independent official PaintZ content packs may contribute unique `PZ-*` finishes while depending directly on PaintZ. Examples include Standard, Pastel, Military, Hunting, Weathered, or other collections. These packages are peers; none needs another official content pack merely to use `PZ`.

Conceptually:

```text
CF
└── PaintZ  (owns PZ)
    ├── PaintZ Standard Pack   (contributes PZ-*)
    ├── PaintZ Pastel Pack     (contributes PZ-*)
    ├── PaintZ Military Pack   (contributes PZ-*)
    └── PaintZ Hunting Pack    (contributes PZ-*)
```

Each content pack has only its normal direct runtime dependency on PaintZ; content packs do not depend on one another.

Additional `PZ?` namespaces remain reserved but unassigned. Assigning one requires an explicit future project decision and runtime-owner support. They must not be used merely to encode content categories.

In API v1 the runtime accepts a reserved-prefix owner only when it matches a currently assigned core-owned declaration. At present the only accepted reserved owner is exactly `PZ` / `PZ_PaintZOfficial`. A content pack setting `official = 1` does not grant itself ownership, and legacy/hand-authored `PZ` owners or `PZ?` owners are rejected before namespace collision resolution.

This is an interoperability gate, not cryptographic publisher authentication. A deliberately modified runtime or hostile executable mod is outside the trust model.

## 4. Finish identity

The canonical runtime and persistence identity of a finish is:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples:

```text
PZ-C-FTN
PZ-S-FDE
NCP-C-FTN
NCP-S-FDE
```

There is no separate long canonical finish ID in API v1.

### Type code

The PaintZ one-character type namespace is part of the ID. The currently integrated API-v1 set is:

- `S` - solid
- `C` - camouflage
- `P` - generic pattern
- `M` - metallic
- `R` - rusted / oxidized
- `W` - weathered
- `F` - fluorescent
- `X` - special / custom
- `T` - transparent / tint

Any separately developed extension to this list must update runtime, PackKit, reference packs, tests, and documentation together before integration.

### Finish suffix

The finish suffix is developer-authored, uppercase alphanumeric, 2-12 characters, with a short descriptive 3-character value preferred.

Changing a released finish ID is a breaking persistence change. Display name, branding, color/texture corrections, or moving the same logical finish between official content packs may occur without changing the finish ID.

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

Official `PZ` content packs are different: PaintZ core itself is the namespace owner. Every official content pack references the core owner and depends only on PaintZ. No official content pack owns or redeclares `PZ`.

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
- required finish metadata/runtime surfaces are valid enough for registration;
- the complete finish ID is unique in the loaded environment.

A duplicate complete finish ID is ambiguous and disabled rather than using first/last-loaded-wins behavior. Other unique finishes in an otherwise valid namespace may remain available.

This applies across independent official packs: multiple official packs may contribute to `PZ`, but they may not define the same complete `PZ-*` finish ID.

No registration may silently overwrite another discovered finish definition.

## 7. Trust and authorship model

API v1 does not attempt cryptographic ownership of short third-party prefixes.

Because PaintZ, PackKit, and paint packs are open/modifiable software, a UUID, generated token, manifest secret, or copied long identifier would not prove authorship. Such mechanisms must not be presented as security.

The API protects deterministic behavior and persistence against ordinary collisions and load-order ambiguity. Server administrators remain responsible for which mods they install.

PackKit can validate local format and local uniqueness, but it cannot guarantee that a chosen third-party prefix is unused by every other Workshop/mod package.

## 8. Finish declarations own their runtime surfaces

A finish declaration must explicitly expose every runtime surface PaintZ may use for that finish. PaintZ must not invent undeclared texture paths from naming conventions and assume they exist.

For an asset-backed solid finish this normally includes its base target-surface texture. For a patterned/camouflage finish this includes every generated scale variant PaintZ may select. Other supported finish representations may expose a validated procedural surface descriptor when the API explicitly defines it.

Can artwork remains separate from target-surface representation.

## 9. Spray-can classes

PaintZ provides the non-spawnable canonical spray-can base class and common behavior.

Each content pack provides one thin spawnable can class per finish. The class identifies the finish through `paintzFinish` and supplies presentation/assets while inheriting behavior from PaintZ.

Normal packs should not require generated per-finish action subclasses. Painted target items are never represented by generated painted target subclasses.

## 10. Persistence and missing packs

PaintZ persists the complete logical finish ID, for example:

```text
PZ-C-FTN
NCP-C-FTN
```

If that finish is not registered later because its content pack is removed, invalid, conflicted, or temporarily unavailable:

- the persisted finish ID remains historical logical state;
- the item is not implicitly stripped;
- the underlying item still loads;
- PaintZ restores the original/default visual where possible;
- Strip Paint remains available;
- if the same finish ID becomes valid again later, it can resolve again.

Therefore moving an official finish between independent official content packs is persistence-safe when its complete `PZ-*` ID is unchanged. A temporary deployment gap may make the visual unresolved, but it does not erase the logical assignment.

## 11. Dependency direction

Normal dependency direction is:

```text
PaintZ PackKit --generates--> Paint Pack --runtime-depends-on--> PaintZ --depends-on--> CF
```

Official content packs are peers:

```text
PaintZ Standard Pack --depends-on--> PaintZ
PaintZ Pastel Pack   --depends-on--> PaintZ
PaintZ Military Pack --depends-on--> PaintZ
PaintZ Hunting Pack  --depends-on--> PaintZ
```

Forbidden assumptions:

```text
PaintZ -> specific paint pack
PaintZ -> PackKit at runtime
Paint pack -> PackKit at runtime
official content pack -> another official content pack merely to access PZ
```

PaintZ core may own the `PZ` namespace declaration without owning any `PZ-*` finish content. Namespace identity infrastructure is not finish-content ownership.

## 12. PackKit obligations

PackKit must follow this contract when generating API-v1 packs.

At minimum it must:

- require/derive one pack prefix;
- normalize/validate prefix syntax;
- reject `PZ`/`PZ?` namespaces for ordinary third-party generation;
- provide an explicit official path for official `PZ` content;
- in that official path, reference `PZ_PaintZOfficial` instead of emitting another `CfgPaintZPacks` owner;
- make official content depend on `PaintZ_DynamicPaint`, not on Standard Pack or another official pack;
- reject unassigned reserved official namespaces;
- derive complete finish IDs from prefix + type code + suffix;
- validate local finish-ID uniqueness;
- not invent UUID/security ownership as a requirement;
- continue supporting one owner plus dependent satellites for third-party multi-PBO families;
- generate thin finish can classes and finish-registration data;
- keep runtime mechanics in PaintZ.

## 13. Official content-pack obligations

Every official pack contributing to `PZ`:

- uses `PZ` IDs unless a genuinely separate official namespace is explicitly assigned later;
- does not declare or own `PZ`;
- references `PZ_PaintZOfficial`;
- depends directly on PaintZ runtime;
- does not depend on Standard Pack or another official content pack merely for namespace access;
- owns only its own finish definitions/assets/can subclasses/registrations;
- preserves released finish IDs when reorganizing catalogue contents between official packs.

The PaintZ Standard Pack is the reference/conformance pack, not the owner or mandatory base pack for other official content packs.

## 14. Packaging and category rules

A content-pack name or category must not be encoded into the canonical finish prefix merely because content is distributed separately.

For example, Standard, Pastel, Military and Hunting may all contain `PZ-*` finishes. Type letters describe finish semantics; package names describe distribution/collection. These are independent concerns.

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
- moving an unchanged `PZ-*` finish between official content packs.

Do not add migration/alias machinery speculatively. If a real released breaking change needs migration, design it explicitly against persisted data at that time.

## 16. Source of truth across repositories

This file in the PaintZ repository is authoritative for runtime interoperability semantics.

- `netcopdev/PaintZ` owns the runtime API contract and official `PZ` namespace identity.
- `netcopdev/PaintZ-PackKit` must generate/validate against it.
- `netcopdev/PaintZ-Standard-Pack` must conform to it as an independent official reference content pack.
- future official content-pack repositories must follow the same peer dependency model.

If repository-local documentation conflicts with this document, fix the local documentation rather than silently creating a divergent contract.
