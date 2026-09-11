# PaintZ Codex Instructions

## Mandatory Git preflight

Before modifying repository files, inspect the active branch, repository status and latest commit. Do not put unrelated work on an existing feature/fix branch and never discard, reset, stash, rewrite or move user work merely to make the workspace convenient.

New independent work normally starts from current `main` or from the feature branch it explicitly extends. `dev` is for integration testing, not normal feature development.

## Documentation progression

Documentation is part of implementation. Changes to runtime behavior, configuration, persistence, APIs, build/release workflow, supported content or another documented contract must update the relevant README/docs/config help on the same branch.

Do not knowingly leave documentation describing superseded behavior after implementation changes.

---

# Mission

PaintZ is a **generic DayZ runtime painting framework for ordinary inventory items**.

A player holds a PaintZ finish applicator, targets an eligible object, and PaintZ:

1. resolves the existing runtime object;
2. checks server JSON relevance/policy;
3. inspects the actual model/hidden selections;
4. conservatively chooses a safe paintable surface;
5. applies the finish to the existing object instance;
6. synchronizes one logical finish state;
7. persists that state with the physical item.

PaintZ must not require compatibility implementation for every classname, slot family or target category.

---

# Highest-priority architecture invariant

## Categories and attachment families are configuration, not code

Weapons and detachable magazines are default domains, not architectural special cases.

For a normal `ItemBase`-derived inventory family, adding support must require JSON configuration only when the runtime model exposes a safe paintable hidden selection.

Examples include suppressors, muzzle devices, handguards, stocks, grips, bipods, optics, lights, clothing, helmets, backpacks, containers and tools.

Adding one of these must not require:

- a new PaintZ state class;
- a new persistence hook;
- a new network synchronization path;
- a new category enum;
- a new paint dispatch branch;
- a new classname compatibility table.

If an ordinary inventory category appears to require Enforce Script changes, first prove that a DayZ engine limitation genuinely prevents the generic design.

The JSON-only guarantee does not automatically extend to unrelated non-`ItemBase` hierarchies such as static world objects, buildings or vehicles.

---

# Generic item state

PaintZ logical state belongs once on `ItemBase`.

The shared state is responsible for:

- canonical finish ID;
- resolved paint selection;
- network synchronization;
- persistent logical assignment;
- post-load visual restoration.

`PaintZ_PaintTarget` must use the shared `ItemBase` state path. Do not branch into weapon/magazine/attachment/clothing-specific state implementations.

Persistence must not inspect runtime eligibility policy. If an item was painted previously, later policy exclusion must not erase its state.

---

# Persistence

PaintZ requires Community Framework and uses **CF ModStorage**.

The production persistence hook belongs once at `ItemBase` through `CF_OnStoreSave` / `CF_OnStoreLoad`.

Do not replace this with broad native `ItemBase::OnStoreSave` / `OnStoreLoad` appends. Derived DayZ classes may serialize additional data after `super`, so broad native base-layer appends can corrupt subclass streams.

Do not reintroduce:

- weapon-specific PaintZ persistence hooks;
- magazine-specific PaintZ persistence hooks;
- per-category persistence code;
- sidecar entity databases without an explicit redesign.

Persist only logical data needed to reconstruct the finish, normally the canonical PaintZ finish ID. Do not persist texture paths, material paths, hidden-selection indexes, policy decisions, cached inspection results or filesystem paths.

Required behavior:

- pre-PaintZ/unpainted items load normally when no PaintZ ModStorage context exists;
- malformed/unsupported PaintZ state must never make the underlying item fail to load;
- unknown finish IDs remain historical logical state and must not become an implicit strip;
- runtime eligibility policy is not consulted during restore;
- temporarily missing content does not erase persisted identity;
- removing CF as well is outside the persistence guarantee.

`CfgMods.PaintZ.storageVersion` is the CF ModStorage schema version source.

---

# Object preservation

Painting modifies the existing object instance. Never replace the target with a painted classname variant.

Preserve, as applicable:

- classname and object/network identity;
- persistence identity;
- health and quantity;
- ammunition and chamber/FSM state;
- attachments and cargo;
- inventory location and parent/container relationships;
- unrelated third-party state.

The normal visual operation is `SetObjectTexture()` or the verified current equivalent. Preserve the target's existing material unless a separately designed finish explicitly requires material behavior.

---

# Runtime eligibility policy

`$profile:PaintZ/paintz_items.json` controls **new painting/repainting**. It is not the persistence schema and not a classname compatibility database.

## Selector consistency

Shared selector families are:

- `type` / `types`;
- `class_pattern` / `class_patterns`;
- `inventory_slot` / `inventory_slots`;
- `inventory_slot_pattern` / `inventory_slot_patterns`.

Singular and plural values in one family form one OR group. Different selector families inside one domain/rule are AND.

Future shared selector families must use consistent naming and semantics in both domains and rules.

## Domains

Domains are a positive OR-list. They may combine type, classname and declared-slot selectors.

Slot matching must inspect the target class's **declared compatible `inventorySlot` values**, not its current attachment location.

Do not introduce a classname-to-slot registry. Read inherited config data generically from the target's actual config root.

Unknown/modded slot names are valid policy data and must not require source changes.

