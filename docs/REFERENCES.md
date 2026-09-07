# References for Codex

Use these as technical references, not as architecture to copy wholesale.

## Current public DayZ script source diff

Repository:

`https://github.com/BohemiaInteractive/DayZ-Script-Diff`

Relevant APIs/classes to verify:

- `EntityAI.GetHiddenSelections()`
- `EntityAI.GetHiddenSelectionIndex()`
- `EntityAI.SetObjectTexture()`
- `EntityAI.GetObjectTexture()` (current applied texture, used to gate Strip Paint)
- `Weapon_Base`
- `Magazine` / `Magazine_Base` typedef
- `RegisterNetSyncVariableInt()`
- `SetSynchDirty()`
- `OnVariablesSynchronized()`
- action framework (`ActionContinuousBase`, `ActionSingleUseBase`, `CCTCursor`)

The installed DayZ 1.29 `dta/scripts.pbo`, `3_game/entities/entityai.c`,
declares `GetObjectTexture(int index)` alongside `SetObjectTexture()`.
`GetHiddenSelectionsTextures()` instead returns config defaults; see
[Bohemia's Object source](https://github.com/BohemiaInteractive/DayZ-Script-Diff/blob/main/scripts/3_game/entities/object.c).

## Historical runtime re-skin example

Repository:

`https://github.com/Dumpgrah/Unoffical-DayZ-Reskin-Manger-Mod`

Useful concepts:

- runtime `SetObjectTexture()`;
- preserve original weapon classname;
- per-instance paint state;
- action-driven spraying.

Do **not** copy:

- per-weapon finish registration;
- hardcoded hidden-selection index `0`;
- its synchronization logic without auditing it;
- its broad persistence serialization pattern without migration/removal testing.

License note: that historical repository states AGPL-3.0. PaintZ is MIT and intentionally contains original implementation code rather than copied AGPL code.
