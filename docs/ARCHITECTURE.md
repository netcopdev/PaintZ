# Architecture

## Principle

PaintZ must discover compatibility from the live target object, not from a maintained list of weapon classnames.

## Runtime flow

1. Spray-can action receives an `ActionTarget`.
2. Cached target-domain rules decide whether PaintZ should offer new-paint feedback.
3. The cached item policy independently decides whether a new application is allowed.
4. `PaintZ_PaintInspector` reads the runtime object's hidden selections.
5. Global selection-name heuristics independently determine technical capability.
6. The server re-runs domain, can, target, policy, and capability checks at completion.
7. Painting stores the `PZ-T-ID` product code as its session identity and calls `SetObjectTexture()` on the selected hidden-selection index.
8. The server sends that product code and selection index to connected clients, which reapply the same finish-rendered surface asset.

## Runtime item policy

`$profile:PaintZ/paintz_items.json` is an administrative policy for new paint
applications and repainting. It is loaded atomically from PaintZ's perspective:
a detached candidate is parsed and validated before replacing the active cached
policy. Failed periodic reloads retain the last-known-good policy. The runtime
JSON contains only configuration data; PaintZ creates the non-overwritten
`paintz_items_README.txt` beside it for operational help. The complete contract
is documented in `docs/item-policy.md`.

The policy is deliberately separate from paint state. Excluding a class never
strips or resets existing paint, and stripping bypasses the policy. Ordered
rules use `include`/`exclude`, `weapon`/`magazine`/`all`, and exactly one of a
case-insensitive glob `class_pattern` or runtime config-hierarchy `inherits`
selector. The last matching rule wins over `default_action`.

The optional `domains` array is a positive OR-list for new-paint interaction
scope. Each entry may contain a DayZ config `type`, a case-insensitive
`class_pattern`, or both (AND). Missing or empty domains use the current weapon
and detachable-magazine defaults; a valid nonmatching rule may intentionally
disable new-paint scope. Domain membership does not imply policy eligibility or
technical capability.

Stripping locates existing PaintZ-painted state directly and never consults
current domains, policy, or the current selection heuristic.

`ItemBase` carries PaintZ's reusable logical state and network fields so future
paintable inventory categories (for example grips, handguards, suppressors, and
clothing) can share one representation. Persistence is deliberately attached
only to the entity hierarchies PaintZ currently supports: `Weapon_Base` and the
separate detachable-magazine hierarchy at `MagazineStorage`. (`Magazine` itself
is an engine class and cannot be modded.) Adding a new target category requires
an explicit persistence-hook review; it must not be assumed to inherit either
current hook.

Native save hooks append a self-identifying PaintZ block after vanilla state.
`OnStoreLoad` restores only the logical finish ID. `AfterStoreLoad` performs a
one-shot runtime selection inspection and visual application, without applying
new-paint policy. The server then dirties the derived network hash and selection
fields, allowing already-connected and late-joining clients to resolve the
canonical finish from the generated catalogue. See `PERSISTENCE_NOTES.md`.

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
