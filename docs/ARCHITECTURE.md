# Architecture

## Principle

PaintZ must discover compatibility from the live target object, not from a maintained list of weapon classnames.

## Runtime flow

1. Spray-can action receives an `ActionTarget`.
2. Target must be a weapon or detachable magazine.
3. `PaintZ_PaintInspector` reads `target.GetHiddenSelections()`.
4. Global selection-name heuristics choose a likely body/camo selection.
5. If no safe choice exists, the target is rejected and the player is informed.
6. Painting stores the `PZ-T-ID` product code as its session identity and calls `SetObjectTexture()` on the selected hidden-selection index.
7. The server sends that product code and selection index to connected clients, which reapply the same finish-rendered surface asset.

## Why no per-weapon definitions

The historical Reskin Manager proves that runtime texture replacement can preserve the object's classname, but it still registers finishes in `modded class <SpecificWeapon>` blocks. PaintZ explicitly rejects that architecture.

A newly installed third-party weapon mod should require zero PaintZ code changes when its model exposes a recognizable/safe hidden selection.

## Selection heuristic

The selection policy is intentionally conservative:

- exact body-like names are preferred;
- obvious non-body visuals are blocked;
- a single non-blocked hidden selection can be accepted as a fallback;
- multiple ambiguous selections are rejected.

False negatives are preferable to painting a scope lens, LED, screen, or another inappropriate surface.

Later versions may improve the heuristic globally (scoring, material/texture inspection, model metadata), but should not fall back to per-class compatibility tables.

## Texture strategy

Can labels live under `data/cans` and are used only by can models. Painted objects use separate finish-rendered color/pattern PAAs under `data/surfaces`; these assets contain no label typography, borders, or can design. The target's existing UV layout controls a pattern's scale and distortion.

Future rendering research should compare:

- one generic tiled/pattern texture across unrelated weapon UVs;
- source textures/pattern extraction;
- keeping target material while replacing only color texture;
- whether material overrides improve scale/appearance without target-specific assets.

Do not assume an atlas from one weapon will map correctly onto another weapon.
