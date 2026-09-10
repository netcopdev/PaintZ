# Paint Pack API v1 - DayZ config representation

This document defines the concrete DayZ config representation consumed by the current PaintZ API-v1 runtime registry. Higher-level identity/collision/persistence semantics remain authoritative in `PAINT_PACK_API.md`.

## Discovery roots

PaintZ exposes:

```cpp
class CfgPaintZPacks {};
class CfgPaintZFinishes {};
```

PaintZ core contributes the canonical official `PZ` namespace owner under `CfgPaintZPacks`. Third-party owner packs add their own non-reserved owner classes there. Content packs add finish classes under `CfgPaintZFinishes`.

Config child class names are DayZ config keys, not persisted PaintZ identities. Keep them distinctive because DayZ merges config trees before PaintZ enumerates them.

## Runtime dependency

Every normal content pack depends on PaintZ through ordinary DayZ add-on ordering:

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

PaintZ must never require a content pack in the opposite direction.

Independent official `PZ` packs also require only `PaintZ_DynamicPaint`; they do not require Standard Pack or another official content pack.

## PaintZ core's official namespace owner

PaintZ core declares:

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

`PZ_PaintZOfficial` is the canonical API-v1 owner linkage key for official `PZ-*` finishes. It is not a persisted finish identity and does not mean PaintZ core owns finish assets.

Official content packs **must not** emit another `CfgPaintZPacks` declaration for `PZ`.

The runtime does not treat `official = 1` as general permission to claim a reserved prefix. For API v1 a reserved-prefix owner is accepted only when it exactly matches an assigned core owner. Currently the only accepted reserved owner is:

```text
prefix = PZ
owner class = PZ_PaintZOfficial
```

Legacy `PZ` owner classes and all currently unassigned `PZ?` owner declarations are rejected before ordinary namespace-collision resolution. Additional reserved namespaces require a future explicit assignment in PaintZ runtime.

## Third-party namespace-owner declaration

A normal third-party standalone pack declares its namespace once:

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

- `apiVersion` - currently `1`;
- `prefix` - permanent 2-3 character public namespace.

Optional metadata:

- `displayName` - human-readable pack/family name.

The owner child classname is config linkage only. It is not user-facing and is not persisted.

A third-party multi-PBO family declares the namespace in one owner/core PBO. Satellite PBOs reference that owner and depend on its `CfgPatches` classname.

Third-party packs must not set `official = 1` and must not claim `PZ*`.

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
        };
    };
};
```

Independent official-pack example:

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

The official pack's local finish config classname should be distinctive to that pack. Only the `owner` linkage is shared.

Required finish fields:

- `id` - complete canonical `<PREFIX>-<TYPE>-<SUFFIX>` identity;
- `owner` - exact active `CfgPaintZPacks` owner child classname;
- `displayName` - human-readable finish name;
- `type` - type name matching the ID type letter;
- `isPattern` - `1` for scale-selectable pattern/camouflage finishes, otherwise omitted/0;
- `Surfaces` - one or more explicitly declared runtime surfaces.

For every official API-v1 `PZ-*` finish:

```text
owner = "PZ_PaintZOfficial"
```

Every finish must expose a 100% surface. Non-pattern finishes currently expose only 100%. Patterns may expose any valid whole-percentage subset from 1 through 1000; PaintZ chooses only variants actually registered and falls back to 100% when the requested scale is absent.

PaintZ never constructs a third-party texture path from the finish ID.

## Type names

Current integrated mapping:

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

The type string and ID type letter must agree. PackKit should emit one canonical spelling per type even where runtime accepts a documented synonym.

## Spray-can class

PaintZ owns the non-spawnable `PaintZ_SprayCanBase`. A content pack supplies one thin spawnable subclass per finish:

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

The can does not define its own action class. `PaintZ_SprayCanBase` attaches the generic PaintZ action, which resolves behavior through `paintzFinish` and the runtime registry.

When moving a released official finish between official content packs, preserve its complete `PZ-*` ID and existing can classname where practical.

## Official pack independence

A normal official content pack generated for `PZ` has this shape:

```text
CfgPatches
  requiredAddons[] = { "PaintZ_DynamicPaint" }

(no CfgPaintZPacks declaration)

CfgPaintZFinishes
  each finish owner = "PZ_PaintZOfficial"

CfgVehicles
  thin spray-can subclasses
```

Valid peer packages include:

```text
PaintZ Standard Pack -> PaintZ
PaintZ Pastel Pack   -> PaintZ
PaintZ Military Pack -> PaintZ
PaintZ Hunting Pack  -> PaintZ
```

No dependency from Pastel/Military/Hunting to Standard is required, nor between other official content packs.

## Third-party satellite packs

Satellite mode remains available for a third-party multi-PBO family whose namespace owner is itself external.

A satellite:

- emits no `CfgPaintZPacks`;
- references its family's existing owner class;
- requires `PaintZ_DynamicPaint` plus the owner/core `CfgPatches` class;
- owns its own textures and thin spray-can classes.

Do not use this owner/satellite dependency pattern for normal official `PZ` collections. PaintZ core is already their stable owner dependency.

## Runtime validation order

PaintZ performs two phases:

1. discover and validate namespace owners;
2. validate/register finish declarations after namespace status is known.

Reserved `PZ*` owner candidates are checked against core assignments first. Currently only `PZ_PaintZOfficial` / `PZ` is valid. Other reserved owner declarations are rejected.

For ordinary non-reserved namespace owners:

- one valid owner -> namespace active;
- multiple valid owners -> namespace conflicted and disabled;
- invalid/unsupported declarations acquire no namespace.

For finishes:

- ID syntax/prefix must be valid;
- prefix must resolve to an active namespace;
- `owner` must exactly match the active owner class;
- metadata/type/surfaces must validate;
- duplicate complete finish IDs are disabled rather than overwritten;
- internal network-hash collisions are rejected rather than allowed to resolve ambiguously.

No first-loaded-wins or last-loaded-wins rule is used.

## Config-key collision limitation

DayZ combines config trees before PaintZ enumerates them. If two add-ons deliberately define the exact same config child class, the engine may merge/override that entry before PaintZ can distinguish the sources.

Therefore third-party owner classnames and every pack's local finish config classnames should be distinctive. Owner linkage must not be presented as cryptographic security.

## Core/content separation

PaintZ core defines:

- runtime registry/validation;
- `PZ_PaintZOfficial` ownership of `PZ`;
- generic can base and paint/strip actions;
- persistence/networking;
- model inspection/policy;
- pattern-scale behavior.

PaintZ core does **not** contain an official finish catalogue, finish-specific spray cans, or official finish assets.

A server wanting official content loads PaintZ plus whichever independent official content packs it wants:

```text
CF -> PaintZ -> { Standard, Pastel, Military, Hunting, ... }
```

The braces describe peer optional packages, not a nested dependency chain.

## Conformance fixture

`tools/sandbox/paint-pack-api-fixture` exercises the merged DayZ config tree, including ordinary namespace conflicts, owner mismatch, reserved-owner rejection, duplicate finish IDs, a valid finish contributed to the core-owned `PZ` namespace, generic painting, and unresolved historical-state stripping.
