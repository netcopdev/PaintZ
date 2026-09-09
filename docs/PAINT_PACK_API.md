# PaintZ Paint Pack Interoperability Contract v1

This document is the authoritative interoperability contract between the PaintZ runtime, paint-pack mods, and PaintZ PackKit.

The contract defines identity, namespace ownership, collision handling, finish representation, dependency direction, persistence behavior, and responsibility boundaries. Any later incompatible semantic change requires an explicit API-version change or documented migration.

## 1. Responsibility boundary

### PaintZ runtime

PaintZ owns runtime mechanics and the runtime registry:

- painting and stripping actions;
- eligibility/policy and model-safety checks;
- synchronization and persistence;
- resolution of a logical finish ID to its registered surface representation;
- namespace and finish-registration validation;
- collision handling;
- the canonical spray-can base class/model/UV contract;
- Paint Pack API version compatibility.

PaintZ must not depend on any particular external paint pack.

### Paint-pack mods

A normal paint pack owns content, not PaintZ gameplay mechanics. It supplies:

- one pack namespace declaration;
- finish declarations;
- target-surface representations: texture assets for asset-backed finishes and procedural color descriptors for Basic finishes;
- generated can textures;
- thin spawnable can classes inheriting the PaintZ base can;
- optional restrained branding/pack metadata.

A normal paint pack depends on PaintZ at runtime. It should not need per-finish Enforce Script actions or a private copy of PaintZ runtime logic.

### PaintZ PackKit

PackKit is an offline authoring/build tool and reference implementation of this contract. It validates author input, generates conforming paint-pack source/assets/config, enforces namespace/ID rules that can be checked locally, generates standardized can artwork and thin can classes, and generates finish registration data.

PackKit is never a runtime dependency and is not a namespace authority. A pack may be authored manually if it conforms to this contract.

## 2. Public pack namespace

Every non-official paint pack chooses one permanent public namespace prefix.

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

The prefix is the pack's public PaintZ identity. There is no required UUID, secret, long reverse-domain identifier, online registry, or PackKit-generated ownership token in API v1.

Once a pack is released, changing its prefix is a breaking identity change because persisted finish IDs contain the prefix.

## 3. Official PaintZ reservation

All valid 2-3 character prefixes beginning with `PZ` are reserved for official PaintZ content, including `PZ`, `PZA` through `PZZ`, and `PZ0` through `PZ9`.

Third-party namespace declarations beginning with `PZ` are invalid and must be rejected.

The PaintZ Standard Pack initially uses the `PZ` namespace. Additional `PZ?` namespaces remain reserved for future official use and are not automatically assigned.

This reservation is an interoperability rule, not cryptographic publisher authentication. A deliberately modified runtime or hostile executable mod is outside the trust model.

## 4. Finish identity

The canonical runtime and persistence identity of a finish is:

```text
<PREFIX>-<TYPE>-<SUFFIX>
```

Examples:

```text
PZ-B-BLK
PZ-S-FDE
PZ-C-FTN
NCP-B-ODG
NCP-C-FTN
```

There is no separate long canonical finish ID in API v1.

### Type code

The PaintZ one-character type namespace is part of the ID:

- `B` - Basic: plain single RGB color only, with no added wear/noise/scratches/rust or other surface treatment; rendered from a procedural color descriptor rather than a target-surface PAA;
- `S` - Solid: fundamentally one color, but asset-backed so it may contain surface character such as noise, scratches, grime, wear, mottling, rust hints, or similar treatment;
- `C` - camouflage;
- `P` - generic non-camouflage pattern;
- `M` - metallic;
- `R` - rusted / oxidized;
- `W` - weathered;
- `F` - fluorescent;
- `X` - special / custom;
- `T` - transparent / tint.

`Basic` and `Solid` are deliberately distinct. Both may represent one nominal color. `Basic` means color only; `Solid` means a one-color finish with additional surface treatment/detail.

### Finish suffix

