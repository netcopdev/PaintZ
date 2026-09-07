# DayZ integration notes

The generator deliberately separates **PaintZ identity/data generation** from the exact stock-can UV implementation.

Current DayZ scripting exposes `Spraycan_ColorBase`; vanilla painting recipes also accept that base class. The generated config therefore inherits from `Spraycan_ColorBase` by default. However, whether a particular stock model texture can be replaced directly depends on the model/config hidden selections used by the PaintZ implementation.

## Generated config fragment

`generated/dayz/PaintZ_Paints.generated.inc` contains one class per manifest paint.
An explicit `dayz_class` is used as-is; otherwise the generated classname follows
`PaintZ_SprayCan_ID`. The texture assignment is controlled by
`paints.json`:

```json
"texture_assignment": "hiddenSelectionsTextures",
"hidden_selection_repeat": 1
```

If the existing PaintZ can implementation uses a different property, base class, number of selections, or custom scripted material swap, change these generator settings/code once. Do **not** patch individual generated classes.

## Stock texture / UV composition

The included artwork is a flat label master. If PaintZ already has a stock-can `_co` texture/UV template, the recommended next integration is to make it an input to this generator and composite the label into one configured rectangle/mask. That source texture should come from the project's lawful DayZ modding workflow; this package does not redistribute vanilla DayZ texture assets.

The same principle applies: one UV template + one mask + generated label content. New paints remain zero-touch.

## PAA

DayZ ultimately uses `.paa` textures. The generator emits lossless PNG source files for both can labels and clean painted surfaces because PAA conversion is a build-tool concern and DayZ Tools is not available on every machine. `scripts/generate.ps1` optionally calls a locally installed `ImageToPAA.exe` when `PAINTZ_IMAGE_TO_PAA` is set.

## PaintZ repo integration

The root `config.cpp` includes the generated fragment. `tools/build.ps1` calls
`tools/generate-paints.ps1` before AddonBuilder; that wrapper renders the catalogue,
copies labels to `data/cans` and finish-rendered coatings to `data/surfaces`, then converts
both sets with ImageToPAA.
Pass `-SkipPaintZGen` only when diagnosing a build with already-generated outputs.
Keep `paints.json` as the hand-maintained paint catalogue.

The same generation pass also writes:

- `PaintZ_Units.generated.inc` for `CfgPatches.units[]`;
- `PaintZ_PaintCatalog.generated.c` for action classes, registration, finish names,
  and paint-code/can-class enumeration used by runtime code and smoke tests;
- `types.generated.xml` for Central Economy integration.

These outputs are committed so a checkout remains buildable, but they must never
be edited by hand. Adding or removing a paint requires only editing `paints.json`
and rerunning the root generation/build command.
