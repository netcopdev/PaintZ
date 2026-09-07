# Design 2: Military Issue

This is the locked PaintZ can identity selected for production.

The label is intentionally divided into **immutable** and **dynamic** content.

## Immutable

- olive/khaki military body;
- dark steel top and bottom bands;
- PaintZ logo with red `Z`;
- `MILITARY ISSUE / SPRAY COATING` series marks;
- technical rules, border, warning copy and general weathering/grain;
- all positions, margins and typographic hierarchy.

## Dynamic

- one fixed-size colour/pattern swatch;
- paint name;
- developer-controlled stock code (`PZ-T-ID`), normally using a short descriptive suffix such as `WHT`, `FDE` or `UCP`;
- finish line;
- swatch/source detail while identity remains independent of the artwork.

The renderer never moves elements for a specific paint. Long names are handled only by reducing the font size inside the same fixed width.

Coordinates are machine-readable in `assets/template/template_spec.json`.

## Reproducibility

The example PNG files in `generated/` are generated artifacts, not editable masters. Change `paints.json` or generator code and regenerate everything.

Typography uses the first supported font found on the build host. For pixel-identical CI output across Windows/Linux, set `PAINTZ_FONT` to the same locally installed bold condensed font on every builder. **No font file is bundled in this package.**