The finish suffix remains developer-authored, uppercase alphanumeric, 2-12 characters, with a short descriptive 3-character value preferred.

Changing a released finish ID is a breaking persistence change. Display name, branding, color correction, and texture correction may change without changing the finish ID when the logical finish remains the same.

## 5. Namespace-owner declaration

A loaded paint-pack family has exactly one namespace-owner declaration for its prefix.

A normal single-PBO pack owns and declares its namespace once. A multi-PBO pack family also declares the namespace only once, normally in a core/owner PBO. Additional content PBOs register finishes under that namespace and use normal DayZ add-on dependencies on the owner/core PBO rather than re-declaring ownership.

The exact config representation is defined in `PAINT_PACK_CONFIG_V1.md`, but this one-owner semantic is mandatory.

## 6. Runtime discovery is two-phase

PaintZ must not use first-loaded-wins or last-loaded-wins semantics.

### Phase 1 - discover and validate namespace owners

PaintZ discovers all namespace-owner declarations before accepting finishes.

For each prefix:

- zero valid owners -> the namespace is unresolved and no new finish from it may be registered;
- exactly one valid owner -> the namespace may proceed to finish registration;
- more than one owner declaration -> the namespace is conflicted and disabled as a whole.

PaintZ must log enough information to identify conflicting declarations.

### Phase 2 - validate finish declarations

Only finishes belonging to a valid namespace are considered.

For each finish, PaintZ validates at least:

- finish-ID syntax;
- prefix matches the registered namespace;
- type code is supported by the API version;
- `type` metadata agrees with the ID type code;
- required finish representation is valid enough for registration;
- the complete finish ID is unique in the loaded environment.

A duplicate complete finish ID is ambiguous and is disabled. Other unique finishes in an otherwise valid namespace may remain available. No registration may silently overwrite an already discovered finish definition.

For a `B`/`basic` finish, the required 100% surface must be a procedural color descriptor and the finish must not be pattern-scaled. Asset-backed finish types continue to reference explicitly declared texture assets.

## 7. Trust and authorship model

API v1 does not attempt cryptographic ownership of short prefixes.

Because PaintZ, PackKit, and paint packs are open/modifiable software, a UUID, generated token, manifest secret, or copied long identifier would not prove authorship. Such mechanisms must not be presented as security.

The API protects deterministic behavior and persistence against ordinary collisions and load-order ambiguity. Server administrators remain responsible for which mods they install.

PackKit can validate local format and local uniqueness, but it cannot guarantee that a chosen third-party prefix is unused by every other Workshop/mod package.

## 8. Finish declarations own their representation

A finish declaration must explicitly expose the surface representation PaintZ may use for that finish. PaintZ must not invent undeclared paths or colors from naming conventions.

### Basic finishes

A `B` finish represents one plain RGB color and uses a DayZ procedural texture descriptor at runtime. It does not require a target-surface `.paa`.

Conceptually, PackKit converts author input such as:

```json
{
  "id": "BLK",
  "name": "Basic Black",
  "type": "basic",
  "color": "#262827"
}
```

into an API-v1 surface value equivalent to:

```text
#(argb,8,8,3)color(0.149020,0.156863,0.152941,1.0,CO)
```

The complete finish ID, not the generated procedural string, remains the persistent identity.

### Asset-backed finishes

`S`, `C`, `P`, `M`, `R`, and other asset-backed types expose the actual target-surface texture assets the runtime may select. Patterned/camouflage finishes expose every generated scale variant that may be selected. The registry knows which variants actually exist.

Can artwork is presentation for the spawnable can class and remains separate from target-surface representation. A Basic finish still normally has a generated can texture even though its painted target surface is procedural.

## 9. Spray-can classes

PaintZ provides the non-spawnable canonical spray-can base class and common behavior.

Each paint pack provides one thin spawnable can class per finish. The class identifies the finish it represents and supplies its can presentation/texture while inheriting behavior from PaintZ.

