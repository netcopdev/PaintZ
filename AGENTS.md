# PaintZ Codex Instructions

## Mandatory Git preflight

Before modifying, creating, deleting, renaming, or generating any repository file, inspect:

```text
git branch --show-current
git status --short
git log -1 --oneline
```

Do not put unrelated work on an existing feature/fix branch. A non-main branch belongs to its named task unless the user explicitly says otherwise. Never discard, reset, stash, rewrite, or move user work merely to make the workspace convenient.

Tightly related corrections belong on the existing feature branch. New independent work normally starts from current `main`. `dev` is for integration testing, not normal feature development.

---

# Mission

PaintZ is a **generic DayZ runtime painting framework for inventory items**.

A player holds a PaintZ finish applicator, targets an eligible object, and PaintZ:

1. resolves the existing runtime object;
2. checks server JSON relevance/policy;
3. inspects the actual model/hidden selections;
4. conservatively chooses a safe paintable selection;
5. applies the finish to the existing object instance;
6. synchronizes the logical finish state;
7. persists that state with the physical item.

PaintZ must not require a compatibility implementation for every classname or every target category.

---

# Highest-priority architecture invariant

## Categories are configuration, not code

Weapons and detachable magazines are the **shipped default domains only**.

For any normal `ItemBase`-derived inventory family, adding support must require JSON configuration only, provided the runtime model exposes a safe paintable hidden selection.

Examples include future support for:

- suppressors;
- handguards;
- stocks/grips;
- clothing;
- helmets;
- backpacks;
- containers;
- tools;
- other ordinary inventory items.

Adding one of these domains must **not** require:

- a new PaintZ state class;
- a new persistence hook;
- a new network synchronization path;
- a new category enum;
- a new paint dispatch branch;
- per-class compatibility code.

If a proposed implementation makes a new ordinary inventory category require Enforce Script changes, stop and redesign it unless a verified DayZ engine limitation genuinely forces an exception.

The JSON-only guarantee does not automatically extend to unrelated non-`ItemBase` hierarchies such as static world objects, buildings or vehicles. Those require a separate explicit design decision.

---

# Generic item state

PaintZ logical state belongs once on `ItemBase`.

The shared state is responsible for:

- canonical finish ID;
- current resolved paint selection;
- network synchronization;
- persistent logical assignment;
- post-load visual restoration.

`PaintZ_PaintTarget` must dispatch through the shared `ItemBase` state path. Do not branch into weapon/magazine/attachment/clothing-specific state implementations.

Persistence must not inspect runtime domains or policy. If an item was painted previously, removing it from a domain or excluding it later must not erase the historical finish.

---

# Persistence

PaintZ requires Community Framework and uses **CF ModStorage**.

The production persistence hook belongs once at `ItemBase` through `CF_OnStoreSave` / `CF_OnStoreLoad`.

Do not replace this with broad native `ItemBase::OnStoreSave` / `OnStoreLoad` appends. Derived DayZ classes may serialize additional data after calling `super`, so inserting PaintZ bytes at a broad native base layer can place them in the middle of subclass streams and break legacy/third-party persistence.

Do not reintroduce:

- `Weapon_Base` PaintZ persistence hooks;
- `MagazineStorage` PaintZ persistence hooks;
- per-category persistence code;
- sidecar entity databases unless a future explicit redesign requires one.

Persist only the logical data needed to reconstruct the finish, normally the canonical PaintZ finish ID. Do not persist derived texture paths, material paths, hidden-selection indexes, policy decisions, cached inspection results, or filesystem paths.

Required behavior:

- pre-PaintZ/unpainted items load normally when no PaintZ ModStorage context exists;
- malformed/unsupported PaintZ state must never make the underlying DayZ item fail to load;
- unknown finish IDs remain logical historical state and must not become an implicit strip;
- runtime policy is not consulted during restore;
- visual restore may be deferred narrowly until the model/hidden selections are safe to access;
- if PaintZ is temporarily unloaded while CF remains loaded, CF's unloaded-mod preservation must retain the opaque PaintZ payload;
- removing CF as well is outside the persistence guarantee.

`CfgMods.PaintZ.storageVersion` is the schema version source for CF ModStorage. Preserve backward compatibility when it changes.

---

# Object preservation

Painting modifies the existing object instance. Never replace the target with a painted classname variant.

Preserve, as applicable:

- classname;
- object/network identity;
- persistence identity;
- health;
- quantity;
- ammunition;
- chamber/FSM state;
- attachments;
- cargo;
- inventory location;
- parent/container relationships;
- third-party state owned by the item/mod.

The normal visual operation is `SetObjectTexture()` or the verified current equivalent. Preserve the target's existing RVMat/material unless a separately designed finish explicitly requires material behavior.

---

# Runtime eligibility policy

`$profile:PaintZ/paintz_items.json` is an administrative layer for **new painting/repainting**. It is not the persistence schema and not the compatibility database.

## Domains

Domains determine whether a target is relevant enough to offer PaintZ feedback/new painting.

A domain may use:

- a DayZ base/config type;
- a case-insensitive classname wildcard;
- both, with AND semantics.

Separate domain entries are OR.

The default config may contain `Weapon_Base` and `Magazine_Base`, but the core must not assume those are the only categories.

## Rules

Rules are ordered include/exclude policy. Last matching rule wins.

Rule scoping must remain generic. A rule may target:

- `all` / omitted type;
- a valid DayZ base/config type;
- legacy `weapon` / `magazine` aliases only for backward compatibility with existing version-1 configs.

Do not use a closed enum whose extension requires code changes for each new category.

Selectors remain deliberately simple:

- `class_pattern` with `*` and `?`;
- `inherits` for DayZ config inheritance.

An include rule means the item may be considered; it never bypasses hidden-selection/model safety.

Policy exclusion or domain removal must not strip existing paint and must not block Strip Paint.

---

# Runtime configuration reload

Do not parse JSON every frame or every action evaluation.

Parse and validate a detached candidate, then atomically replace the active config only when valid. A failed reload retains the last-known-good config. Startup with no valid config fails closed for new painting.

Validate schema version, required fields, action values, selectors, wildcard data, and type/inheritance references where the current API permits reliable validation.

Keep operational explanations in the adjacent README/docs, not pseudo-comments inside JSON.

---

# Hidden-selection/model safety

Compatibility comes from the actual target object, not a classname registry.

Use verified runtime APIs/current config fallback to inspect hidden selections. Never invent a selection.

Prefer plausible body selections such as:

- `camo`;
- `zbytek`;
- `body`;
- `receiver`;
- `weapon`;
- `mag`;
- `magazine`.

Reject clearly functional/non-body selections involving:

- glass/lens;
- optic/reticle;
- display/screen;
- LED/light;
- emissive/glow;
- flame;
- similar functional visuals.

If exactly one non-blocked selection exists, it may be a conservative fallback. If several remain ambiguous, reject the target rather than guessing.

A false negative is preferable to painting a lens, reticle, display, or emissive component.

---

# Server authority and networking

Painting is server-authoritative.

At action completion revalidate:

- applicator type/state;
- applicator quantity;
- ruined state;
- target state;
- domain membership;
- runtime policy;
- finish ID;
- model capability/selection;
- relevant action context/range.

Clients may display actions and visuals but must not be trusted to choose arbitrary finish IDs, textures, selections, or policy bypasses.

Use one logical PaintZ state source. Persistence and network replication are separate responsibilities but must not duplicate state.

Late-joining/relevant clients must resolve the authoritative finish correctly.

---

# Ruined items

A ruined PaintZ applicator cannot apply paint. A ruined target cannot receive new paint.

Persisted historical paint restoration is not a new paint application; do not erase or refuse historical state merely because the target is now ruined or excluded.

Validate ruined conditions again on the authoritative server at completion.

---

# Stripping

Stripping is separate from new-paint policy.

An already-painted item must remain strippable even when:

- its class is excluded;
- its domain is removed;
- the current policy would deny new painting.

Strip the existing item; do not replace it. Restore the intended unpainted texture/state and clear the logical persisted PaintZ assignment.

Do not scatter consumption magic numbers through action code; keep such design data centralized/configurable where practical.

---

# Finish identity and assets

Paint IDs are stable developer-authored logical identifiers such as `PZ-S-WHT` or the current project convention.

IDs must be unique and should remain stable after release. Display-name/color corrections should not require changing the logical ID.

Generated assets must be deterministic and reproducible. Fix generator/source data rather than hand-editing generated outputs.

Do not add third-party textures, camouflage, logos, fonts, graphics or other assets without a license that permits intended use/redistribution.

Painted-surface assets must not bake inappropriate fabric/geometry-specific appearance into generic metal/polymer use. Pattern assets intended for repetition should be seamless/tileable.

---

# Third-party compatibility

Treat third-party content as unknown until inspected.

Compatibility should come from:

- runtime type/inheritance;
- actual hidden selections;
- generic safe heuristics;
- optional administrator policy.

Do not add a compatibility patch merely because one modded item initially fails. Determine why the generic mechanism rejected it first.

A third-party item that breaks the normal DayZ `super` persistence chain may remain an external compatibility limitation; do not add classname hacks to compensate without an explicit design decision.

---

# DayZ / CF API discipline

Do not invent APIs, callbacks, inheritance relationships, replication behavior, or serializer guarantees.

When uncertain, inspect current authoritative/public sources first:

- `BohemiaInteractive/DayZ-Script-Diff` for DayZ script behavior;
- current `Arkensor/DayZ-CommunityFramework` source/docs for CF ModStorage.

Important verified current relationship: DayZ defines `InventoryItemSuper`/`InventoryItemBase` as `ItemBase` aliases. Do not infer a separate architecture merely because `Weapon` or `Magazine` source declarations use the alias name.

When source reality conflicts with documentation, fix the documentation/architecture rather than forcing an obsolete assumption.

---

# Scope and coding discipline

Keep changes focused and reviewable.

- Prefer generic helpers over category branches.
- Centralize shared rules.
- Avoid unnecessary framework layers.
- Avoid per-frame work and repeated filesystem parsing.
- Avoid unnecessary RPC traffic.
- Fail safely.
- Keep client/server responsibilities explicit.
- Comments should explain why, especially around engine behavior.
- Do not opportunistically redesign unrelated systems.

Before considering work complete:

1. inspect the full diff against the branch base;
2. remove unrelated changes;
3. confirm no generated files were hand-edited incorrectly;
4. run available static/generator tests;
5. run DayZ Tools/server compilation when available;
6. perform live persistence/multiplayer acceptance for lifecycle changes;
7. update `README.md`, `docs/ARCHITECTURE.md`, relevant feature docs, and release acceptance criteria when architecture changes.

Do not claim an unrun build/test passed.
