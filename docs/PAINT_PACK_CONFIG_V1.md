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

The child class name, here `NCP_NetcopMilitaryPaints`, is the runtime owner key referenced by that pack's finish declarations. It is not user-facing and is not persisted. PackKit should generate a deterministic, distinctive owner class name for the pack.

For an official reserved `PZ*` namespace, the declaration additionally uses:

```cpp
official = 1;
```

This flag is an interoperability gate, not cryptographic authentication. Ordinary third-party packs must not set it and must not claim `PZ*`.

A multi-PBO pack family declares the namespace in only one owner/core PBO. Satellite PBOs depend on that PBO through normal `CfgPatches.requiredAddons[]` and reference that same owner class from their finish declarations without another owner declaration.

## Finish declaration

Each finish is a child of `CfgPaintZFinishes` and explicitly names its namespace owner:

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

Required fields:

- `id` - complete canonical finish ID `<PREFIX>-<TYPE>-<SUFFIX>`;
- `owner` - exact `CfgPaintZPacks` child class that owns the ID prefix;
- `displayName` - human-readable finish name;
- `type` - type name corresponding to the ID type letter;
- `isPattern` - `1` for a scale-selectable pattern/camouflage finish, otherwise `0`/omitted;
- `Surfaces` - one or more explicitly declared runtime surface assets.

PaintZ accepts a finish only when the prefix extracted from `id` resolves to one active namespace owner and the finish's `owner` field matches that owner's config child class exactly. This owner key is deliberately not another public/canonical ID and is not a security credential; it is config-level linkage that prevents an unrelated declaration from accidentally contributing finishes to some other active namespace.

Every finish must declare a `scalePercent = 100` surface. A non-pattern finish must currently declare only the 100% surface. A pattern may declare any subset of valid whole percentages from 1 through 1000; PaintZ selects only variants actually declared by that finish and falls back to 100% when a configured scale is unavailable.

The runtime does not construct third-party texture paths from the finish ID.

### Type names

The current mapping is:

```text
S -> solid
C -> camo
P -> pattern
M -> metallic
R -> rusted
W -> weathered
F -> fluorescent
X -> special or custom
T -> transparent
```

The `type` string and ID type letter must agree.

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

`paintzFinish` is the API-v1 property. During the migration from the embedded generator, PaintZ also accepts legacy `paintzCode` on existing official cans.

The can does not define its own action class. `PaintZ_SprayCanBase` attaches the single generic PaintZ paint action, which resolves the finish from `paintzFinish` and the runtime registry.

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

The Paint Pack API does not claim cryptographic authorship or hostile-mod isolation. DayZ combines config trees before PaintZ enumerates them. If two add-ons deliberately define the exact same `CfgPaintZPacks` child class, the engine may merge/override that config entry before PaintZ can distinguish the sources.

Therefore owner class names should be distinctive and PackKit should generate them predictably from pack metadata, but `owner` must not be presented as a security token. Runtime namespace/finish collision handling is designed for deterministic interoperability among normally authored mods.

## Current transition bridge

Until `PaintZ-Standard-Pack` takes ownership of official finishes, PaintZ core creates an internal legacy owner for `PZ` and imports the generated built-in `PZ-*` catalogue into the runtime registry.

Consequences during this temporary phase:

- existing PaintZ cans continue to work;
- the runtime path is already registry/generic-action based;
- an additional `PZ` owner declaration conflicts with the bridge and disables `PZ`, as required by normal collision semantics;
- Standard Pack migration must remove the legacy owner/catalogue bridge from PaintZ core at the same time that the external official `PZ` owner becomes authoritative.

The bridge is migration infrastructure, not part of the permanent Paint Pack API.