PaintZ runtime logic resolves behavior generically from the finish ID. Normal paint packs should not require generated per-finish action subclasses.

Painted target items are never represented by generated painted weapon/magazine/item subclasses.

## 10. Persistence and missing packs

PaintZ persists the complete logical finish ID, for example:

```text
NCP-B-ODG
NCP-C-FTN
```

PaintZ does not persist a Basic finish's RGB/procedural texture string separately. `B` is still a predefined registered finish exactly like every other finish category.

If a finish is not registered on a later server start because the pack is removed, conflicted, invalid, or temporarily unavailable:

- the persisted finish ID remains historical logical state;
- the item must not be implicitly stripped;
- the underlying item must still load;
- PaintZ should restore the original/default visual when the finish cannot be resolved safely;
- Strip Paint remains available for historical PaintZ state;
- if the same finish ID becomes valid again later, normal restoration can resolve it again.

Unknown/unresolved finish IDs must therefore be tolerated by persistence code.

## 11. Dependency direction

The dependency graph is:

```text
PaintZ PackKit --generates--> Paint Pack --runtime-depends-on--> PaintZ --depends-on--> CF
```

Forbidden dependency assumptions:

```text
PaintZ -> specific paint pack
PaintZ -> PackKit at runtime
Paint pack -> PackKit at runtime
```

The PaintZ Standard Pack is still a paint pack from the runtime architecture's perspective. PaintZ core must not special-case its individual finishes merely because they are official.

## 12. PackKit obligations

PackKit must follow this contract when generating API-v1 packs. At minimum it must:

- require/derive one pack prefix;
- normalize/validate prefix syntax;
- reject `PZ`/`PZ?` namespaces for ordinary third-party generation;
- provide an explicit official/internal path for generating Standard Pack content;
- derive complete finish IDs from prefix + type code + suffix;
- validate local finish-ID uniqueness;
- support `B`/`basic` as plain RGB-only finishes and emit a procedural runtime surface without generating a target-surface PAA;
- keep `S` as the asset-backed one-color category;
- reject appearance-profile/pattern treatment on a Basic finish;
- generate one namespace-owner declaration per pack family;
- generate thin finish can classes and finish-registration data;
- keep runtime mechanics in PaintZ rather than generated pack scripts where the API supports generic behavior.

PackKit must document that it cannot guarantee global uniqueness of a third-party prefix.

## 13. PaintZ Standard Pack obligations

The official Standard Pack:

- uses `PZ` as its initial namespace;
- may use another reserved `PZ?` namespace only through an explicit future project decision;
- depends on PaintZ runtime;
- contains official finish content/assets/can subclasses/registrations, not duplicated runtime mechanics;
- serves as the first-party reference/conformance pack for the current Paint Pack API;
- may provide both Basic and Solid finishes for the same nominal color because their complete IDs differ by type code and their visual semantics differ.

Existing released `PZ-S-*` finishes remain Solid identities. Adding a corresponding `PZ-B-*` finish is additive and must not silently repurpose the existing `S` identity.

## 14. Breaking changes and versioning

The following are identity-breaking after public release:

- changing a pack prefix;
- changing a finish's complete ID;
- reusing an old finish ID for a materially different logical finish.

The following normally are not identity-breaking:

- display-name corrections;
- texture/color corrections that still represent the same intended finish;
- can-label/branding corrections;
- pack display-name/author metadata changes.

Do not add migration/alias machinery speculatively. If a real released breaking change needs migration, design it explicitly against persisted data at that time.

## 15. Source of truth across repositories

This file in the PaintZ repository is authoritative for runtime interoperability semantics.

- `netcopdev/PaintZ` owns the runtime API contract.
- `netcopdev/PaintZ-PackKit` must generate/validate against it.
- `netcopdev/PaintZ-Standard-Pack` must conform to it as the official reference pack.

If repository-local documentation conflicts with this document, fix the local documentation rather than silently creating a divergent contract.
