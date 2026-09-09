PaintZ action tuning
====================

Runtime file:
  $profile:PaintZ/paintz_action_tuning.json

The runtime README beside that file is refreshed from the mod on every server start and will be overwritten. The JSON configuration itself is created only when missing and is not overwritten by PaintZ after an administrator edits it.

Purpose
-------
This file controls painting/stripping action time and consumable usage from the target item's physical size. PaintZ reuses the same longest-collision-box-dimension measurement already used by pattern scaling.

There are no discrete size ranges. One linear dependency is used between min_dimension_m and max_dimension_m. Values below the minimum use the minimum endpoint; values above the maximum use the maximum endpoint.

Fields
------
version
  Configuration schema version. Current value: 1.

reload_seconds
  Positive whole number: reload approximately at that interval while the server is running.
  -1: load only at startup.
  0 or values below -1 are treated as -1 with a warning.
  An invalid reload keeps the previous valid configuration active.

min_dimension_m
  Small-item endpoint in meters. Default: 0.2.

max_dimension_m
  Large-item endpoint in meters. Default: 0.8.

min_time_seconds
  Action duration at or below min_dimension_m. Default: 5 seconds.

max_time_seconds
  Action duration at or above max_dimension_m. Default: 20 seconds.

paint_applications_per_full_can_at_max_size
  Number of max-size paint applications a completely full PaintZ spray can should provide. Default: 3.

strip_applications_per_full_can_at_max_size
  Number of max-size strip operations a completely full paint-stripper can should provide. Default: 3.

Timing formula
--------------
Let D be the measured longest collision-box dimension clamped to min_dimension_m..max_dimension_m.

ratio = (D - min_dimension_m) / (max_dimension_m - min_dimension_m)
time  = min_time_seconds + ratio * (max_time_seconds - min_time_seconds)

With the shipped defaults:
  <= 0.2 m -> 5 seconds
     0.5 m -> 12.5 seconds
  >= 0.8 m -> 20 seconds

Consumption formula
-------------------
Paint and stripper consumption are proportional to physical size, not bucketed.

size_fraction = D / max_dimension_m
usage_fraction_of_full_can = size_fraction / applications_per_full_can_at_max_size
usage = applicator.GetQuantityMax() * usage_fraction_of_full_can

Paint and stripper use their own applications-per-full-can setting.

With the shipped defaults:
  <= 0.2 m -> 1/12 of a full can (about 8.33%)
  >= 0.8 m -> 1/3 of a full can (about 33.33%)

A normal full paint can therefore paints approximately three rifle-size objects, while magazine-size objects consume proportionally less.

Failure/fallback behavior
-------------------------
If PaintZ cannot obtain a valid collision-box dimension for a target, it uses max_dimension_m for this calculation. This is deliberately conservative: an unknown-size object never becomes a near-free/instant paint operation.

Multiplayer/reload behavior
---------------------------
The server owns this configuration. Valid reloads are synchronized to connected clients so their continuous-action timing matches the server. Newly connecting clients receive the active settings during connection setup.

A reload affects newly started actions. An action already in progress keeps the duration chosen when that action started; the server still revalidates required quantity when the action completes.
