# PaintZ product IDs

## Format

`PZ-T-ID`

Examples:

- `PZ-S-WHT`
- `PZ-S-FDE`
- `PZ-C-UCP`
- `PZ-C-WDL`

The suffix is intentionally human-selected and descriptive rather than derived from a hash.

## Type codes

- `S` — solid
- `C` — camouflage
- `P` — generic pattern
- `M` — metallic
- `R` — rusted / oxidized
- `W` — weathered
- `F` — fluorescent
- `X` — special/custom
- `T` — transparent/tint

## Explicit ID is preferred

A paint should normally declare its suffix directly:

```json
{
  "id": "FDE",
  "name": "Flat Dark Earth",
  "type": "solid",
  "color": "#7B6647"
}
```

The full code becomes `PZ-S-FDE`.

Three characters are preferred for readability on the can, but suffixes from 2 to 12 letters/digits are accepted.

## Automatic suggestion fallback

If `id` is omitted, the generator derives a short descriptive suggestion from the display name. It does not hash the name and does not maintain any registry.

Examples:

- `White` -> `WHT`
- `Black` -> `BLK`
- `Flat Dark Earth` -> `FDE`
- `Universal Camouflage Pattern` -> `UCP`

This fallback is a convenience for development. `--check` marks suggested IDs so a developer can review and commit an explicit value before release.

## Stability

Once an explicit suffix is assigned, it is the product identity. Changing RGB values, texture artwork, wear profiles or label artwork does not change the code.

Changing `id` or `type` changes the public product code intentionally.

## Duplicate policy

The generator validates the complete catalogue and refuses to build if two paints produce the same full code. Resolve the conflict by choosing a different explicit suffix for one paint.

No external database is involved.
