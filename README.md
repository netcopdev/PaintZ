# PaintZ

PaintZ is a DayZ runtime painting mod. Its central design rule is that compatible weapons and detachable magazines become paintable automatically based on the target model's runtime hidden selections — without a class or compatibility entry for every weapon.

## Repository status

PaintZ is prepared as a production mod. The finish changes have been packaged with DayZ Tools and compiled in the installed DayZ 1.29 diagnostic server. Complete the release acceptance checks, including in-game client and multiplayer review, before publishing an updated build.

Every finish in `tools/paintzgen/paints.json` is available through the same runtime inspection and paint actions. Only the dedicated `PaintZ_PaintStripperCan` offers Strip Paint, and only when the selected surface currently carries a PaintZ finish.

Painted weapons and detachable magazines persist through native DayZ entity
save/load hooks; Community Framework is not required. Keep PaintZ loaded for
every persistence save cycle where paint preservation matters. See
`docs/PERSISTENCE_NOTES.md` for the accepted removal limitation and restart
test procedure.

Paint applies at full opacity and another paint can can overwrite the finish.

Server administrators can control both new-paint target domains and application
eligibility with
`$profile:PaintZ/paintz_items.json`. PaintZ creates that file from the bundled
`config/paintz_items.default.json` template on first startup and never
overwrites an existing administrator copy. It also creates
`$profile:PaintZ/paintz_items_README.txt` without overwriting an existing file.
The JSON contains configuration only; see the adjacent operational README and
`docs/item-policy.md` for reload, wildcard, inheritance, precedence, and failure
semantics. Objects outside all domains are silent for new painting. Excluding a
class keeps it domain-relevant for an explanatory action, never removes an
existing finish, and never blocks Strip Paint.
Five-coat blending and color mixing
are not implemented. See `docs/PAINT_COATS_FEASIBILITY.md` for the rendering limitation.

## Intended player flow

1. Player holds `PaintZ_SprayCan_WDL`.
2. Player points at an object inside a configured PaintZ target domain.
3. PaintZ evaluates can state, target state, eligibility policy, then capability.
4. PaintZ reads that object's `hiddenSelections[]` at runtime only when needed.
5. A safe allowed target shows `Paint Woodland`; failures show a specific reason.
6. Painting calls `SetObjectTexture()` on the existing object. No item replacement and no classname change.

## Start with Codex

Open the extracted folder as the repository/project and tell Codex to read `AGENTS.md` first. A ready-to-use initial task is in:

`docs/CODEX_START_PROMPT.md`

Recommended Git initialization:

```powershell
git init -b main
git add .
git commit -m "Initial PaintZ dynamic paint mod"
```

Then create/push the remote as usual.

## Project layout

- `config.cpp` — mod registration plus generated paint declarations and the separate stripper can.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintInspector.c` — runtime hidden-selection inspection.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintVisuals.c` — texture application.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintTarget.c` — generic paint/strip dispatch.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintedState.c` — existing-paint lookup independent from domains/policy.
- `Scripts/4_World/PaintZ/Paint/PaintZ_ItemPaintState.c` — reusable logical paint state and network synchronization for present and future inventory-item targets.
- `Scripts/4_World/PaintZ/Paint/PaintZ_WeaponPaintPersistence.c` — native weapon persistence hook.
- `Scripts/4_World/PaintZ/Paint/PaintZ_MagazinePaintState.c` — native detachable-magazine persistence hook.
- `Scripts/4_World/PaintZ/Paint/PaintZ_PaintPersistence.c` — shared versioned persistence codec and post-load visual restoration.
- `Scripts/4_World/PaintZ/Actions/` — shared Paint / Cannot Paint action behavior.
- `Scripts/4_World/PaintZ/Items/PaintZ_SprayCan.c` — shared spray-can behavior.
- `docs/` — architecture, acceptance criteria, persistence cautions, references.

## Texture note

Can models use label artwork from `data/cans`. Painted surfaces use clean generated
color or pattern assets from `data/surfaces`. Pattern scale and distortion depend
on each target model's UV layout. Edit the paint catalogue or paintzgen sources and
rerun `tools/generate-paints.ps1`; do not hand-edit generated runtime textures.

## Generated spray cans

`tools/paintzgen/paints.json` is the single hand-maintained paint catalogue.
Each entry receives a stable ID-based can class unless `dayz_class` overrides it,
plus a matching action, action-menu label, runtime display-name mapping,
`CfgPatches.units[]` entry, economy type entry, can label, and surface texture.
A normal `tools/build.ps1` run regenerates the labels in `data/cans` and clean
coating-only textures in `data/surfaces`, converts both sets to PAA, and then runs
AddonBuilder. Removed catalogue entries are removed from generated outputs on the
next generation/build. Do not hand-edit files under `tools/paintzgen/generated`.

## Build

The exact AddonBuilder invocation varies by local DayZ Tools setup. `include.lst` is provided, along with a minimal PowerShell helper in `tools/build.ps1`. Review the tool paths/arguments before first use.

## Public source repository

This repository contains source code, documentation, and build tooling only. Generated paint assets, packaged PBOs, local server profiles, editor settings, credentials, and artwork/font source files are intentionally excluded. Supply the private artwork library locally before generating textures and building a distributable package.

## License

MIT for original PaintZ code. Do not copy AGPL code from the historical Reskin Manager into this repository unless you intentionally change licensing/redistribution strategy accordingly.
