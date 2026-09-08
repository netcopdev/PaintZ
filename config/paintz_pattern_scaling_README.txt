PaintZ pattern scaling
======================

Runtime file
------------
$profile:PaintZ/paintz_pattern_scaling.json

The server creates the file from the bundled default on first startup and does
not overwrite an existing administrator copy.

Purpose
-------
Patterned finishes use generated texture variants with different pattern
geometry sizes. PaintZ measures the target's longest collision-box dimension,
then selects a configured scale for the finish being applied.

This is an approximation. Physical model size does not reveal UV density, so
two items with similar physical dimensions can still need different-looking
pattern scales.

Only finishes backed by a pattern image use scaling. Solid colors always use
the normal 1x texture.

Supported scales
----------------
0.5, 0.75, 1.0, 1.5, 2.0, 3.0

1.0 is the existing PaintZ pattern size.
2.0 makes the pattern geometry approximately twice as large.
0.5 makes it approximately half as large.

Configuration
-------------
Example:

{
  "version": 1,
  "reload_seconds": 10,
  "enabled": true,
  "default_scale": 1.0,
  "ranges": [
    { "max_dimension_m": 0.20, "scale": 2.0 },
    { "max_dimension_m": 0.40, "scale": 1.5 },
    { "max_dimension_m": 0.80, "scale": 1.0 }
  ]
}

Ranges are evaluated from top to bottom and must be ordered by strictly
increasing max_dimension_m. The first range whose maximum is greater than or
equal to the measured item dimension wins. Items larger than every configured
range use default_scale.

Set ranges to [] to make every patterned item use default_scale.

reload_seconds
--------------
A positive whole number reloads the file periodically at approximately that
interval. Set -1 for startup-only loading. 0 and values below -1 are treated as
-1 with a warning.

A failed reload leaves the previous valid configuration active.

Reload behavior
---------------
Changing this file does NOT rescale already-painted loaded items.

The new mapping is used:
- the next time an item is painted or repainted;
- when persisted paint is restored after a server restart/load.

This is intentional so a live config reload never sweeps through every painted
object on the server.

Persistence
-----------
PaintZ persists only the canonical finish ID, for example PZ-C-WDL. The chosen
scale and derived texture path are never persisted. Scale is recalculated from
the current configuration whenever the persisted finish is restored.

Testing/tuning
--------------
When a patterned finish is applied, PaintZ logs the measured longest collision
dimension and chosen scale. Use those values to tune the ranges, save this
file, wait for its reload interval, then repaint the test item.

Because magazines, suppressors, optics, stocks and other attachments can have
overlapping physical sizes but different UV layouts, a size-only mapping
cannot guarantee identical real-world camouflage geometry on every model.
