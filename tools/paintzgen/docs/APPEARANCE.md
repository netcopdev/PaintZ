# Phase-one appearance system

The goal is to stop solid paints from looking like flat digital rectangles while keeping the production pipeline deterministic and fully automatic.

## Scope

This package currently implements **phase one only**:

- one generated visual result per paint;
- deterministic output based on paint code;
- no runtime randomization;
- no per-instance variation inside DayZ;
- no manual paint-by-paint editing.

## How solid finishes are built

For every solid paint, the swatch panel is generated as:

1. base colour fill;
2. procedural fine grain/noise;
3. grime/spot overlay;
4. scratch/scuff overlay;
5. optional rust overlay;
6. edge-wear overlay.

The overlay choice and its effective strength vary deterministically from a seed derived from the generated PaintZ code. That means:

- the same paint always regenerates the same finish;
- different paints generally get different wear placement/intensity;
- there is still no registry or external state.

## Overlay assets

Reusable graphical assets live in `assets/overlays/`:

- `grime_*.png`
- `scratches_*.png`
- `rust_*.png`
- `edgewear_01.png`

These are not tied to one specific paint. They are reusable building blocks for the generator.

## Appearance profiles

The solid-finish system is driven by `config/appearance_profiles.json`.

Included profiles:

- `clean`
- `used`
- `weathered`
- `rusted`

Each profile controls scalar intensities for:

- `noise`
- `scratches`
- `grime`
- `rust`
- `edgewear`

A paint opts into one profile with:

```json
{
  "id": "ODG",
  "name": "Olive Drab",
  "type": "solid",
  "color": "#556B2F",
  "appearance_profile": "used"
}
```

If omitted, the default profile from `config/appearance_profiles.json` is used.

## Important identity rule

Appearance profiles and overlay artwork **do not participate in ID generation**.

Changing:

- `appearance_profile`
- scratch overlays
- grime overlays
- rust overlays
- compositing strengths

must not renumber the paint.

## Future expansion

A later phase can generate multiple deterministic variants per paint or allow DayZ-side random selection among pre-generated finish variants. That is intentionally excluded from this version to keep the system simple and repo-ready.
