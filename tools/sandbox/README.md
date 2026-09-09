# PaintZ safe sandbox

Double-click `Run-PaintZSandbox.cmd` in the repository root. By default the launcher
reuses the newest packaged `PaintZ.pbo` it can find in the sandbox runtime or the
project's `dist` directory, starts `DayZDiag_x64.exe` in server mode, waits for the
mission-ready marker, then launches a second Diag instance as the client and connects it to
`127.0.0.1:2302`. Closing that DayZ client
also stops the server.

Building is opt-in. Pass `-Build` to package current source using the existing
generated paint assets, or `-Generate` to regenerate paint assets and then package
the mod before launch. A fresh checkout with no packaged PBO reports this choice
instead of silently starting a full build.

Run the file directly from the same Windows desktop account that owns the
signed-in Steam process. The launcher checks this before building; a Steam game
cannot initialize correctly if another service or sandbox account starts it.

The launcher auto-detects the installed Diag executable and DayZ Tools on common
Steam library drives. Runtime files and logs are isolated under
`%LOCALAPPDATA%\PaintZSandbox`; the generated mission is staged as
`DayZ\mpmissions\PaintZ_Sandbox.ChernarusPlus`.

## Environment guarantees

- The mission never calls `CreateHive()`, so Central Economy persistence,
  infected, animals, dynamic events, and ambient loot do not start.
- Empty effect-area and underground-trigger configs prevent fallback to the
  terrain's static hazard definitions.
- Both server and client use DayZDiag, so BattlEye is disabled consistently.
- The client uses the existing DayZ profile (`NetCop` by default).
- Weather is locked to clear midday.
- Health, blood, shock, water, and energy are restored every five seconds.
- Every joining player receives two full cans of the first catalogued paint plus two paint strippers. The first paint can starts in hand. Every generated paint can is also staged as a ground fixture, so catalogue growth does not overflow player inventory.
- Spawn and fixtures use NWAF runway concrete at `4475 0 10200`, 105 metres west of the previous grassy spawn. Fixture rows follow the runway northwest.
- Fixtures use the native `PlaceOnSurfaceRotated` transform from the inventory drop path; the launcher no longer forces zero pitch/roll after spawning.
- The shared fixture row is spawned once at mission startup, directly ahead of
  the fixed player spawn. Readiness is reported only after these items exist.

## Default fixtures

The sandbox intentionally uses a curated vanilla set instead of instantiating every
`scope=2` weapon class from the game config. DayZ contains public/internal weapon
entries whose FSMs are incomplete or unsuitable for direct mission spawning in
DayZDiag; blindly constructing them produces unrelated `Virtual Machine Exception`
noise and obscures PaintZ failures.

Current curated weapons:

- `M4A1`
- `AKM`
- `FAL`
- `MP5K`
- `SKS`
- `Mosin9130`
- `SVD`
- `Winchester70`

Current curated magazines:

- `Mag_CMAG_30Rnd_Black`
- `Mag_STANAG_30Rnd`
- `Mag_AKM_30Rnd`
- `Mag_AK74_30Rnd`
- `Mag_FAL_20Rnd`

Every class generated from the current legacy `paints.json` bridge is staged as one
paint can on the ground, together with `PaintZ_PaintStripperCan`.

Paint cans only paint. Hold the separate orange Paint Stripper to remove a finish.
It offers Strip Paint only on items carrying PaintZ state. Paint/stripper quantity
consumption is size-dependent and comes from `paintz_action_tuning.json`; the
sandbox must not assume a fixed per-action quantity cost.

Fixture classnames exist only in this isolated mission template. PaintZ's runtime
paint code remains classname-agnostic. A specific third-party weapon can still be
added explicitly with `-ThirdPartyWeaponClass`.

## Useful options

Run the PowerShell launcher directly for options:

```powershell
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 -Build
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 -Generate
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 -RunSmokeTests -ServerOnly
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 -ServerOnly
```

`-RunSmokeTests` appends the diagnostic smoke suite to the generated mission.
Some checks still exercise the transitional built-in `PaintZ_PaintCatalog` bridge
and will be retired when the Standard Pack migration removes that bridge.

`-SkipBuild` remains accepted for compatibility, but is deprecated because skipping
the build is now the default.

To add one third-party compatibility fixture without changing PaintZ, load the
mod and pass its weapon classname:

```powershell
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 `
  -AdditionalMods 'D:\Workshop\SomeWeaponMod' `
  -ThirdPartyWeaponClass 'SomeWeaponClass'
```

`-AdditionalMods` accepts more than one directory. The supplied classname is
validated and inserted only into the generated test mission, never the PaintZ PBO.

## Paint Pack API v1 conformance fixture

The repository also contains a non-shipping synthetic paint pack under:

```text
tools\sandbox\paint-pack-api-fixture
```

It includes valid and intentionally invalid API-v1 registrations so the runtime
registry can be tested against the actual merged DayZ config tree.

Build it with:

```powershell
.\tools\sandbox\Build-PaintPackApiFixture.ps1
```

Then load the produced mod after PaintZ:

```powershell
$fixture = "$env:LOCALAPPDATA\PaintZSandbox\@PaintZ-PaintPackApiFixture"
pwsh -File .\tools\sandbox\Start-PaintZSandbox.ps1 `
  -AdditionalMods $fixture
```

The fixture runs its own registry and player/application checks. After those checks,
it also leaves a red test can, a pattern test can, and an M4 on the ground near the
player so manual API-v1 painting and stripping can be verified without VPP/Admin tools.

Search the RPT or script log for:

```text
[PaintZ][PackAPI Smoke]
```

Any `FAIL` line is an acceptance failure for the Paint Pack API runtime registry.
See `paint-pack-api-fixture/README.md` for the exact cases covered.