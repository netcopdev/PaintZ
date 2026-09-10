# Paint Pack API v1 - DayZ config representation

This document defines the concrete DayZ config representation consumed by the current PaintZ API-v1 runtime registry. The higher-level identity/collision/persistence rules remain authoritative in `PAINT_PACK_API.md`.

## Discovery roots

PaintZ exposes two config roots:

```cpp
class CfgPaintZPacks {};
class CfgPaintZFinishes {};
```

PaintZ itself contributes the canonical official `PZ` namespace owner under `CfgPaintZPacks`. Third-party owner packs add their own child classes there. All content packs add finish child classes under `CfgPaintZFinishes`.

Config child class names are **not** persisted PaintZ identities. They are DayZ config keys only. Authors/PackKit should nevertheless generate distinctive finish/config class names because DayZ merges config trees before PaintZ enumerates them.

## Runtime dependency

Every normal paint content pack depends on PaintZ through ordinary DayZ add-on dependency ordering. Its `CfgPatches` must require PaintZ's runtime patch before inheriting the can base or registering content:

```cpp
class CfgPatches
{
    class NCP_MilitaryPaints
    {
        units[] = {"NCP_SprayCan_FTN"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "PaintZ_DynamicPaint"
        };
    };
};
```

PaintZ core must never require a paint pack in the opposite direction.

Independent official `PZ` content packs also require only `PaintZ_DynamicPaint`; they do not require PaintZ Standard Pack or another official content pack.

## PaintZ core's official namespace owner

PaintZ core declares the official namespace once:

```cpp
class CfgPaintZPacks
{
    class PZ_PaintZOfficial
    {
        apiVersion = 1;
        prefix = "PZ";
        displayName = "PaintZ Official";
        official = 1;
    };
};
```

`PZ_PaintZOfficial` is the canonical API-v1 owner linkage key for official `PZ-*` finishes. It is not a persisted finish identity and it does not mean PaintZ core owns any finish assets.

Official content packs **must not** emit another `CfgPaintZPacks` declaration for `PZ`. A second owner would correctly create a namespace conflict.

The remaining `PZ?` prefixes are reserved but unassigned. They must not be used until a future explicit project decision provides a canonical owner for them.

## Third-party namespace-owner declaration

A normal third-party standalone pack declares its namespace exactly once:

```cpp
class CfgPaintZPacks
{
    class NCP_NetcopMilitaryPaints
    {
        apiVersion = 1;
        prefix = "NCP";
        displayName = "Netcop Military Paints";
    };
};
```

Required fields:

- `apiVersion` - must currently be `1`;
- `prefix` - the permanent 2-3 character public namespace.

Optional metadata:

- `displayName` - human-readable pack/family name.

The child class name, here `NCP_NetcopMilitaryPaints`, is the runtime owner key referenced by that namespace's finish declarations. It is not user-facing and is not persisted.

A multi-PBO third-party family declares the namespace in only one owner/core PBO. Satellite PBOs depend on that PBO through normal `CfgPatches.requiredAddons[]` and reference the same owner class without another owner declaration.

The `official = 1` marker is reserved for PaintZ-owned official namespace infrastructure. Ordinary third-party packs must not set it and must not claim `PZ*`.

## Finish declaration

Each finish is a child of `CfgPaintZFinishes` and explicitly names its namespace owner.

Third-party example:

```cpp
class CfgPaintZFinishes
{
    class NCP_C_FTN
    {
        id = "NCP-C-FTN";
        owner = "NCP_NetcopMilitaryPaints";
        displayName = "Flecktarn";
        type = "camo";
        isPattern = 1;

        class Surfaces
        {
            class S050
            {
                scalePercent = 50;
                texture = "NCP_Military\\data\\surfaces\\ncp_c_ftn_s050_co.paa";
            };
            class S100
            {
                scalePercent = 100;
                texture = "NCP_Military\\data\\surfaces\\ncp_c_ftn_co.paa";
            };
            class S150
            {
                scalePercent = 150;
                texture = "NCP_Military\\data\\surfaces\\ncp_c_ftn_s150_co.paa";
            };
        };
    };
};
```

Independent official pack example:

```cpp
class CfgPaintZFinishes
{
    class PZ_Military_C_FTN
    {
        id = "PZ-C-FTN";
        owner = "PZ_PaintZOfficial";
        displayName = "Flecktarn";
        type = "camo";
        isPattern = 1;

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "PaintZ_Military_Pack\\data\\surfaces\\pz_c_ftn_co.paa";
            };
        };
    };
};
```

The official pack's own config child classname (`PZ_Military_C_FTN` above) should be distinctive to that content pack. Only the `owner` value is shared. This lets multiple independent official packs contribute to `PZ` without reusing each other's local config-class namespace.

Required fields:

- `id` - complete canonical finish ID `<PREFIX>-<TYPE>-<SUFFIX>`;
- `owner` - exact `CfgPaintZPacks` child class that owns the ID prefix;
- `displayName` - human-readable finish name;
- `type` - type name corresponding to the ID type letter;
- `isPattern` - `1` for a scale-selectable pattern/camouflage finish, otherwise `0`/omitted;
- `Surfaces` - one or more explicitly declared runtime surface representations.

