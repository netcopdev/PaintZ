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

## Universal architecture

- [ ] PaintZ state, synchronization and persistence are implemented once on the shared `ItemBase` path.
- [ ] No PaintZ weapon-specific persistence hook exists.
- [ ] No PaintZ magazine-specific persistence hook exists.
- [ ] `PaintZ_PaintTarget` does not dispatch paint state by weapon/magazine category.
- [ ] Policy code does not use a closed weapon/magazine enum as the extensibility mechanism.
- [ ] A non-weapon/non-magazine `ItemBase` family can be enabled by JSON only and receives the same state/persistence behavior without script changes.
- [ ] No per-class compatibility registry or painted subclasses exist.

## Runtime policy

- [ ] Missing runtime JSON is created from the bundled default; existing admin files are not overwritten.
- [ ] Default domains are weapon + detachable magazine, but additional valid DayZ base classes work without code changes.
- [ ] `type` in rules accepts arbitrary valid DayZ base/config classes.
- [ ] Legacy version-1 `weapon` / `magazine` aliases remain accepted.
- [ ] Omitted rule `type` behaves as `all`.
- [ ] Ordered rules remain last-match-wins.
- [ ] `*` and `?` matching is case-insensitive and deterministic.
- [ ] Failed reload retains the last-known-good policy.
- [ ] Excluding or removing a domain from an already-painted item does not remove its finish or block stripping.

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
- [ ] At least one configured attachment/clothing/container fixture paints and strips without any category-specific PaintZ code.
- [ ] Optics/lens/display/light selections remain blocked by the global safety heuristic.
- [ ] Paint cans never offer Strip Paint; the dedicated stripper does.
- [ ] Ruined cans and ruined targets cannot receive new paint.

`tools/sandbox/PaintZ_FinishSmokeTest.c` remains an opt-in server smoke test. Multiplayer UI/visual replication and persistence restart scenarios require live-server acceptance testing.
