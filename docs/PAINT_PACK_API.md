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

PackKit is an offline authoring/build tool and reference implementation of this contract. It:

- validates author input;
- generates conforming paint-pack source/assets;
- enforces namespace/ID rules that can be checked locally;
- generates standardized can artwork and thin can classes;
- generates finish-registration/config data.

PackKit is never a runtime dependency and is not a namespace authority. A pack may be authored manually if it conforms to this contract.

## 2. Public namespace identity

Every non-official standalone paint-pack family chooses one permanent public namespace prefix.

Valid third-party prefixes:

```text
^[A-Z][A-Z0-9]{1,2}$
```

Therefore a prefix is 2 or 3 uppercase alphanumeric characters and begins with a letter. Lower/mixed-case author input may be normalized to uppercase by authoring tools before release.

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

PaintZ core permanently declares the active official `PZ` namespace owner. Content packs must not redeclare it.

`PZ` represents the official PaintZ finish catalogue as a logical identity space. It does **not** represent one particular Workshop mod, PBO, category, or collection.

Any number of independent official PaintZ content packs may contribute unique `PZ-*` finishes while depending directly on PaintZ. Examples of possible distribution packages include Standard, Pastel, Military, Hunting, Weathered, or other collections. These packages are peers; none needs another official content pack merely to use `PZ`.

Conceptually:

```text
CF
└── PaintZ  (owns PZ)
    ├── PaintZ Standard Pack   (contributes PZ-*)
    ├── PaintZ Pastel Pack     (contributes PZ-*)
    ├── PaintZ Military Pack   (contributes PZ-*)
    └── PaintZ Hunting Pack    (contributes PZ-*)
```

The dependency graph above is conceptual. Each content pack has only its normal direct runtime dependency on PaintZ; content packs do not depend on one another.

Additional `PZ?` namespaces remain reserved but unassigned. They must not be used merely to encode content categories. Assigning one requires an explicit future project decision and runtime-owner support.

This reservation is an interoperability rule, not cryptographic publisher authentication. A deliberately modified runtime or hostile executable mod is outside the trust model.

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

The PaintZ one-character type namespace is part of the ID. The currently released API-v1 set is:

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

The finish suffix remains developer-authored, uppercase alphanumeric, 2-12 characters, with a short descriptive 3-character value preferred.

Changing a released finish ID is a breaking persistence change. Display name, branding, color correction, texture correction, or moving the same logical finish between official content packs may occur without changing the finish ID.

## 5. Namespace-owner declarations

### Third-party standalone pack

A normal third-party standalone pack owns and declares its namespace once.

### Third-party multi-PBO family

A multi-PBO third-party family still declares the namespace only once, normally in a core/owner PBO. Additional satellite content PBOs register finishes under that namespace and use normal DayZ add-on dependencies on the owner/core PBO rather than re-declaring ownership.

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

PaintZ discovers all namespace-owner declarations before accepting finishes.

For each prefix:

- zero valid owners -> the namespace is unresolved and no new finish from it may be registered;
- exactly one valid owner -> the namespace may proceed to finish registration;
- more than one owner declaration -> the namespace is conflicted.

A conflicted namespace is disabled as a whole. No claimant wins because of mod load order.

Because PaintZ core owns `PZ`, an official content pack that incorrectly redeclares `PZ` creates a duplicate-owner conflict and is invalid. PackKit's official generation path must therefore never emit a second `PZ` owner.

### Phase 2 - validate finish declarations

Only finishes belonging to a valid namespace are considered.

For each finish, PaintZ validates at least:

- finish-ID syntax;
- prefix matches the registered namespace;
- owner linkage matches the active namespace owner;
- type code is supported by the API version;
- required finish metadata/runtime surfaces are valid enough for registration;
- the complete finish ID is unique in the loaded environment.

A duplicate complete finish ID is ambiguous and must not use first/last-loaded-wins behavior. The duplicated finish ID is disabled. Other unique finishes in an otherwise valid namespace may remain available.

This rule applies across independent official packs as well: two official packs may both contribute to `PZ`, but they may not define the same complete `PZ-*` finish ID.

No registration may silently overwrite an already discovered finish definition.

## 7. Trust and authorship model

API v1 does not attempt cryptographic ownership of short prefixes.

