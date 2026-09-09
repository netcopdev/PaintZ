# PaintZ Standard Pack cut-over acceptance

This procedure validates the final separation of PaintZ runtime core from the official PaintZ Standard Pack.

It is intentionally different from the earlier registry-bridge acceptance run: the core legacy `PZ` bridge is removed, so the external Standard Pack must be loaded for official `PZ-*` finishes to exist.

## Required branches

Use these coordinated work branches:

```text
PaintZ
  feature/external-standard-pack-cutover

PaintZ-PackKit
  feature/paint-pack-api-v1-output

PaintZ-Standard-Pack
  feature/standard-pack-v1
```

Do not merge any of them merely to perform this test.

## 1. Seed Standard Pack pattern sources once

The old PaintZ repository historically ignored binary artwork. If the seven Standard Pack source PNGs have not yet been committed to `PaintZ-Standard-Pack`, copy them once from the local pre-split PaintZ checkout:

```powershell
cd E:\DayZDev\PaintZ-Standard-Pack
pwsh -File .\tools\Import-LegacyAssets.ps1 -PaintZRoot E:\DayZDev\PaintZ
```

Required files under `assets\pattern_sources`:

```text
erdl.png
woodland.png
vanilla-marpat-woodland.png
ftn.png
mctp.png
tiger-stripe.png
ucp.png
```

Review them before committing. After they are committed, Standard Pack is their authoritative source and the import helper is no longer part of normal builds.

## 2. Build Standard Pack

Make sure PackKit is on `feature/paint-pack-api-v1-output` and its Python environment has the tested dependencies installed.

Then:

```powershell
cd E:\DayZDev\PaintZ-Standard-Pack
pwsh -File .\tools\Build.ps1
```

Expected release mod:

```text
E:\DayZDev\PaintZ-Standard-Pack\dist\release\@PaintZ-Standard-Pack
```

Expected PBO:

```text
E:\DayZDev\PaintZ-Standard-Pack\dist\release\@PaintZ-Standard-Pack\Addons\PaintZ_Standard_Pack.pbo
```

Generation/build must produce 13 can textures and 48 target surfaces:

- 6 solid 100% surfaces;
- 7 patterned finishes x 6 scales = 42 patterned surfaces;
- total target surfaces = 48.

The generated API config must contain:

```text
CfgPaintZPacks PZ_PaintZStandardPack
prefix = PZ
official = 1
```

and 13 `CfgPaintZFinishes` declarations.

## 3. Build the synthetic API fixture

From the PaintZ cut-over branch:

```powershell
cd E:\DayZDev\PaintZ
pwsh -File .\tools\sandbox\Build-PaintPackApiFixture.ps1
```

The fixture remains useful because it exercises valid third-party registration plus deliberate owner/namespace/finish conflicts alongside the official Standard Pack.

## 4. Start the cut-over sandbox

Do **not** use `-RunSmokeTests` for this test. That switch refers to the older built-in-catalogue smoke harness and is not part of the external Standard Pack acceptance gate.

Run the sandbox script directly in the current PowerShell session so `AdditionalMods` binds as a real `string[]` rather than being flattened by a nested `pwsh -File` invocation:

```powershell
cd E:\DayZDev\PaintZ

$standardPack = "E:\DayZDev\PaintZ-Standard-Pack\dist\release\@PaintZ-Standard-Pack"
$fixture = "$env:LOCALAPPDATA\PaintZSandbox\@PaintZ-PaintPackApiFixture"

& .\tools\sandbox\Start-PaintZSandbox.ps1 `
  -Build `
  -AdditionalMods @($standardPack, $fixture)
```

The launcher loads:

```text
CF -> PaintZ core -> PaintZ Standard Pack -> synthetic API fixture
```

## 5. Registry expectations

Search the server logs:

```powershell
rg -n "paint_pack_registry|\[PaintZ\]\[PackAPI Smoke\]|Virtual Machine Exception|\[PaintZ\].*FAIL" `
  "$env:LOCALAPPDATA\PaintZSandbox\server-profiles" `
  -g "*.RPT" -g "*.log"
```

Required positive evidence:

```text
namespace_registered prefix=PZ owner=PZ_PaintZStandardPack source=CfgPaintZPacks PZ_PaintZStandardPack
```

All 13 official IDs must register from `CfgPaintZFinishes`, including:

```text
PZ-S-RGR
PZ-S-FDE
PZ-S-FGY
PZ-S-UGY
PZ-S-BLK
PZ-S-WHT
PZ-C-ERDL
PZ-C-WDL
PZ-C-DWD
PZ-C-FTN
PZ-C-MCT
PZ-C-TGR
PZ-C-UCP
```

The synthetic fixture should still report:

```text
[PaintZ][PackAPI Smoke] REGISTRY COMPLETE passed=20 failed=0
[PaintZ][PackAPI Smoke] PLAYER COMPLETE passed=50 failed=0
```

Required negative evidence:

```text
no namespace_conflict prefix=PZ
no PaintZ_LegacyBuiltIn
no <PaintZ core legacy bridge>
no [PaintZ] ... FAIL
no PaintZ-caused Virtual Machine Exception
```

Expected deliberate fixture warnings for `DUP`, `PZA`, `AP2`, owner mismatch and duplicate `TST-S-DUP` remain valid success evidence.

With Standard Pack plus the two valid `TST` fixture finishes loaded, the active registry should contain 2 valid namespaces (`PZ`, `TST`) and 15 valid finishes.

## 6. Manual Standard Pack checks

The sandbox now discovers loaded paint cans from `CfgVehicles`/`paintzFinish`, so the Standard Pack cans are staged without any hard-coded runtime catalogue.

Check at least:

1. one solid official can, preferably `PaintZ_SprayCan_FDE`;
2. one patterned official can, preferably `PaintZ_SprayCan_FTN`;
3. one synthetic external can such as the red `TST-S-RED` fixture;
4. `PaintZ_PaintStripperCan`.

For each official can:

- Paint action is offered on a supported M4/AK target;
- correct finish name/ID appears after painting;
- can quantity is consumed;
- patterned finish uses a registered Standard Pack surface;
- Strip Paint is offered afterward;
- stripping restores the original target appearance and clears logical state.

The existing official can classnames are intentionally preserved. No `PaintZ_SprayCan_*` classname migration should be necessary for server configuration.

## 7. Core/pack ownership check

The built PaintZ core PBO must not contain:

- official `PaintZ_SprayCan_*` finish classes;
- `PZ-*` finish surface textures;
- generated finish-specific can textures;
- `PaintZ_PaintCatalog`;
- generated per-finish paint actions.

Those belong to Standard Pack.

PaintZ core should contain only framework/runtime content plus the generic non-spawnable `PaintZ_SprayCanBase` and `PaintZ_PaintStripperCan`.

## Acceptance

The cut-over is acceptable when:

- Standard Pack builds successfully from its manifest/source artwork through PackKit;
- PaintZ core builds without paint generation/assets;
- `PZ` registers exactly once from Standard Pack;
- all 13 official finishes register;
- synthetic PackAPI suites remain 20/20 and 50/50;
- solid/pattern official painting and stripping work manually;
- no `PZ` collision, legacy-bridge reference, PaintZ `FAIL`, or PaintZ VM exception remains.

Only after this coordinated acceptance should the Standard Pack and PaintZ cut-over branches be considered candidates for integration.
