# Paint Pack API v1 sandbox fixture

This fixture is development/test input for the PaintZ runtime registry. It lives under `tools/sandbox` and is not part of the normal PaintZ PBO.

It exercises the merged DayZ config tree with both valid and deliberately invalid declarations:

- core-owned official namespace `PZ` supplied by PaintZ itself;
- valid synthetic official contribution `PZ-S-API` referencing `PZ_PaintZOfficial` without declaring another owner;
- valid third-party namespace `TST`;
- valid Basic procedural finish `TST-B-RED`;
- rejection of invalid Basic finish `TST-B-BAD` whose S100 surface is a PAA path instead of a procedural color descriptor;
- valid pattern finish `TST-C-PAT` with only 50% and 100% variants;
- owner mismatch rejection;
- duplicate complete finish-ID rejection;
- duplicate namespace-owner rejection (`DUP`);
- reserved third-party `PZA` rejection;
- unsupported API-version rejection (`AP2`);
- thin spray-can classes using `paintzFinish`;
- the generic `ActionPaintZPaint` runtime path with a Basic finish;
- stripping of a registered external finish;
- preservation and stripping of an unresolved historical finish ID.

The procedural `#(argb...)color(...)` surfaces are deliberate. `TST-B-RED` specifically exercises API-v1 Basic representation without introducing finish artwork into the PaintZ repository. Some deliberately invalid/non-B fixture entries also use procedural colors simply to avoid shipping test artwork; their acceptance/rejection is driven by the condition each fixture is testing.

## Build

From the PaintZ repository root:

```powershell
.\tools\sandbox\Build-PaintPackApiFixture.ps1
```

Default output:

```text
%LOCALAPPDATA%\PaintZSandbox\@PaintZ-PaintPackApiFixture
```

Pass `-AddonBuilderExe` if DayZ Tools is installed somewhere the helper does not detect.

## Run

Build PaintZ normally, then load the fixture as an additional mod:

```powershell
$fixture = "$env:LOCALAPPDATA\PaintZSandbox\@PaintZ-PaintPackApiFixture"
.\tools\sandbox\Start-PaintZSandbox.ps1 -AdditionalMods $fixture
```

The fixture runs registry checks from `MissionServer.OnInit()` and player/application checks after the test player connects. Search script/RPT logs for:

```text
[PaintZ][PackAPI Smoke]
```

Any `FAIL` line is an acceptance failure.

## Expected registry behavior

`PZ` must be active from PaintZ core and `PZ-S-API` must register even though the fixture does not declare a `PZ` owner. This verifies the independent official-content model.

`TST` must be active. `TST-B-RED` and `TST-C-PAT` must register. `TST-B-BAD`, `TST-S-BAD`, and `TST-S-DUP` must not register.

`DUP`, `PZA`, and `AP2` must not become active namespaces. No invalid/conflicted declaration may replace another owner or finish because of load order.

The player smoke test must be able to paint an M4 with `TST-B-RED`, retain the logical Basic finish ID, apply the registered procedural surface, consume paint, and strip it normally.

This fixture is not an example of a normal distributed paint pack: it deliberately mixes a synthetic official contribution, third-party valid content, invalid declarations, and test scripts for runtime conformance testing. Normal pack shapes are documented in `docs/PAINT_PACK_CONFIG_V1.md`.
