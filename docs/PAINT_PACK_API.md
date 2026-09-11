# PaintZ Paint Pack Interoperability Contract v1

This document is the authoritative interoperability contract between the PaintZ runtime, paint-pack mods and PaintZ PackKit.

The contract defines identity, namespace ownership, collision handling, finish representation, dependency direction, persistence behavior and responsibility boundaries. An incompatible semantic change requires an explicit API-version change or documented migration.

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

PaintZ core deliberately owns no individual official finishes and must not depend on any particular content pack.

### Paint-pack mods

A paint pack owns content, not PaintZ gameplay mechanics. It supplies:

- finish declarations;
- target-surface representations;
- declared pattern-scale variants where applicable;
- generated can textures;
- thin spawnable can classes inheriting the PaintZ base can;
- optional restrained branding/pack metadata.

A normal third-party standalone pack also declares one namespace owner for its prefix.

An official PaintZ content pack does not declare `PZ`; it contributes unique `PZ-*` finishes to the namespace owned by PaintZ core.

A paint pack depends on PaintZ at runtime and must not require per-finish Enforce Script actions or a private copy of PaintZ runtime logic.

### PaintZ PackKit

PackKit is offline authoring/build tooling and a reference implementation of this contract. It validates author input and generates conforming pack output.

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

The prefix is PaintZ identity, not a Workshop package/category identifier. Packaging boundaries must not force an otherwise unchanged finish to change identity.

There is no required UUID, secret, reverse-domain identifier, online registry or ownership token in API v1.

Changing a released third-party prefix is an identity-breaking change because persisted finish IDs contain the prefix.

## 3. Official PaintZ namespace

All valid 2-3 character prefixes beginning with `PZ` are reserved for official PaintZ use.

PaintZ core permanently declares the active official owner:

```text
PZ_PaintZOfficial -> PZ
```

Content packs must not redeclare it.

`PZ` represents the official PaintZ finish identity space. It does not represent one particular Workshop mod, PBO or category.

Independent official content packs may contribute unique `PZ-*` finishes while depending directly on PaintZ. Official packs are peers and do not need another official pack merely to use `PZ`.

Additional `PZ?` namespaces remain reserved but unassigned. Assigning one requires an explicit project/API decision and runtime-owner support.

Under API v1, the runtime accepts reserved-prefix ownership only when it exactly matches a currently assigned core-owned declaration. At present the only accepted reserved owner is `PZ` / `PZ_PaintZOfficial`.

This is an interoperability gate, not cryptographic publisher authentication.

## 4. Finish identity

Canonical runtime and persistence identity is:

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

The one-character type code is part of the ID:

- `B` — **Basic**: plain RGB color only; represented by a procedural color descriptor rather than a target-surface PAA;
- `S` — **Solid**: fundamentally one color, asset-backed and allowed to include deterministic surface treatment/detail;
- `C` — camouflage;
- `P` — generic non-camouflage pattern;
- `M` — metallic;
- `R` — rusted / oxidized;
- `W` — weathered;
- `F` — fluorescent;
- `X` — special / custom;
- `T` — transparent / tint.

`Basic` and `Solid` are distinct identities. `NCP-B-FDE` and `NCP-S-FDE`, for example, are different finishes.

Adding a Basic counterpart must not silently rename, alias or repurpose an existing Solid finish. If a released finish identity is deliberately retired or replaced, handle that as an explicit breaking migration through the stale-finish mechanism.

### Finish suffix

The finish suffix is developer-authored, uppercase alphanumeric, 2-12 characters, with a short descriptive 3-character value preferred.

Changing a released complete finish ID is a breaking persistence change. Display name, branding, color/texture corrections or moving the same logical official finish between packages do not require an ID change.

## 5. Namespace-owner declarations

### Third-party standalone pack

A normal third-party standalone pack owns and declares its namespace once.

### Third-party multi-PBO family

A multi-PBO third-party family declares the namespace once, normally in a core/owner PBO. Satellite PBOs register finishes under that namespace and depend on the owner/core PBO.