Because PaintZ, PackKit, and paint packs are open/modifiable software, a UUID, generated token, manifest secret, or copied long identifier would not prove authorship. Such mechanisms must not be presented as security.

The API protects deterministic behavior and persistence against ordinary collisions and load-order ambiguity. Server administrators remain responsible for which mods they install.

PackKit can validate local format and local uniqueness, but it cannot guarantee that a chosen third-party prefix is unused by every other Workshop/mod package.

## 8. Finish declarations own their runtime surfaces

A finish declaration must explicitly expose every runtime surface PaintZ may use for that finish.

PaintZ must not invent undeclared texture paths from naming conventions and assume they exist.

For an asset-backed solid finish this normally includes its base target-surface texture. For a patterned/camouflage finish this includes every generated scale variant that PaintZ may select. Other supported finish representations may expose a validated procedural surface descriptor instead of a texture asset when the API explicitly defines that representation.

Can artwork is presentation for the spawnable can class and remains separate from target-surface representation.

## 9. Spray-can classes

PaintZ provides the non-spawnable canonical spray-can base class and common behavior.

Each paint pack provides one thin spawnable can class per finish. The class identifies the finish it represents and supplies its can presentation/texture while inheriting behavior from PaintZ.

PaintZ runtime logic resolves behavior generically from the finish ID. Normal paint packs must not require generated per-finish action subclasses.

Painted target items are never represented by generated painted weapon/magazine/item subclasses.

## 10. Persistence and missing packs

PaintZ persists the complete logical finish ID, for example:

```text
PZ-C-FTN
NCP-C-FTN
```

If that finish is not registered on a later server start because its content pack is removed, invalid, conflicted, or temporarily unavailable:

- the persisted finish ID remains historical logical state;
- the item must not be implicitly stripped;
- the underlying item must still load;
- PaintZ should restore the original/default visual when the finish cannot be resolved safely;
- Strip Paint remains available for historical PaintZ state;
- if the same finish ID becomes valid again later, normal restoration can resolve it again.

Therefore moving an official finish between independent official content packs is persistence-safe when its complete `PZ-*` ID is unchanged. A temporary deployment gap may make the visual unresolved, but it does not erase the logical assignment.

Unknown/unresolved finish IDs must be tolerated by persistence code.

## 11. Dependency direction

The normal dependency graph is:

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
- in that official `PZ` path, reference PaintZ core's canonical owner instead of emitting another `CfgPaintZPacks` owner;
- make official content depend on `PaintZ_DynamicPaint`, not on Standard Pack or another official pack;
- derive complete finish IDs from prefix + type code + suffix;
- validate local finish-ID uniqueness;
- not invent UUID/security ownership as a requirement;
- continue supporting one namespace-owner declaration plus dependent satellite PBOs for third-party multi-PBO families;
- generate thin finish can classes and finish-registration data;
- keep runtime mechanics in PaintZ rather than generated pack scripts where the API supports generic behavior.

PackKit must document that it cannot guarantee global uniqueness of a third-party prefix.

## 13. Official content-pack obligations

Every official pack contributing to `PZ`:

- uses `PZ` IDs for official finishes unless a genuinely separate official namespace is explicitly assigned later;
- does not declare or own the `PZ` namespace;
- references PaintZ core's canonical `PZ` owner;
- depends directly on PaintZ runtime;
- does not depend on Standard Pack or another official content pack merely for namespace access;
- owns only its own finish definitions/assets/can subclasses/registrations;
- preserves released finish IDs when reorganizing catalogue contents between official packs.

The PaintZ Standard Pack is the reference/conformance pack, not the owner or mandatory base pack for other official content packs.

## 14. Packaging and category rules

A content-pack name or category must not be encoded into the canonical finish prefix merely because content is distributed separately.

For example, Standard, Pastel, Military and Hunting may all contain `PZ-*` finishes. Type letters describe finish semantics; package names describe distribution/collection. These are independent concerns.

Avoid terms such as parent pack or child pack in user-facing documentation. For official content, use **independent optional PaintZ content pack**. The owner/contributor distinction is an internal API detail.

## 15. Breaking changes and versioning

The following are identity-breaking after public release:

- changing a third-party pack prefix;
- changing a finish's complete ID;
- reusing an old finish ID for a materially different logical finish;
- moving an official finish from `PZ-*` to another prefix rather than retaining its identity.

The following normally are not identity-breaking:

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