PaintZ accepts a finish only when the prefix extracted from `id` resolves to one active namespace owner and the finish's `owner` field matches that owner's config child class exactly.

For all official `PZ-*` finishes under API v1:

```text
owner = "PZ_PaintZOfficial"
```

Every finish must declare a `scalePercent = 100` surface. A non-pattern finish currently declares only the 100% surface. A pattern may declare any subset of valid whole percentages from 1 through 1000; PaintZ selects only variants actually declared by that finish and falls back to 100% when a configured scale is unavailable.

The runtime does not construct paint-pack texture paths from the finish ID.

### Type names

The current integrated mapping is:

```text
S -> solid
C -> camo or camouflage
P -> pattern
M -> metallic
R -> rusted or oxidized
W -> weathered
F -> fluorescent
X -> special or custom
T -> transparent or tint
```

The `type` string and ID type letter must agree. PackKit should emit one canonical spelling per type even though the runtime accepts documented synonyms for interoperability.

## Spray-can class

PaintZ owns the common non-spawnable `PaintZ_SprayCanBase`. A content pack provides one thin spawnable subclass per finish:

```cpp
class CfgVehicles
{
    class PaintZ_SprayCanBase;

    class NCP_SprayCan_FTN : PaintZ_SprayCanBase
    {
        scope = 2;
        displayName = "PaintZ - Flecktarn";
        paintzFinish = "NCP-C-FTN";
        hiddenSelectionsTextures[] = {"NCP_Military\\data\\cans\\ncp_c_ftn_co.paa"};
    };
};
```

`paintzFinish` is the API-v1 property and should be used by all API-v1 packs.

The can does not define its own action class. `PaintZ_SprayCanBase` attaches the single generic PaintZ paint action, which resolves the finish from `paintzFinish` and the runtime registry.

Moving a released official finish between official content packs should preserve its existing can classname where practical as well as its canonical `PZ-*` finish ID. Classname compatibility and finish identity are separate concerns.

## Official pack independence

An official content pack generated for `PZ` has this shape:

```text
CfgPatches
  requiredAddons[] = { "PaintZ_DynamicPaint" }

(no CfgPaintZPacks owner declaration)

CfgPaintZFinishes
  each finish owner = "PZ_PaintZOfficial"

CfgVehicles
  thin spray-can subclasses
```

Therefore these packages are valid peers:

```text
PaintZ Standard Pack -> PaintZ
PaintZ Pastel Pack   -> PaintZ
PaintZ Military Pack -> PaintZ
PaintZ Hunting Pack  -> PaintZ
```

There is no required dependency from Pastel/Military/Hunting to Standard, nor between any other official content packs.

## Third-party satellite packs

The existing satellite mechanism remains useful for a third-party multi-PBO family whose namespace owner is itself an external pack.

A satellite:

- does not emit `CfgPaintZPacks`;
- references its family's existing owner class;
- requires both `PaintZ_DynamicPaint` and the owner's `CfgPatches` classname;
- owns its own textures and thin spray-can classes.

Do not use this parent/satellite dependency pattern for normal official `PZ` collections. PaintZ itself is already the stable owner dependency.

## Runtime validation order

PaintZ performs two logical phases:

1. enumerate and validate all namespace-owner declarations;
2. enumerate finish declarations only after namespace status is known.

For namespace owners:

- one valid declaration for a prefix -> namespace is active;
- multiple valid declarations for the same prefix -> entire namespace is conflicted and disabled;
- invalid or unsupported declarations do not acquire a namespace.

For finishes:

- the ID must be valid and its prefix must belong to an active namespace;
- `owner` must match that namespace's active owner config class;
- metadata/type and all surface declarations must validate;
- duplicate complete finish IDs are disabled rather than overwritten;
- PaintZ also rejects an internal network-hash collision rather than allowing ambiguous client resolution.

No first-loaded-wins or last-loaded-wins rule is used.

## Config-key collision limitation

The Paint Pack API does not claim cryptographic authorship or hostile-mod isolation. DayZ combines config trees before PaintZ enumerates them. If two add-ons deliberately define the exact same config child class, the engine may merge/override that config entry before PaintZ can distinguish the sources.

Third-party owner class names and every pack's local finish config class names should therefore be distinctive. `owner` must not be presented as a security token.

## Core/content separation

PaintZ core defines:

- the registry;
- the official `PZ` namespace owner;
- generic can base;
- generic paint/strip actions;
- persistence/networking;
- model inspection;
- policy;
- pattern-scale behavior.

PaintZ core does **not** contain an official finish catalogue, finish-specific spray cans, or official finish assets.

A server wanting official content loads PaintZ plus whichever independent official content packs it wants. Example:

```text
CF -> PaintZ -> { Standard, Pastel, Military, Hunting, ... }
```

The braces describe optional peer packages, not a nested runtime dependency chain.

## Conformance

`tools/sandbox/paint-pack-api-fixture` should exercise the actual merged DayZ config tree, including namespace conflicts, owner mismatch, reserved-prefix rejection, duplicate finish IDs, generic painting, unresolved historical-state stripping, and at least one finish registered against the core-owned official `PZ` namespace.
