# Release acceptance criteria

A release is accepted only when all mandatory checks pass.

## Repository reconciliation

- [ ] An outstanding-work audit has been performed according to `docs/DEFERRED_WORK_TRACKING.md`.
- [ ] Every relevant branch with commits absent from `main` has an explicit disposition: integrated/equivalent, intentionally pending with an active tracker, approved for integration, superseded, rejected/obsolete, or experimental.
- [ ] Every deferred fix/feature awaiting verification has an open PR or issue recording its branch/head SHA, verification state, blocker, next action, and integration criteria.
- [ ] No verified and approved relevant fix remains only on another branch without a newly recorded blocker.
- [ ] Open PRs/issues and release-acceptance requirements do not identify a known unresolved defect on `main` that is being ignored for release.
- [ ] No branch is deleted or broad cleanup declared complete until relevant outstanding work has been reconciled.

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
- [ ] Moving an unchanged official finish between official content packs preserves its complete `PZ-*` ID and requires no persistence migration.

## Universal architecture

- [ ] PaintZ state, synchronization and persistence are implemented once on the shared `ItemBase` path.
- [ ] No PaintZ weapon-specific persistence hook exists.
- [ ] No PaintZ magazine-specific persistence hook exists.
- [ ] No PaintZ attachment/optic/light-specific persistence hook exists.
- [ ] `PaintZ_PaintTarget` does not dispatch paint state by category.
- [ ] Policy code does not use a closed weapon/magazine/attachment enum as the extensibility mechanism.
- [ ] A non-weapon/non-magazine `ItemBase` family can be enabled by JSON only and receives the same state/persistence behavior without script changes.
- [ ] No per-class compatibility registry or painted subclasses exist.
- [ ] Stale-finish migration/pruning uses the same shared `ItemBase` logical state and introduces no per-category persistence path.

## Painted item presentation

- [ ] A painted vanilla weapon shows ` [finish name]` after its normal display name and a `Finish: name (ID)` line in its tooltip/description.
- [ ] A painted magazine and at least one non-weapon/non-magazine `ItemBase` target receive the same presentation through the shared path.
- [ ] A weapon/subclass `NameOverride` that returns its own dynamic name without calling `super.NameOverride()` is still decorated by PaintZ through the final display-name accessor path.
- [ ] A weapon/subclass `DescriptionOverride` that returns its own dynamic description without calling `super.DescriptionOverride()` is still decorated by PaintZ through the final tooltip accessor path.
- [ ] Upstream dynamic names/descriptions remain intact before PaintZ adds finish information.
- [ ] A client can resolve presentation from synchronized PaintZ state without relying on category-specific presentation patches.
- [ ] Stripping removes only the PaintZ finish presentation and restores the normal upstream name/description behavior.
- [ ] PaintZ does not add weapon/classname-specific presentation patches.

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
- [ ] The bundled default includes weapon/magazine domains and common weapon/pistol/suppressor slot families, with no blanket crossbow exclusion.

## Stale finish recovery

- [ ] `$profile:PaintZ/paintz_stale_finishes.json` is created from the bundled default only when missing.
- [ ] The bundled default is non-destructive: `prune_unknown = false` and `migrations = {}`.
- [ ] A currently registered source finish is never migrated or pruned, even when listed in `migrations`.
- [ ] An exact unregistered OLD -> registered NEW mapping migrates the existing item without replacing its classname/object.
- [ ] A successful migration changes the authoritative logical ID to NEW and NEW persists after restart.
- [ ] A migration whose destination is not currently registered preserves OLD and logs a warning.
- [ ] Migration chains are rejected during config validation.
- [ ] Wildcard migration IDs are rejected during config validation.
- [ ] A matching migration takes precedence over `prune_unknown`.
- [ ] A matching migration that fails does not fall through to pruning.
- [ ] `prune_unknown = true` clears an unmapped unregistered PaintZ assignment on encounter.
- [ ] Pruning restores the configured/original texture when a safe selection is available.
- [ ] Pruning with no safe recoverable selection clears logical stale state without guessing a texture selection.
- [ ] Policy exclusion alone never makes a registered finish stale.
- [ ] Successful runtime config reload affects the item's next encounter and does not scan/mutate all entities immediately.
- [ ] Each item is evaluated at most once per successful config revision, preventing action-condition warning spam.
- [ ] Failed periodic reload retains the last-known-good stale-recovery config.
- [ ] Startup without a valid stale-recovery config preserves stale state.

## CF persistence

- [ ] `CfgMods.PaintZ.storageVersion` is positive and CF is a declared dependency.
- [ ] Painted weapon survives restart.
- [ ] Painted magazine survives restart with ammunition intact.
- [ ] A painted non-weapon/non-magazine `ItemBase` fixture survives restart using the same persistence path.
- [ ] Painted items survive in player inventory, persistent/nested storage and vehicle cargo.
- [ ] Repaint A -> B persists B.
- [ ] Strip -> restart remains unpainted.
- [ ] Runtime policy exclusion does not prevent restoration.
- [ ] Missing finish definition preserves the logical ID with the default stale-recovery config.
- [ ] Pre-PaintZ/unpainted items load normally with no PaintZ payload required.
- [ ] Late-joining client sees restored finish.
- [ ] Save with PaintZ removed but CF retained, then restore PaintZ: PaintZ state returns unless explicitly migrated/pruned after restoration.
- [ ] Removing CF as well is documented as outside the persistence guarantee.

## Selection / strip regression

- [ ] Known-good weapon and magazine fixtures still paint and strip.
- [ ] At least one configured non-weapon `ItemBase` fixture paints and strips without category-specific PaintZ code.
- [ ] At least one optic or flashlight with a safe body/camo/housing selection can paint.
- [ ] An optic/light exposing only functional glass/lens/reticle/display/emissive surfaces remains safely unsupported.
- [ ] Category words such as `optic` or `light` alone do not blacklist an otherwise safe body selection.
- [ ] Paint cans never offer Strip Paint; the dedicated stripper does.
- [ ] Ruined cans and ruined targets cannot receive new paint.
- [ ] Stale migration still requires a safe paintable selection and never guesses a protected/ambiguous surface.

`tools/sandbox/PaintZ_FinishSmokeTest.c` and the Paint Pack API fixture remain opt-in smoke tests. Multiplayer UI/visual replication, painted-item presentation interoperability, stale migration/prune persistence, and persistence restart scenarios require live-server acceptance testing.
