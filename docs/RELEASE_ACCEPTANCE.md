# Release acceptance criteria

A release is accepted only when all mandatory checks pass.

## Build / runtime

- [ ] Mod compiles against the target DayZ version with Community Framework loaded.
- [ ] Paint cans spawn and can be held.
- [ ] A configured target with a safe hidden selection offers Paint.
- [ ] An unsafe/ambiguous configured target fails with a useful reason.
- [ ] Painting modifies the existing object; classname and identity remain unchanged.
- [ ] Health, ammunition, chamber state, attachments, cargo and inventory relationships are preserved.
- [ ] Another connected player sees the painted finish.

## Paint Pack API / official namespace

- [ ] PaintZ core declares exactly one official `PZ` namespace owner: `PZ_PaintZOfficial`.
- [ ] PaintZ core contains no individual official finish catalogue, finish-specific spray cans, or official finish assets.
- [ ] PaintZ starts and functions with no official content pack installed.
- [ ] An official PackKit-generated content pack emits no `CfgPaintZPacks` declaration for `PZ`.
- [ ] Every official `PZ-*` finish registration references `owner = "PZ_PaintZOfficial"`.
- [ ] An official content pack depends directly on `PaintZ_DynamicPaint` and not on another official content pack merely for namespace access.
- [ ] Two independent official content packs can contribute distinct `PZ-*` finishes simultaneously.
- [ ] A deliberate duplicate complete `PZ-*` finish ID is disabled rather than overwritten by load order.
- [ ] A content pack attempting to redeclare `PZ` conflicts with the core owner and does not replace it.
- [ ] Unassigned reserved `PZ?` namespaces remain unavailable unless explicitly assigned by a future API decision.
- [ ] Moving an unchanged official finish between official content packs preserves its complete `PZ-*` ID and requires no persistence alias/migration.

## Universal architecture

- [ ] PaintZ state, synchronization and persistence are implemented once on the shared `ItemBase` path.
- [ ] No PaintZ weapon-specific persistence hook exists.
- [ ] No PaintZ magazine-specific persistence hook exists.
- [ ] No PaintZ attachment/optic/light-specific persistence hook exists.
- [ ] `PaintZ_PaintTarget` does not dispatch paint state by category.
- [ ] Policy code does not use a closed weapon/magazine/attachment enum as the extensibility mechanism.
- [ ] A non-weapon/non-magazine `ItemBase` family can be enabled by JSON only and receives the same state/persistence behavior without script changes.
- [ ] No per-class compatibility registry or painted subclasses exist.

## Runtime policy

- [ ] Missing runtime JSON is created from the bundled default; existing admin files are not overwritten.
- [ ] Domains support arbitrary valid DayZ base/config classes without code changes.
- [ ] Domains support exact `inventory_slot` and wildcard `inventory_slot_pattern` selectors.
- [ ] Slot matching reads the target class's declared compatible `inventorySlot` values, not its current attachment position.
- [ ] A loose stock/handguard/suppressor/optic/flashlight can match its declared slot family while on the ground.
- [ ] Domain fields are AND and separate domain entries are OR.
- [ ] `type` in rules accepts arbitrary valid DayZ base/config classes.
- [ ] Rules support the documented singular/plural selector families and multi-condition matching semantics.
- [ ] Legacy version-1 `weapon` / `magazine` aliases remain accepted.
- [ ] Omitted rule `type` behaves as `all`.
- [ ] Ordered rules remain last-match-wins.
- [ ] `*` and `?` matching is case-insensitive and deterministic for both class and slot patterns.
- [ ] Failed reload retains the last-known-good policy.
- [ ] Excluding or removing a domain from an already-painted item does not remove its finish or block stripping.
- [ ] The bundled default includes weapon/magazine domains, common weapon/pistol/suppressor slot families and `SmallProtectorCase`, with no blanket crossbow exclusion.

## CF persistence

- [ ] `CfgMods.PaintZ.storageVersion` is positive and CF is a declared dependency.
- [ ] Painted weapon survives restart.
- [ ] Painted magazine survives restart with ammunition intact.
- [ ] A painted non-weapon/non-magazine `ItemBase` fixture survives restart using the same persistence path.
- [ ] Painted items survive in player inventory, persistent/nested storage and vehicle cargo.
- [ ] Repaint A -> B persists B.
- [ ] Strip -> restart remains unpainted.
- [ ] Runtime policy exclusion does not prevent restoration.
- [ ] Missing finish definition preserves the logical ID without implicit strip.
- [ ] Pre-PaintZ/unpainted items load normally with no PaintZ payload required.
- [ ] Late-joining client sees restored finish.
- [ ] Save with PaintZ removed but CF retained, then restore PaintZ: PaintZ state returns.
- [ ] Removing CF as well is documented as outside the persistence guarantee.

## Selection / strip regression

- [ ] Known-good weapon and magazine fixtures still paint and strip.
- [ ] At least one configured attachment and `SmallProtectorCase` paint and strip without category-specific PaintZ code.
- [ ] At least one optic or flashlight with a safe body/camo/housing selection can paint.
- [ ] An optic/light exposing only functional glass/lens/reticle/display/emissive surfaces remains safely unsupported.
- [ ] Category words such as `optic` or `light` alone do not blacklist an otherwise safe body selection.
- [ ] Paint cans never offer Strip Paint; the dedicated stripper does.
- [ ] Ruined cans and ruined targets cannot receive new paint.

`tools/sandbox/PaintZ_FinishSmokeTest.c` and the Paint Pack API fixture remain opt-in smoke tests. Multiplayer UI/visual replication and persistence restart scenarios require live-server acceptance testing.