### Official PaintZ content packs

PaintZ core owns `PZ`. Every official content pack references `PZ_PaintZOfficial` and depends directly on PaintZ. No official content pack owns or redeclares `PZ`.

## 6. Runtime discovery is two-phase

PaintZ must not use first-loaded-wins or last-loaded-wins semantics.

### Phase 1 — discover and validate namespace owners

Reserved `PZ*` candidates are validated against currently assigned core-owned owner identities first.

For ordinary non-reserved prefixes:

- zero valid owners -> namespace unresolved;
- exactly one valid owner -> namespace active;
- more than one valid owner -> namespace conflicted and disabled.

No claimant wins because of mod load order.

### Phase 2 — validate finish declarations

Only finishes belonging to a valid namespace are considered.

For each finish, PaintZ validates at least:

- finish-ID syntax;
- prefix matches the active namespace;
- owner linkage matches the active namespace owner;
- type code is supported by the API version;
- `type` metadata agrees with the ID type code;
- required finish representation is valid;
- the complete finish ID is unique in the loaded environment.

A duplicate complete finish ID is disabled rather than overwritten. Other unique finishes in a valid namespace may remain available.

For a `B`/`basic` finish, the required 100% surface is a procedural color descriptor and the finish is not pattern-scaled.

## 7. Trust and authorship model

API v1 does not attempt cryptographic ownership of short third-party prefixes.

A copied UUID, token or long identifier would not prove authorship in an open/modifiable ecosystem. Such mechanisms must not be presented as security.

The API guarantees deterministic registry behavior and avoids load-order ambiguity. Server administrators remain responsible for which mods they install.

PackKit can validate local format and local uniqueness, but it cannot guarantee that a third-party prefix is globally unused.

## 8. Finish declarations own runtime representation

A finish declaration explicitly exposes the surface representation PaintZ may use. PaintZ must not invent undeclared paths or colors from naming conventions.

### Basic finishes

A `B` finish represents one predefined plain RGB color and uses a DayZ procedural texture descriptor at runtime. It does not require a target-surface `.paa`.

The complete finish ID remains persisted identity; the RGB/procedural descriptor is derived representation.

Basic is infrastructure for predefined pack-owned finishes. API v1 does not define anonymous user-generated colors, paint mixing, cumulative tint state or separate RGB persistence.

### Asset-backed finishes

`S`, `C`, `P`, `M`, `R` and other asset-backed types expose the actual target-surface texture assets the runtime may select. Patterned/camouflage finishes expose every generated scale variant that may be selected.

Can artwork is presentation for the spawnable can class and remains separate from target-surface representation.

## 9. Spray-can classes

PaintZ provides the non-spawnable canonical spray-can base class and common behavior.

Each content pack provides one thin spawnable can class per finish. The class identifies the finish through `paintzFinish` and supplies presentation/assets while inheriting behavior from PaintZ.

Normal packs must not require generated per-finish action subclasses. Painted target items are never represented by generated painted subclasses.

Complete finish IDs include the type letter, so Basic and Solid finishes may share a suffix. DayZ config classnames are a separate namespace and must still be unique.

## 10. Persistence, missing packs and stale recovery

PaintZ persists only the complete logical finish ID.

If a finish is not registered later because its content pack is removed, invalid, conflicted or temporarily unavailable, the default behavior is conservative:

- preserve the persisted finish ID as historical logical state;
- do not implicitly strip the item;
- keep the underlying item loadable;
- restore the original/default visual where possible;
- keep Strip Paint available;
- allow the same finish ID to resolve again if it becomes valid later.

Moving an unchanged official finish between independent official content packs is persistence-safe when its complete `PZ-*` ID is unchanged.

### Explicit server-side stale recovery

A server administrator may configure persistence repair through:

```text
$profile:PaintZ/paintz_stale_finishes.json
```

A stored finish is stale only when its exact ID is not currently registered. Recovery never remaps or prunes a currently registered source ID.

For a stale ID PaintZ applies:

