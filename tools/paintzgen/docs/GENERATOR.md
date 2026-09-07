# Generator usage

## Install and run

```bash
python -m pip install -r requirements.txt
python tools/generate_paints.py --check
python tools/generate_paints.py --clean
```

Linux/Fedora and PowerShell wrappers are also provided under `scripts/`.

## Add a solid colour

Preferred form:

```json
{
  "id": "FDE",
  "name": "Flat Dark Earth",
  "type": "solid",
  "color": "#7B6647",
  "appearance_profile": "used"
}
```

This generates `PZ-S-FDE`.

Changing only `color` or `appearance_profile` later leaves the product code unchanged.

## Add camouflage

```json
{
  "id": "UCP",
  "name": "Universal Camouflage Pattern",
  "type": "camo",
  "pattern": "assets/pattern_sources/ucp_procedural.png"
}
```

This generates `PZ-C-UCP`.

Changing or replacing the source image does not change the product code.

## Let the generator suggest an ID

You may omit `id` temporarily:

```json
{
  "name": "Desert Tan",
  "type": "solid",
  "color": "#B49A72"
}
```

The generator currently suggests `DTN`, producing `PZ-S-DTN`. `--check` reports it as `SUGGESTED`. Review it and add the explicit `id` before release.

## Appearance profiles

Every rendered surface uses its `clean`, `used`, `weathered` or `rusted` appearance profile from `config/appearance_profiles.json`. The configured grain, grime, scratches, optional rust, and edge wear are baked into both solid and pattern/camouflage surface textures. Can-label swatches retain their existing solid-finish rendering behavior.

## Outputs

- `generated/labels/*_co.png`
- `generated/surfaces/*_co.png`
- `generated/previews/*_preview.png`
- `generated/preview_catalog.png`
- `generated/catalog.json`
- `generated/dayz/PaintZ_Paints.generated.inc`
- `generated/dayz/types.generated.xml`

All paints are written to the DayZ config and types outputs. An explicit
`dayz_class` is used as-is; otherwise the generated classname follows
`PaintZ_SprayCan_ID`.

Generated files are overwritten on regeneration.

## Duplicate IDs

Duplicate full product codes are fatal. If two paints both resolve to `PZ-S-GRN`, generation stops and asks for a different explicit `id` on one of them.
