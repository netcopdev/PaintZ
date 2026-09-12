# Painted item presentation test

This branch ports the painted-item presentation fix onto current `main`.

## Expected behavior

For any painted item, PaintZ decorates the final DayZ presentation result rather than the lower-level `NameOverride()` / `DescriptionOverride()` hooks directly.

Example:

```text
KA-74 [Flecktarn]
```

The tooltip/description also includes:

```text
Finish: Flecktarn (PZ-C-FTN)
```

This must work through the shared `ItemBase` path for weapons, magazines, and other eligible inventory items without weapon/class-specific patches.

Vicinity container headers must also refresh when PaintZ changes the effective display name. A weapon must not need to pass through player inventory/hands before the vicinity header reflects paint, repaint, or stripping.

## Live acceptance

1. Build this branch with the normal PaintZ release workflow and load the same content packs used for the current server test.
2. Paint a vanilla weapon such as KA-74 while it remains in vicinity. Confirm its inventory/inspect name immediately gains ` [finish name]` and its existing vicinity object header updates without first moving the weapon through the player.
3. Inspect the weapon. Confirm the normal description remains intact and the `Finish: name (ID)` line appears.
4. Repaint the same weapon while it remains in vicinity. Confirm the existing vicinity header changes to the new finish without inventory movement.
5. Paint a detachable magazine and one non-weapon `ItemBase` target. Confirm the same naming/tooltip behavior.
6. Move the painted weapon between hands, inventory, ground, and storage. Confirm the suffix remains visible wherever DayZ uses `GetDisplayName()`.
7. Reconnect or use a second client. Confirm the synchronized painted weapon shows the suffix and finish line without repainting.
8. Restart the server with the painted weapon persisted. Confirm the restored item still shows the suffix and finish line.
9. Strip the weapon while it remains in vicinity. Confirm the normal upstream name/description and the existing vicinity header are restored without moving the weapon through the player.
10. Check the script log for compile/runtime errors related to `GetDisplayName`, `GetTooltip`, PaintZ item state, vicinity headers, or synchronization.

Do not merge until the weapon case is confirmed live. The generic magazine/non-weapon checks are regression coverage for the shared `ItemBase` implementation.
