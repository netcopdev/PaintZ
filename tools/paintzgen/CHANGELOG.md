# Changelog

## 1.5.0

- Added generated pattern-scale surface variants configured by `generator.pattern_scales`.
- Kept the existing 1x surface filename stable; non-1x variants use `_sNNN` suffixes such as `_s050` and `_s200`.
- Added generated runtime catalogue metadata for pattern finishes and supported scale percentages.
- Pattern scaling changes camouflage geometry before wear overlays are applied, so scratches/grime are not enlarged with the pattern.
- Added per-paint surface-variant metadata to `generated/catalog.json` for the DayZ export/build pipeline.

## 1.4.0

- Removed hash-derived PaintZ IDs entirely.
- Product codes now use developer-controlled descriptive suffixes: `PZ-T-ID`.
- Added preferred explicit `id` field to `paints.json`.
- Added deterministic name-based suggestion fallback when `id` is omitted; suggestions are not hashes and are flagged for review.
- Added duplicate full-code validation.
- Added dedicated camouflage namespace `C`; common military camouflage now uses codes such as `PZ-C-UCP` and `PZ-C-WDL`.
- Retained `P` for generic non-camouflage patterns and moved special/custom to `X`.
- Added a procedural UCP starter pattern.
- Regenerated all assets, previews, DayZ fragments and documentation with the new IDs.

## 1.3.0

- Added phase-one solid-finish rendering: deterministic grain, grime, scratch, optional rust and edge-wear compositing.
- Added reusable overlay assets in `assets/overlays/`.
- Added `config/appearance_profiles.json` with `clean`, `used`, `weathered` and `rusted` profiles.
- Expanded the starter military catalogue.

## 1.2.0

- Introduced the now-retired short hash ID scheme.

## 1.1.0

- Pattern identity derived from canonical pattern name instead of image contents.

## 1.0.0

- Initial Design 2 Military Issue generator.