1. exact configured migration to a currently registered destination;
2. if no migration exists, optional `prune_unknown` cleanup;
3. otherwise preserve the historical state.

A matching migration takes precedence over pruning. If migration cannot complete, the stale state is preserved rather than falling through to prune.

Migration mappings are exact IDs only. Wildcards and migration chains are not supported. Successful migration changes the authoritative logical finish ID to the destination, which is then persisted normally by CF ModStorage.

`prune_unknown` is deliberately destructive for otherwise-unmapped stale PaintZ state. It does not make policy-excluded but registered finishes stale and does not operate on arbitrary non-PaintZ textures.

Recovery is lazy. Reloading the JSON does not scan the world or persistence database. Invalid reloads retain the previous valid configuration. The bundled default is non-destructive.

## 11. Dependency direction

Runtime dependency direction is:

```text
Paint Pack -> PaintZ -> CF
```

PackKit is offline tooling:

```text
PaintZ PackKit --generates--> Paint Pack
```

Forbidden runtime assumptions include:

```text
PaintZ -> specific paint pack
PaintZ -> PackKit
Paint pack -> PackKit
official content pack -> another official content pack merely to access PZ
```

## 12. PackKit obligations

PackKit must follow this contract when generating API-v1 packs. At minimum it must:

- require/derive one pack prefix;
- normalize/validate prefix syntax;
- reject reserved `PZ*` namespaces for ordinary third-party generation;
- provide an explicit official path for official `PZ` content;
- reference `PZ_PaintZOfficial` for official output instead of emitting another `PZ` owner;
- make official content depend on `PaintZ_DynamicPaint` rather than on another official pack;
- derive complete finish IDs from prefix + type code + suffix;
- validate local finish-ID and generated classname uniqueness;
- support `B`/`basic` as procedural plain-color finishes;
- keep `S` as the asset-backed one-color category;
- reject pattern/appearance treatment on Basic finishes;
- support owner/satellite generation for third-party multi-PBO families;
- generate thin can classes and finish-registration data;
- keep runtime mechanics and stale-state recovery in PaintZ.

PackKit does not emit or own server stale-recovery mappings.

## 13. Official content-pack obligations

Every official pack contributing to `PZ`:

- uses `PZ` IDs unless a separate official namespace is explicitly assigned later;
- does not declare or own `PZ`;
- references `PZ_PaintZOfficial`;
- depends directly on PaintZ runtime;
- does not depend on another official content pack merely for namespace access;
- owns only its own finish definitions/assets/can subclasses/registrations;
- preserves released finish IDs when reorganizing catalogue contents between official packs.

Official packs are independent peers. No official pack is the runtime owner or mandatory base for another.

## 14. Packaging and category rules

A content-pack name or category must not be encoded into canonical finish identity merely because content is distributed separately.

Type letters describe finish semantics; package names describe distribution. These are independent concerns.

## 15. Breaking changes and versioning

Identity-breaking after release includes:

- changing a third-party pack prefix;
- changing a finish's complete ID;
- reusing an old finish ID for a materially different logical finish;
- moving an official finish from `PZ-*` to another prefix rather than retaining its identity.

Normally not identity-breaking:

- display-name corrections;
- texture/color corrections that retain the same intended finish;
- can-label/branding corrections;
- pack display-name/author metadata changes;
- moving an unchanged `PZ-*` finish between official content packs;
- adding a new Basic counterpart alongside an existing Solid finish.

Do not use stale migration as a routine substitute for stable IDs. When a real released identity change requires repair, an administrator may map the retired unregistered ID directly to the intended currently registered replacement through PaintZ's stale-recovery configuration.

## 16. Source of truth across repositories

This file in the PaintZ repository is authoritative for runtime interoperability semantics.

- `netcopdev/PaintZ` owns the runtime API contract, generic runtime behavior, official `PZ` namespace identity, persistence and stale-state recovery.
- `netcopdev/PaintZ-PackKit` must generate/validate against it.
- official and third-party content-pack repositories must conform to it.

If repository-local documentation conflicts with this document, fix the local documentation rather than creating a divergent contract.
