# Paint Pack API v1 - DayZ config representation

This document defines the concrete DayZ config representation consumed by the current PaintZ API-v1 runtime registry. The higher-level identity/collision/persistence rules remain authoritative in `PAINT_PACK_API.md`.

## Discovery roots

PaintZ exposes two config roots:

```cpp
class CfgPaintZPacks {};
class CfgPaintZFinishes {};
```

Paint packs add child classes to these roots. PaintZ enumerates both trees at runtime.

Config child class names are **not** persisted PaintZ identities. They are DayZ config keys only. Authors/PackKit should nevertheless generate distinctive class names because two add-ons defining the same config child class can be merged by the engine before PaintZ sees the tree.

## Runtime dependency

A normal pack depends on PaintZ through ordinary DayZ add-on dependency ordering. Its `CfgPatches` should require PaintZ's runtime patch before inheriting the can base or registering content:

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

## Namespace-owner declaration

A normal pack declares its namespace exactly once:

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

- `displayName` - human-readable pack name.

The child class name, here `NCP_NetcopMilitaryPaints`, is the runtime owner key referenced by that pack's finish declarations. It is not user-facing and is not persisted.

For an official reserved `PZ*` namespace, the declaration additionally uses:

```cpp
official = 1;
```

This flag is an interoperability gate, not cryptographic authentication. Ordinary third-party packs must not set it and must not claim `PZ*`.

The official PaintZ Standard Pack owns `PZ` through this same mechanism.

A multi-PBO pack family declares the namespace in only one owner/core PBO. Satellite PBOs depend on that PBO through normal `CfgPatches.requiredAddons[]` and reference that same owner class from their finish declarations without another owner declaration.

## Finish declaration

Each finish is a child of `CfgPaintZFinishes` and explicitly names its namespace owner.

### Asset-backed example

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

### Basic procedural-color example

A `B`/`basic` finish uses the same registration shape, but its only S100 surface is a DayZ procedural color texture descriptor rather than a PAA path:

```cpp
class CfgPaintZFinishes
{
    class NCP_B_BLK
    {
        id = "NCP-B-BLK";
        owner = "NCP_NetcopMilitaryPaints";
        displayName = "Basic Black";
        type = "basic";

        class Surfaces
        {
            class S100
            {
                scalePercent = 100;
                texture = "#(argb,8,8,3)color(0.149020,0.156863,0.152941,1.0,CO)";
            };
        };
    };
};
```

Basic finishes are predefined registered finishes. The procedural texture descriptor is derived representation, not identity; persistence still stores only `NCP-B-BLK`.

### Required finish fields

- `id` - complete canonical finish ID `<PREFIX>-<TYPE>-<SUFFIX>`;
- `owner` - exact `CfgPaintZPacks` child class that owns the ID prefix;
- `displayName` - human-readable finish name;
- `type` - type name corresponding to the ID type letter;
- `isPattern` - `1` for a scale-selectable pattern/camouflage finish, otherwise `0`/omitted;
- `Surfaces` - one or more explicitly declared runtime surface representations.

PaintZ accepts a finish only when the prefix extracted from `id` resolves to one active namespace owner and the finish's `owner` field matches that owner's config child class exactly.

Every finish must declare a `scalePercent = 100` surface. A non-pattern finish must currently declare only the 100% surface. A pattern may declare any subset of valid whole percentages from 1 through 1000; PaintZ selects only variants actually declared by that finish and falls back to 100% when a configured scale is unavailable.

For `type = "basic"`:

- the ID type letter must be `B`;
- `isPattern` must not be set;
- only S100 is valid under the normal non-pattern rule;
- the S100 `texture` value must be a `#(argb,8,8,3)color(...)` procedural color descriptor;
- no target-surface PAA is required.

PaintZ does not construct paint-pack texture paths or procedural colors from the finish ID. The pack declaration must expose the actual representation.

### Type names

The current mapping is:

```text
B -> basic
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

`B/basic` means a plain single RGB color with no added treatment. `S/solid` means a fundamentally single-color finish whose target-surface asset may include noise, scratches, grime, wear, mottling, rust hints, or similar detail.

The `type` string and ID type letter must agree. PackKit should emit one canonical spelling per type even though the runtime accepts the documented synonyms for interoperability.

## Spray-can class

PaintZ owns the common non-spawnable `PaintZ_SprayCanBase`. A pack provides one thin spawnable subclass per finish:

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

Basic finishes still use normal generated can artwork. Eliminating the target-surface PAA does not eliminate the spray-can presentation texture.

A pack may use the same suffix across different finish types because the complete IDs remain distinct, for example `NCP-B-FDE` and `NCP-S-FDE`. Generated `CfgVehicles` classnames must nevertheless remain unique. PackKit must detect classname collisions, and a pack may provide explicit distinct `dayz_class` values where compatibility requires legacy classnames to be preserved.

The Standard Pack intentionally preserves historical official `PaintZ_SprayCan_*` classnames for existing finishes. New Basic counterparts therefore use distinct Basic-specific can classnames rather than repurposing those historical classes.

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
- Basic finishes must use the procedural-color S100 representation described above;
- duplicate complete finish IDs are disabled rather than overwritten;
- PaintZ also rejects an internal network-hash collision rather than allowing ambiguous client resolution.

No first-loaded-wins or last-loaded-wins rule is used.

## Config-key collision limitation

The Paint Pack API does not claim cryptographic authorship or hostile-mod isolation. DayZ combines config trees before PaintZ enumerates them. If two add-ons deliberately define the exact same `CfgPaintZPacks` child class, the engine may merge/override that config entry before PaintZ can distinguish the sources.

Therefore owner class names should be distinctive and PackKit should generate them predictably from pack metadata, but `owner` must not be presented as a security token. Runtime namespace/finish collision handling is designed for deterministic interoperability among normally authored mods.

## Core/content separation

PaintZ core defines the registry, generic can base, generic paint/strip actions, persistence, networking, model inspection, policy and pattern-scale behavior.

PaintZ core understands procedural Basic surfaces generically but does **not** contain an official Basic color catalogue. Official `PZ-B-*` definitions belong in `PaintZ-Standard-Pack`, exactly like official Solid/Camouflage content.

PaintZ core does not register an implicit `PZ` owner and does not contain an official finish catalogue, finish-specific spray cans or official finish textures.

The official `PZ` namespace is supplied by `PaintZ-Standard-Pack` through ordinary API-v1 config registration. Third-party packs use the same mechanism without `official = 1` and with their own non-reserved prefixes.

A server that wants the official PaintZ finishes therefore loads:

```text
CF -> PaintZ -> PaintZ Standard Pack
```

## Conformance fixture

`tools/sandbox/paint-pack-api-fixture` contains a non-shipping synthetic paint pack with valid and deliberately invalid registrations. It exists to exercise the actual merged DayZ config tree, including namespace conflicts, owner mismatch, reserved-prefix rejection, duplicate finish IDs, generic painting, and unresolved historical-state stripping.

Basic procedural finish acceptance/rejection should be included in this conformance coverage when the fixture is next extended.

For the final Standard Pack cut-over procedure, see `STANDARD_PACK_CUTOVER_ACCEPTANCE.md`.
