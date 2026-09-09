# Paint Pack API v1 runtime acceptance

This checklist is the acceptance gate for the PaintZ runtime-registry implementation. It does not replace `PAINT_PACK_API.md` or `PAINT_PACK_CONFIG_V1.md`; it defines how to prove the implementation actually loads and behaves correctly in DayZ.

The branch must not be merged solely because static review passes. Enforce Script and DayZ config must be compiled/loaded by DayZ Tools/DayZDiag.

## 1. Static preflight

Before building:

```powershell
git switch feature/paint-pack-runtime-registry
git status --short
rg -n "STRIP_COST|ActionPaintZPaint_[A-Z]" tools\sandbox\PaintZ_FinishSmokeTest.c
rg -n '^\s*[A-Za-z_][A-Za-z0-9_\.]*\s*\(\s*$' Scripts tools\sandbox\paint-pack-api-fixture -g '*.c'
```

Expected:

- no `STRIP_COST` reference in the current smoke suite;
- no generated per-finish paint action used by the current smoke-suite action tests;
- every parser-safety grep match is a declaration or otherwise intentionally valid, not a vertically split invocation.

The transitional `PaintZ_PaintCatalog` is still expected while the built-in `PZ` bridge exists. It is removed only when Standard Pack becomes the official `PZ` owner.

## 2. Build PaintZ

With existing generated legacy paint assets:

```powershell
pwsh -File .\tools\build.ps1 -SkipPaintZGen
```

If the checkout has no generated legacy assets, regenerate them for this transitional branch instead:

```powershell
pwsh -File .\tools\build.ps1
```

Acceptance:

- AddonBuilder completes successfully;
- `PaintZ.pbo` is freshly produced;
- no Enforce compile error is reported when the PBO is subsequently loaded by DayZDiag;
- current built-in `PZ-*` cans still load through the temporary registry bridge.

## 3. Build the external API fixture

```powershell
pwsh -File .\tools\sandbox\Build-PaintPackApiFixture.ps1
```

Default result:

```text
%LOCALAPPDATA%\PaintZSandbox\@PaintZ-PaintPackApiFixture
```

This fixture is test-only and deliberately contains invalid registrations in addition to valid `TST` content.

## 4. Run both smoke suites

```powershell
$fixture = "$env:LOCALAPPDATA\PaintZSandbox\@PaintZ-PaintPackApiFixture"

pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 `
  -Build `
  -RunSmokeTests `
  -AdditionalMods $fixture
```

Do not use `-ServerOnly` for the full acceptance run because both suites include player/connect-time checks.

Search the server/client RPT and script logs for:

```text
[PaintZ][Smoke]
[PaintZ][PackAPI Smoke]
[PaintZ][WARNING] paint_pack_registry
SCRIPT (E)
SCRIPT (W)
```

## 5. Expected registry results

The fixture must produce these effective results independent of load order:

```text
TST              active
TST-S-RED        active
TST-C-PAT        active
TST-S-BAD        rejected: owner mismatch
TST-S-DUP        rejected: duplicate full finish ID
DUP              disabled: two namespace owners
DUP-S-RED        unavailable because DUP is conflicted
PZA              rejected: reserved PZ* namespace used as third party
AP2              rejected: unsupported API version
```

Warnings for the deliberately invalid fixture entries are expected. They are successful test evidence, not acceptance failures.

There must be no first-loaded-wins or last-loaded-wins behavior.

## 6. Expected action/state results

The Pack API fixture must report no `FAIL` lines and must prove at least:

- `PaintZ_TestSprayCan_RED` resolves `paintzFinish = "TST-S-RED"`;
- the single generic `ActionPaintZPaint` accepts the external-pack can;
- painting stores `TST-S-RED` as logical state;
- the explicitly registered external surface is applied;
- paint quantity uses current size-based action tuning;
- Strip Paint removes the external finish and uses current size-based stripper tuning;
- painting sets the synchronized PaintZ-state marker and stripping clears it;
- an unresolved historical ID keeps logical state but produces network hash `0`;
- unresolved state with a recoverable selection remains strippable;
- unresolved state with no recoverable paint selection still remains strippable through the synchronized state marker.

The legacy smoke suite must also have no `[PaintZ][Smoke] FAIL` lines, proving that the temporary built-in `PZ` bridge still supports current PaintZ behavior while migration is in progress.

## 7. Expected network safety

A registered finish may synchronize only when its finish-ID hash is unique among all valid registered finishes.

An unresolved finish must synchronize:

```text
hasState = true
finish hash = 0
selection = resolved selection or -1
```

It must never synchronize the unresolved string's raw hash. This prevents an unavailable historical ID from accidentally resolving to an unrelated active finish whose hash happens to collide.

A stripped/unpainted item must synchronize:

```text
hasState = false
finish hash = 0
selection = -1
```

## 8. Manual visual checks

During the client run verify:

1. one current built-in `PZ-*` can still offers Paint and applies its expected finish;
2. `PaintZ_TestSprayCan_RED` offers the same generic Paint action and paints a supported M4 red;
3. the test can does not need its own Enforce action subclass;
4. Paint Stripper restores the original target appearance for both built-in and test-pack paint;
5. unsupported/relevant-target feedback remains unchanged from current PaintZ policy behavior.

## 9. Persistence tests not provided by the no-hive sandbox

The safe sandbox intentionally does not start Central Economy/Hive persistence. Its serializer smoke tests and synthetic unresolved-state checks validate the code path, but they do not replace a real persisted-server restart test.

Before a public Paint Pack API release, also test on a disposable persistent server:

1. install PaintZ plus an API-v1 paint pack;
2. paint items in player inventory, nested storage and vehicle cargo;
3. restart and verify the registered finish restores;
4. stop the server and remove only the paint pack;
5. restart and verify the underlying items load with original/default appearance while PaintZ state remains strippable;
6. restore the same pack and restart;
7. verify the original finish ID resolves again on items that were not stripped;
8. verify stripped items remain stripped.

This persistent-server test is mandatory before calling missing-pack persistence release-tested.

## 10. Acceptance decision

The runtime-registry branch is ready for integration review only when:

- PaintZ builds and loads in DayZDiag without PaintZ script/config errors;
- the legacy smoke suite reports no failures;
- the Paint Pack API fixture reports no failures;
- expected invalid fixture registrations are rejected exactly as documented;
- manual built-in and external-can paint/strip checks pass;
- any remaining untested persistent-server cases are explicitly recorded rather than implied to have passed.

The later Standard Pack migration must remove the legacy owner/catalogue bridge and repeat the relevant acceptance checks with Standard Pack as the real `PZ` namespace owner.
