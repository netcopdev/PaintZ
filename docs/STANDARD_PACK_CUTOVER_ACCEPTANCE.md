# PaintZ core-owned `PZ` / Standard Pack acceptance

This procedure validates the architecture in which PaintZ core owns the official `PZ` namespace while PaintZ Standard Pack is an independent content contributor.

The authoritative contracts are `PAINT_PACK_API.md` and `PAINT_PACK_CONFIG_V1.md`.

## Required coordinated branches

Use the matching feature branches without merging solely to perform this test:

```text
PaintZ
  feature/core-owned-pz-namespace

PaintZ-PackKit
  feature/core-owned-pz-namespace

PaintZ-Standard-Pack
  feature/core-owned-pz-namespace
```

## 1. Generate/build Standard Pack

With the PackKit feature branch available as the sibling checkout:

```powershell
cd E:\DayZDev\PaintZ-Standard-Pack
pwsh -File .\tools\Build.ps1
```

Expected release PBO:

```text
dist\release\@PaintZ-Standard-Pack\Addons\PaintZ_Standard_Pack.pbo
```

For the current 13-finish catalogue, generation should still produce the expected can/surface assets for those finishes. Catalogue reorganization is a separate content change and is not required for this namespace acceptance.

## 2. Inspect generated Standard Pack config

The generated config **must not** contain a `CfgPaintZPacks` declaration for `PZ`.

Every official finish registration must contain:

```text
owner = "PZ_PaintZOfficial"
```

`CfgPatches.requiredAddons[]` must contain:

```text
PaintZ_DynamicPaint
```

and must not contain a dependency on another official content pack merely for namespace access, including:

```text
PaintZ_Standard_Pack   # as an owner dependency inside another official pack
PaintZ_Military_Pack
PaintZ_Pastel_Pack
PaintZ_Hunting_Pack
```

Standard Pack itself naturally has its own patch classname; the prohibition is on content-pack dependency chains.

## 3. Inspect PaintZ core config

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

PaintZ core must not contain individual official finish registrations, official finish textures, or finish-specific spray-can subclasses.

## 4. Build the synthetic API fixture

```powershell
cd E:\DayZDev\PaintZ
pwsh -File .\tools\sandbox\Build-PaintPackApiFixture.ps1
```

The fixture should continue to exercise valid third-party registration plus deliberate namespace/owner/finish conflicts.

Where the fixture contains assumptions that Standard Pack owns `PZ`, update the fixture first. The canonical official owner is now always `PZ_PaintZOfficial` from PaintZ core.

## 5. Start sandbox with Standard Pack

Example:

```powershell
cd E:\DayZDev\PaintZ

$standardPack = "E:\DayZDev\PaintZ-Standard-Pack\dist\release\@PaintZ-Standard-Pack"
$fixture = "$env:LOCALAPPDATA\PaintZSandbox\@PaintZ-PaintPackApiFixture"

& .\tools\sandbox\Start-PaintZSandbox.ps1 `
  -Build `
  -AdditionalMods @($standardPack, $fixture)
```

Logical load/dependency order:

```text
CF -> PaintZ -> Standard Pack
             -> fixture
```

## 6. Registry expectations

Required positive evidence includes one active official namespace owner from PaintZ core:

```text
prefix=PZ
owner=PZ_PaintZOfficial
```

All currently shipped Standard Pack finish IDs should register against that owner.

Required negative evidence:

```text
no second PZ owner
no PZ namespace conflict
no PZ_PaintZStandardPack owner reference
no first/last-loaded overwrite behavior
no PaintZ-caused config/script exception
```

Deliberate third-party fixture warnings remain acceptable when they match the fixture's documented invalid cases.

## 7. Manual finish checks

Check at least:

1. one Standard solid can;
2. one Standard patterned/camouflage can;
3. one external third-party fixture can;
4. PaintZ paint stripper.

For each applicable official can verify:

- the generic Paint action is offered on a supported target;
- the expected finish name/ID is stored/displayed;
- the registered Standard Pack surface is applied;
- paint quantity is consumed;
- Strip Paint remains available and restores original appearance/logical state correctly.

## 8. Missing-pack / independence check

Run PaintZ **without Standard Pack**.

Acceptance:

- `PZ` namespace still exists because PaintZ owns it;
- there are simply no Standard Pack finish registrations;
- PaintZ does not require Standard Pack to start;
- historical persisted `PZ-*` IDs remain unresolved but preserved/strippable according to persistence rules.

Then restore Standard Pack and verify those IDs resolve again.

## 9. Peer official-pack check

Create or generate a small temporary official test pack with `--official`, a unique test `PZ-*` finish, and no Standard Pack dependency.

Test both:

```text
PaintZ + temporary official pack
```

and:

```text
PaintZ + Standard Pack + temporary official pack
```

Acceptance:

- the temporary official pack works without Standard Pack installed;
- both official packs register unique `PZ-*` finishes together;
- both reference `PZ_PaintZOfficial`;
- neither declares `PZ`;
- duplicate finish IDs are still rejected if deliberately introduced.

This test proves the reason for the architecture change: official content packages are independent peers.

## Acceptance

The core-owned namespace design is acceptable when:

- PaintZ builds/loads with one `PZ_PaintZOfficial` namespace owner;
- Standard Pack emits no namespace owner;
- Standard Pack finishes register against the core owner;
- PaintZ runs without Standard Pack;
- Standard Pack depends only on PaintZ for runtime/namespace access;
- a second independent official test pack works with and without Standard Pack;
- duplicate finish-ID and namespace collision protections remain deterministic;
- painting, stripping, synchronization and persistence behavior remain intact;
- no PaintZ-caused config/script errors remain.

Only after this coordinated acceptance should these branches be considered candidates for integration.
