# PaintZ Codex instructions

## Project goal

Build a generic DayZ runtime painting framework. A player holds a finish-specific spray can, points at a weapon or detachable magazine, and the mod determines at runtime whether the actual target model exposes a suitable hidden selection. If it does, offer a paint action. If it does not, offer an informative `Cannot Paint` action/message.

## Non-negotiable architecture constraints

Do **not** solve compatibility by creating or maintaining any of the following:

- painted subclasses such as `AKM_Woodland`, `M4_Green`, etc.
- per-weapon `modded class M4A1_Base`, `modded class FAL_Base`, etc.
- per-weapon texture registration tables
- classname allowlists for supported guns/magazines
- compatibility patches whose only purpose is to name individual weapon classes

The target must be inspected dynamically at interaction time.

Allowed global/base-level hooks:

- `Weapon_Base`
- `Magazine` / `Magazine_Base` as appropriate for the current DayZ script API
- the PaintZ spray-can class
- generic actions and helper classes

## Current scope

The current release supports only these things:

1. `PaintZ_SprayCan_Woodland` can target `Weapon_Base` and detachable magazines.
2. The target's `hiddenSelections[]` are read dynamically from the live object.
3. A global heuristic chooses a plausible body/camo selection without knowing the target classname.
4. A compatible item exposes `Paint Woodland`.
5. An incompatible weapon/magazine exposes `Cannot Paint` and informs the player why.
6. Painting changes the existing object instance with `SetObjectTexture()`; the classname does not change.
7. The selected finish/selection is synchronized to clients for the current server session.
8. Debug logging reports target type, discovered selections, selected selection, and rejection reason.

Do **not** expand the current scope into an RGB picker, attachments, texture extraction, or persistence redesign unless explicitly asked.

## Persistence warning

The old Reskin Manager appends custom data from a broad base-class `OnStoreSave()` / `OnStoreLoad()` override. Do not copy that pattern blindly. It can be unsafe when installing onto an existing persistence database and can conflict with subclass/mod serialization order.

Persistence is deliberately not implemented. Before adding it:

- research current DayZ persistence extension patterns;
- consider install-on-existing-server compatibility;
- test save created before PaintZ is installed;
- test save created with PaintZ then loaded after PaintZ removal;
- test interaction with other mods overriding persistence on weapon/magazine classes.

Document the chosen persistence mechanism before coding it.

## Hidden-selection policy

Use the target object's runtime `GetHiddenSelections()` / `GetHiddenSelectionIndex()` data.

Selection heuristic:

- Prefer globally known body-like names such as `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag`, `magazine`.
- Reject obvious non-body selections such as glass, lens, reticle, optic, display, screen, LED, light, emissive, glow, flame.
- If there is exactly one hidden selection and it is not blocked, it may be accepted as a conservative fallback.
- If multiple ambiguous selections remain, fail safely and report that no safe paint selection could be identified.

This is a global heuristic, not a per-item database.

## Source/API discipline

Do not invent DayZ APIs. When uncertain, inspect current public sources first, especially:

- BohemiaInteractive/DayZ-Script-Diff
- the historical Dumpgrah/Unoffical-DayZ-Reskin-Manger-Mod only as a behavioral/reference example

Do not copy the historical mod's per-weapon registration architecture.

## Coding style

- Keep helpers small and auditable.
- Prefer explicit names over clever abstractions.
- Log decisions in debug code.
- Avoid unnecessary dependencies.
- Do not introduce unnecessary framework layers.
- Preserve original classname, health, ammo, attachments, inventory location, and object identity by never replacing the target object.

## Before claiming success

Verify against `docs/RELEASE_ACCEPTANCE.md` and report any engine/API limitation rather than working around it with per-weapon definitions.
