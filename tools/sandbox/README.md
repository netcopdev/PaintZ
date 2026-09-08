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

| Fixture | Expected behavior |
|---|---|
| `M4A1` | `Paint Woodland` (`camo`) |
| `Mag_AKM_30Rnd` | `Paint Woodland` (`camo`) |
| `Mag_STANAG_30Rnd` | `Cannot Paint` (no suitable hidden selection) |
| `Ammo_556x45` | No PaintZ action; loose ammo is deliberately out of scope |
| Every class generated from `paints.json` | One paint can on the ground |
| `PaintZ_PaintStripperCan` | Orange can; the only can that strips paint |

Paint cans only paint. Hold the separate orange Paint Stripper to remove a finish.
It offers Strip Paint only on painted compatible items and consumes 10 quantity
per completed strip. Untouched and already stripped items offer no Strip Paint.
The sandbox enumerates public vanilla weapons and magazines dynamically, so
the rows above are examples rather than a complete fixture list.

Fixture classnames exist only in this isolated mission template. PaintZ's runtime
paint code remains classname-agnostic.

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
Server startup runs catalogue, wildcard, ordered-rule, type-filter, inheritance,
and existing-paint/stripping policy checks. When a player joins, the suite also
runs completion-consumption and policy-reload race checks.

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