## Rules

Rules are ordered include/exclude policy. Last matching rule wins.

Type scope may use valid DayZ base/config types plus legacy `weapon` / `magazine` aliases for backward compatibility.

An include rule means the item may be considered; it never bypasses hidden-selection/model safety.

Policy exclusion or domain removal must not strip existing paint or block Strip Paint.

---

# No hard category exclusions

Do not hard-exclude sensible inventory families such as optics, flashlights, suppressors, muzzle devices, stocks, handguards, grips, containers or clothing.

Whether a category is relevant is a JSON policy decision. Whether a specific model is technically paintable is a model/selection-safety decision.

---

# Runtime configuration reload

Do not parse JSON every frame or every action evaluation.

Parse and validate a detached candidate, then replace the active config only when valid. A failed reload retains the last-known-good config. Startup with no valid required config fails safely for the affected feature.

Keep operational explanations in adjacent README/docs, not pseudo-comments inside JSON.

---

# Hidden-selection/model safety

Compatibility comes from the actual target object/model, not a classname registry.

Use verified runtime APIs/config fallback to inspect hidden selections. Never invent a selection.

Prefer plausible body selections such as `camo`, `zbytek`, `body`, `receiver`, `weapon`, `mag`, `magazine`, `housing`, `shell` and `frame`.

Reject clearly functional/non-body selections involving glass/lens, reticle, display/screen, LED/emissive/glow, flame or other functional surfaces.

Do not reject a selection merely because its name contains a broad category word such as `optic` or `light`.

If several candidate selections remain ambiguous, reject the target rather than guessing.

---

# Server authority and networking

Painting is server-authoritative.

At action completion revalidate applicator state/quantity, ruined state, target state, domain membership, runtime policy, finish ID, model capability/selection and relevant action context/range.

Clients must not be trusted to choose arbitrary finish IDs, textures, selections or policy outcomes.

Runtime policy synchronization must serialize every supported selector consistently on server and client.

Use one logical PaintZ state source. Persistence and network replication are separate responsibilities but must not duplicate state.

Late-joining/relevant clients must resolve authoritative finish state correctly.

---

# Ruined items

A ruined PaintZ applicator cannot apply paint. A ruined target cannot receive new paint.

Persisted historical paint restoration is not a new application; do not erase historical state merely because the target later becomes ruined or excluded.

Validate ruined conditions again on the authoritative server at completion.

---

# Stripping

Stripping is separate from new-paint eligibility policy.

An already-painted item must remain strippable even when its class/domain/slot family is no longer eligible for new painting.

Strip the existing item; do not replace it. Restore the intended unpainted texture/state and clear the logical persisted PaintZ assignment.

Keep consumption/timing design data centralized/configurable rather than scattering magic numbers through action code.

---

# Finish identity and assets

Paint IDs are stable developer-authored logical identifiers such as `PZ-C-FTN`, `PZ-B-BLK` or a third-party `NCP-S-FDE`.

IDs must be unique and should remain stable after release. Display-name/color corrections should not require changing the logical ID. A deliberate breaking identity change must be documented and handled through the stale-finish mechanism rather than silently repurposing an existing ID.

Generated assets must be deterministic and reproducible. Fix generator/source data rather than hand-editing generated output.

Do not add third-party textures, camouflage, logos, fonts, graphics or other assets without a license that permits intended use/redistribution.

Pattern assets intended for repetition should be seamless/tileable and should not bake inappropriate fabric/geometry-specific appearance into generic surfaces.

---

# Third-party compatibility

Treat third-party content as unknown until inspected.

Compatibility should come from runtime type/inheritance, declared inventory-slot config, actual hidden selections, generic safe heuristics and optional administrator policy.

Do not add a compatibility patch merely because one modded item initially fails. Determine whether it is outside policy, declares unexpected slots, exposes no safe hidden selection or violates normal engine behavior.

---

# DayZ / CF API discipline

Do not invent APIs, callbacks, inheritance relationships, config semantics, replication behavior or serializer guarantees.

When uncertain, inspect current authoritative/public sources first:

- `BohemiaInteractive/DayZ-Script-Diff` for DayZ script/config behavior;
- current `Arkensor/DayZ-CommunityFramework` source/docs for CF ModStorage.

When source reality conflicts with documentation, fix the documentation/architecture rather than forcing an obsolete assumption.

---

# Scope and completion discipline

Keep changes focused and reviewable.

- Prefer generic helpers over category branches.
- Centralize shared rules.
- Avoid unnecessary framework layers, per-frame work and RPC traffic.
- Fail safely.
- Keep client/server responsibilities explicit.
- Comments should explain why, especially around engine behavior.
- Do not opportunistically redesign unrelated systems.

Before considering work complete:

1. inspect the full diff against the branch base;
2. remove unrelated changes;
3. confirm generated files were not hand-edited incorrectly;
4. run available static/generator tests;
5. run DayZ Tools/server compilation when relevant and available;
6. exercise selector forms affected by policy changes;
7. verify protected functional surfaces remain unsupported;
8. perform live persistence/multiplayer acceptance for lifecycle changes;
9. update all relevant documentation on the same branch.

Do not claim an unrun build/test passed.
