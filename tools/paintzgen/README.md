# PaintZ Design 2 generator

Repo-ready generator for the selected **Military Issue** PaintZ spray-can design.

## Product codes

PaintZ now uses developer-controlled product IDs rather than hash IDs.

Format:

`PZ-T-ID`

Examples:

- `PZ-S-WHT` — solid white;
- `PZ-S-FDE` — Flat Dark Earth;
- `PZ-C-UCP` — UCP camouflage;
- `PZ-C-WDL` — Woodland camouflage.

`T` is the type namespace. The final `ID` is normally a short descriptive 3-character suffix, but 2-12 alphanumeric characters are supported.

### Preferred workflow

Write the suffix explicitly when creating a paint:

```json
{
  "id": "WHT",
  "name": "Arctic White",
  "type": "solid",
  "color": "#E7E8E3"
}
```

If `id` is omitted, the generator tries to make a descriptive suggestion from the name. Suggested IDs are marked in `--check` output and should normally be made explicit before release.

There is no hash ID and no ID database/registry.

## Type letters

- `S` — solid
- `C` — camouflage
- `P` — generic pattern
- `M` — metallic
- `R` — rusted / oxidized
- `W` — weathered
- `F` — fluorescent
- `X` — special/custom
- `T` — transparent/tint

## Quick start

```bash
python -m pip install -r requirements.txt
python tools/generate_paints.py --check
python tools/generate_paints.py --clean
```

or use `scripts/generate.sh` / `scripts/generate.ps1`.

## Appearance system

Solid paints use the v1.3 phase-one deterministic finish stack: grain, grime, scratches, optional rust and edge wear. Appearance settings do not affect product IDs.

## Starter catalogue

The manifest includes common military colours and camouflage finishes, including `PZ-S-WHT`, `PZ-S-FDE`, `PZ-C-WDL`, `PZ-C-MCT` and `PZ-C-UCP`.

## Contents

- `paints.json` — paint catalogue and explicit IDs;
- `config/appearance_profiles.json` — solid-finish profiles;
- `tools/generate_paints.py` — CLI generator;
- `tools/paintzgen/` — renderer, ID handling, validation and DayZ output;
- `assets/template/` — Design 2 base artwork;
- `assets/overlays/` — reusable wear overlays;
- `assets/pattern_sources/` — procedural camouflage examples;
- `generated/` — generated labels, previews, catalogue and DayZ fragments;
- `docs/` — detailed documentation.

## Integration boundary

The PaintZ build runs this generator automatically. Every paint is emitted into
the mod config and generated runtime action catalogue, its label is copied to
`data/cans`, and its finish-rendered coating is copied to `data/surfaces`; both are converted
to PAA. `dayz_class` may override the generated classname; otherwise the generator
uses `PaintZ_SprayCan_ID`. See `docs/DAYZ_INTEGRATION.md`.

## Public source checkout

The public repository excludes the binary artwork library and all generated output. Provide the artwork assets locally, then run the generator before building the DayZ package.
