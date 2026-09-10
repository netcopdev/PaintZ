# PaintZ core-owned `PZ` / official-pack regression acceptance

This procedure validates the current architecture in which PaintZ core owns the official `PZ` namespace while Standard, Field, Vanilla and other official packs are independent content contributors.

The authoritative contracts are `PAINT_PACK_API.md` and `PAINT_PACK_CONFIG_V1.md`. This is now a regression checklist, not a pending cutover procedure.

## 1. Use current integration state

Run this acceptance against current `main` branches, or against coordinated work branches when validating a new cross-repository change. Do not rely on obsolete `feature/core-owned-pz-namespace` branches.

Relevant repositories currently include:

```text
PaintZ
PaintZ-PackKit
PaintZ-Standard-Pack
PaintZ-Field-Pack
PaintZ-Vanilla-Pack
```

## 2. Generate/build official packs

With PackKit available as the sibling authoring dependency, build the packs being tested with their canonical scripts, for example:

```powershell
cd E:\DayZDev\PaintZ-Standard-Pack
pwsh -File .\tools\Build.ps1
```

Expected Standard release PBO:

```text
dist\release\@PaintZ-Standard-Pack\addons\PaintZ_Standard_Pack.pbo
```

Field and Vanilla use their corresponding lowercase `addons`, `keys`, and `server-config` release directories.

Current catalogue expectations:

- Standard: 16 general-purpose `PZ-B-*` Basic colors;
- Field: 8 field/service Basics plus ERDL/WDL/FTN/MCT/TGR/UCP camouflage;
- Vanilla: `PZ-C-DWD` as the initial vanilla-inspired finish.

Each pack owns its referenced source assets locally.

## 3. Inspect generated official-pack config

Generated official-pack config **must not** contain a `CfgPaintZPacks` declaration for `PZ`.

Every official finish registration must contain:

```text
owner = "PZ_PaintZOfficial"
```

`CfgPatches.requiredAddons[]` must contain:

```text
PaintZ_DynamicPaint
```

and must not contain another official content pack merely for namespace or asset access.

Basic finishes must use one procedural S100 color surface and must not require a target-surface PAA. Asset-backed finishes must reference only assets owned by the pack that registers them.

## 4. Inspect PaintZ core config

PaintZ core must declare exactly one canonical owner:

```cpp
class CfgPaintZPacks
{
    class PZ_PaintZOfficial
    {
        apiVersion = 1;
        prefix = "PZ";
        official = 1;
    };
};
```

PaintZ core must not contain individual official finish registrations, official finish textures, finish-specific spray-can subclasses, or the removed legacy in-core `paintzgen` authoring implementation.

## 5. Build the synthetic API fixture

```powershell
cd E:\DayZDev\PaintZ
pwsh -File .\tools\sandbox\Build-PaintPackApiFixture.ps1
```

The fixture should continue to exercise valid third-party registration plus deliberate namespace/owner/finish conflicts.

The canonical official owner is always `PZ_PaintZOfficial` from PaintZ core.

## 6. Start sandbox with current official packs

Example:

```powershell
cd E:\DayZDev\PaintZ

$standardPack = "E:\DayZDev\PaintZ-Standard-Pack\dist\release\@PaintZ-Standard-Pack"
$fieldPack = "E:\DayZDev\PaintZ-Field-Pack\dist\release\@PaintZ-Field-Pack"
$vanillaPack = "E:\DayZDev\PaintZ-Vanilla-Pack\dist\release\@PaintZ-Vanilla-Pack"
$fixture = "$env:LOCALAPPDATA\PaintZSandbox\@PaintZ-PaintPackApiFixture"

& .\tools\sandbox\Start-PaintZSandbox.ps1 `
  -Build `
  -AdditionalMods @($standardPack, $fieldPack, $vanillaPack, $fixture)
```

Logical load/dependency order:

```text
CF -> PaintZ -> Standard Pack
             -> Field Pack
             -> Vanilla Pack
             -> fixture
```

## 7. Registry expectations

Required positive evidence includes one active official namespace owner from PaintZ core:

```text
prefix=PZ
owner=PZ_PaintZOfficial
```

All current official finish IDs should register against that owner with no duplicate complete IDs.

Required negative evidence:

```text
no second PZ owner
no PZ namespace conflict
no obsolete content-pack PZ owner reference
no first/last-loaded overwrite behavior
no PaintZ-caused config/script exception
```

Deliberate third-party fixture warnings remain acceptable when they match the fixture's documented invalid cases.

## 8. Manual finish checks

Check at least:

1. one Standard Basic can;
2. one Field Basic can;
3. one Field camouflage can;
4. Vanilla DWD;
5. one external third-party fixture can;
6. PaintZ paint stripper.

For each applicable can verify:

- the generic Paint action is offered on a supported target;
- the expected finish name/ID is stored/displayed;
- the registered surface representation is applied;
- paint quantity is consumed;
- Strip Paint restores original appearance/logical state correctly;
- generated can artwork includes the visible PaintZ top logo with the red `Z`.

## 9. Missing-pack / independence checks

Run PaintZ without any official content pack.

Acceptance:

- `PZ` namespace still exists because PaintZ owns it;
- there are simply no registrations from the removed packs;
- PaintZ does not require Standard, Field or Vanilla to start;
- historical persisted `PZ-*` IDs remain unresolved but preserved/strippable according to persistence rules.

Then test each official pack independently with PaintZ. Field and Vanilla must not require Standard.

## 10. Packaging-only catalogue move check

The completed camouflage split is a reference case:

- ERDL/WDL/FTN/MCT/TGR/UCP are registered by Field with unchanged `PZ-C-*` IDs;
- DWD is registered by Vanilla with unchanged `PZ-C-DWD`;
- Standard no longer registers those finishes or carries their source PNGs.

Persisted items with unchanged camouflage IDs must resolve from their new package without migration or alias data.

## 11. Solid-to-Basic migration check

The six retired Solid IDs are a different case because their canonical IDs changed:

```text
PZ-S-RGR -> PZ-B-RGR
PZ-S-FDE -> PZ-B-FDE
PZ-S-FGY -> PZ-B-FGY
PZ-S-UGY -> PZ-B-UGY
PZ-S-BLK -> PZ-B-BLK
PZ-S-WHT -> PZ-B-WHT
```

When historical persisted state exists, configure exact mappings through `$profile:PaintZ/paintz_stale_finishes.json` and verify migration/persistence through the normal stale-finish acceptance procedure. The content packs do not activate these mappings automatically.

## Acceptance

The architecture remains acceptable when:

- PaintZ builds/loads with one `PZ_PaintZOfficial` namespace owner;
- official packs emit no namespace owner and register against the core owner;
- PaintZ runs without official content packs;
- Standard, Field and Vanilla each depend only on PaintZ for runtime/namespace access;
- multiple independent official packs work together and separately;
- duplicate finish-ID and namespace collision protections remain deterministic;
- Basic procedural surfaces and asset-backed surfaces both work through the same runtime registry;
- packaging-only moves preserve unchanged IDs without migration;
- explicit stale-finish migration handles only deliberately retired identities;
- painting, stripping, synchronization and persistence behavior remain intact;
- no PaintZ-caused config/script errors remain.
