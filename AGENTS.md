# PaintZ Codex Instructions

## Git branch safety — mandatory preflight

Before modifying, creating, deleting, renaming, or generating ANY repository
file, run:

    git branch --show-current
    git status --short
    git log -1 --oneline

Then determine whether the current branch is clearly intended for the user's
current task.

### Hard rule

NEVER implement a new feature, fix, refactor, or unrelated change on an
existing feature/fix branch merely because that branch is currently checked
out.

If the current branch contains work for another task, STOP BEFORE MODIFYING
FILES.

Do not:
- add the new work to the unrelated branch
- "temporarily" make the changes there
- assume the user wants the current branch reused
- switch branches silently
- create a new branch silently
- move existing uncommitted work between branches

Instead, report:

    Branch mismatch: current branch is <branch>.
    This task appears unrelated to that branch.
    No files were modified.

A non-main branch should be treated as belonging to its named task unless the
user explicitly says the new task belongs there.

Examples:

- current: feature/runtime-paint-rules
  task: add spray animation
  => STOP; unrelated task

- current: feature/runtime-paint-rules
  task: fix wildcard matching in runtime paint rules
  => continue

- current: fix/spray-action-animation
  task: adjust spray animation timing
  => continue

When uncertain whether the task belongs to the current branch, STOP rather
than modifying the repository.

## Mission

PaintZ is a generic DayZ runtime painting framework.

The intended interaction is:

- a player holds a PaintZ spray can or other supported finish applicator;
- the player targets a supported object such as a weapon or detachable magazine;
- PaintZ inspects the actual runtime object;
- PaintZ determines whether the object is eligible and exposes a safe paintable hidden selection;
- if compatible, PaintZ applies the selected finish to the existing object instance;
- if incompatible, excluded, ruined, or ambiguous, PaintZ fails safely and explains why.

PaintZ must remain broadly compatible with vanilla and third-party weapons without requiring compatibility code for every supported classname.

Prefer generic runtime behavior over item-specific patches.

---

# Core architecture

## Generic runtime compatibility is non-negotiable

Do **not** solve compatibility by creating or maintaining:

- painted weapon subclasses such as `AKM_Woodland`, `M4_Green`, etc.;
- painted magazine subclasses for every finish;
- per-weapon `modded class M4A1_Base`, `modded class FAL_Base`, etc.;
- large classname-to-texture registration tables;
- classname allowlists as the primary compatibility mechanism;
- compatibility PBOs whose only purpose is to identify individual weapon classes;
- replacement objects created solely to represent a painted variant.

The target object must be inspected dynamically.

PaintZ should work because the runtime object exposes usable model selections and satisfies generic eligibility rules, not because PaintZ already knows that exact classname.

Allowed generic hooks include appropriate DayZ base classes such as:

- `Weapon_Base`;
- `Magazine_Base` or the correct current DayZ magazine base API;
- PaintZ spray-can/item classes;
- generic PaintZ actions;
- generic PaintZ runtime/helper classes.

If the current DayZ API differs from assumptions in this file, verify the API and use the correct current implementation rather than forcing an outdated assumption.

---

# Object preservation

## Modify the existing object

Painting must normally modify the existing object instance.

Do not replace the target with another classname to simulate a painted variant.

Painting must preserve, as applicable:

- classname;
- object/network identity;
- persistence identity;
- health;
- quantity;
- ammunition;
- chamber state;
- attachments;
- cargo;
- inventory location;
- parent/container relationships;
- other mod-specific state belonging to the object.

The normal visual operation is `SetObjectTexture()` or the correct verified equivalent for the current DayZ API.

Material replacement is **not** the default PaintZ mechanism.

Preserve the target's existing material/RVMat unless a deliberately designed finish specifically requires material behavior.

---

# Paint eligibility

Painting must pass all relevant eligibility checks before modifying the target.

At minimum:

- the paint/applicator item must be valid;
- the target must be a supported object category;
- the target must pass runtime configuration policy;
- the target must not be ruined;
- the target must expose a safe paintable hidden selection;
- the requested finish must be valid and available.

## Ruined spray cans

A ruined PaintZ spray can must not be usable.

Do not:

- offer a functional paint action from a ruined can;
- consume paint from a ruined can;
- allow client-side action invocation to bypass this restriction.

Validate the can condition on the authoritative side when the action executes, not only when deciding whether to display the action.

If the can becomes ruined between action discovery and execution, painting must fail safely.

## Ruined targets

A ruined weapon, magazine, or other paintable target must not be painted.

This restriction must be enforced during authoritative action execution.

Do not rely solely on UI/action visibility because object health may change after the action was presented.

A ruined target should fail with an appropriate player-facing reason such as:

`Cannot Paint: Item is ruined`

Use the project's normal localization/message mechanism rather than hard-coding duplicate strings throughout the codebase.

---

# Runtime hidden-selection policy

Determine paintability from the actual target object.

Use verified runtime APIs such as:

- `GetHiddenSelections()`;
- `GetHiddenSelectionIndex()`;

or their current DayZ equivalents.

Never invent a hidden selection that the object does not expose.

## Selection heuristic

Prefer globally plausible body-like selection names such as:

- `camo`;
- `zbytek`;
- `body`;
- `receiver`;
- `weapon`;
- `mag`;
- `magazine`.

Reject obviously unsafe/non-body selections such as those representing:

- glass;
- lenses;
- optics;
- reticles;
- displays;
- screens;
- LEDs;
- lights;
- emissive surfaces;
- glow surfaces;
- flames;
- similar functional visual components.

Matching should be case-insensitive where appropriate.

If there is exactly one usable hidden selection and it is not blocked, it may be used as a conservative fallback.

If several selections remain and PaintZ cannot determine safely which one represents the intended paintable body, fail safely.

Do not guess merely to increase compatibility.

An unsupported item is preferable to damaging its optics, display, emissive components, or other functional textures.

---

# Runtime JSON configuration

Runtime configuration is an administrative policy layer.

It must not replace PaintZ's generic runtime compatibility architecture.

The intended use is to let server administrators:

- explicitly include object families;
- explicitly exclude object families;
- limit rules to categories such as weapon or magazine;
- work around problematic mod families;
- change policy without rebuilding the PaintZ PBO where practical.

Do not turn the configuration into a mandatory database containing every supported weapon.

Generic hidden-selection detection remains the normal compatibility mechanism.

---

## Include/exclude rules

Configuration may support rules such as:

- classname matching;
- wildcard classname matching;
- target category filtering;
- explicit include rules;
- explicit exclude rules.

Example concepts:

- `TTC_AK*`
- all weapons from a particular mod namespace;
- magazines matching a particular family;
- exclusion of a known problematic item family.

The exact schema belongs in project documentation, for example:

`docs/RUNTIME_CONFIG.md`

`AGENTS.md` defines the architectural rules; the configuration documentation defines the exact JSON contract.

---

## Wildcard behavior

Wildcard matching must remain deliberately simple.

Prefer shell-style matching such as:

- `*` = zero or more characters;
- `?` = exactly one character, if needed.

Do not silently introduce:

- regular expressions;
- complex glob extensions;
- arbitrary script expressions;
- executable predicates.

unless there is a documented and approved reason.

Wildcard semantics must be:

- deterministic;
- documented;
- testable;
- consistent between reloads.

Classname matching should normally be case-insensitive unless DayZ behavior makes case sensitivity materially necessary.

Do not rely on unspecified engine string behavior.

---

## Rule precedence

Configuration precedence must be explicit and documented.

Preferred default policy:

1. invalid rules are ignored with an error;
2. explicit exclusions override inclusions;
3. matching inclusion rules may permit targets otherwise restricted by administrative policy;
4. generic PaintZ compatibility checks still apply;
5. configuration must not force painting onto an object that lacks a safe usable hidden selection.

An administrative include rule means:

`This object family may be considered for painting.`

It must **not** mean:

`Ignore model safety and paint whatever selection is available.`

Configuration must never bypass core model-safety rules.

---

## Target categories

Where useful, rules may distinguish supported target categories such as:

- `weapon`;
- `magazine`.

Future categories should be added deliberately.

Do not implement vague type matching where a rule may unexpectedly affect unrelated DayZ objects.

A configuration rule should have predictable scope.

---

## Default behavior

The configuration specification must clearly define:

- behavior when include lists are absent;
- behavior when include lists are empty;
- behavior when exclude lists are absent;
- behavior when exclude lists are empty;
- conflict resolution;
- duplicate rules;
- unsupported rule types;
- malformed wildcard patterns;
- unknown object categories.

Do not allow default behavior to emerge accidentally from implementation details.

---

# Runtime configuration reload

If configuration can be changed without restarting the server, reloading must be safe.

Do not:

- read and parse the JSON every frame;
- read it repeatedly for every action condition evaluation;
- partially modify live configuration while parsing;
- replace working configuration with malformed configuration.

Use either:

- an explicit administrative reload mechanism;
- a reasonable periodic file-change/reload check;
- another simple documented approach.

Avoid unnecessary filesystem work.

---

## Last-known-good configuration

Parse and validate new configuration separately.

Only replace active configuration after the new configuration has been fully validated.

If reload fails:

- retain the current valid configuration;
- log the error clearly;
- do not partially apply the broken configuration;
- do not silently fall back to materially different defaults.

Runtime configuration replacement should be atomic from PaintZ's perspective.

---

## Configuration validation

Validate, where applicable:

- JSON syntax;
- schema/version;
- target categories;
- required fields;
- value types;
- wildcard patterns;
- duplicate or conflicting definitions;
- unsupported values.

Configuration errors should identify the relevant rule or field where practical.

Do not report successful reload when some materially important part failed silently.

---

## Configuration versioning

If the runtime JSON format is expected to evolve, include a simple schema/config version early.

Prefer an explicit field such as:

`"version": 1`

Do not infer configuration format solely from which fields happen to exist.

When the schema changes incompatibly, either:

- support migration;
- support the previous version temporarily;
- or fail with a clear unsupported-version error.

Do not silently reinterpret old configuration using new semantics.

---

# Server authority and networking

Game-affecting painting must be server-authoritative.

Clients may present actions and display synchronized visual state, but must not be trusted to arbitrarily choose:

- target eligibility;
- hidden-selection index;
- unrestricted texture paths;
- unrestricted material paths;
- arbitrary PaintZ finish IDs;
- bypasses for item condition;
- bypasses for runtime configuration.

Validate the action again on the server when it executes.

This includes validating:

- spray can condition;
- target condition;
- target type;
- range/context where relevant;
- runtime policy;
- requested finish;
- selected hidden selection;
- remaining paint quantity where applicable.

Do not assume that because the client displayed an action, its original conditions are still valid.

---

# Finish identity

Paint IDs are persistent logical identifiers.

Prefer explicit developer-authored IDs such as:

`PZ-S-WHT`

or other IDs following the established PaintZ naming convention.

Rules:

- IDs must be unique;
- IDs should normally be short and descriptive;
- IDs should remain stable after release;
- changing a display name should not require changing the ID;
- small color corrections should not require changing the ID;
- IDs must not depend solely on mutable RGB/pattern properties;
- do not introduce a database merely to map sequential numbers to finishes unless there is a real architectural reason.

When modifying ID rules, preserve backwards compatibility where practical.

---

# Texture and material philosophy

PaintZ normally changes the target's visible finish while preserving its original material behavior.

For ordinary solid colors and camouflage:

- change the texture;
- retain the original material/RVMat;
- avoid baking weapon-specific lighting into reusable textures;
- avoid baking cloth/fabric appearance into textures intended for metal or polymer objects;
- avoid fake geometry-dependent shading;
- keep camouflage reusable across different models.

Pattern assets intended for repeating use should normally be seamless/tileable.

Pattern scale may need adjustment for substantially different target dimensions, but scaling logic must remain generic rather than classname-specific.

Do not introduce a per-weapon pattern scale database without a deliberate design decision.

---

# Wear and appearance overlays

Reusable appearance effects may include:

- scratches;
- scuffs;
- paint wear;
- dirt;
- spotting;
- mild surface variation;
- similar non-geometry-specific effects.

Keep reusable wear effects as generator assets or procedural layers rather than manually baking a unique version into every source paint definition.

Procedural variation should be deterministic.

Do not allow generator randomness to produce different release assets from identical inputs.

---

# Asset generation

Generated PaintZ assets must be reproducible.

Running the generator twice with identical:

- source assets;
- paint definitions;
- generator version;
- configuration;

should produce functionally identical output.

If visual variation is procedural, use deterministic seeding based on stable input such as:

- finish ID;
- explicit seed;
- another documented stable identifier.

Do not generate random production output that changes every build.

---

## Generated files are not source files

Source configuration, templates, source patterns, source overlays, and generator code are the source of truth.

Do not manually fix generated output when the correct fix belongs in:

- generator code;
- paint definition;
- SVG/template;
- source pattern;
- overlay asset;
- layout definition.

Fix the source and regenerate.

Before editing a file, determine whether it is generated.

---

## Texture validation

Game textures destined for DayZ conversion must satisfy the actual requirements of the conversion/build tools.

Where `ImageToPAA` requires power-of-two dimensions, validate that before conversion.

Generator validation should detect, where practical:

- invalid dimensions;
- missing source files;
- duplicate paint IDs;
- malformed color values;
- invalid paint definitions;
- missing output directories;
- unsupported image formats;
- broken source references;
- invalid appearance profiles.

Fail early with a useful error.

Do not silently skip failed finishes and still report successful generation.

---

# Asset licensing

Do not add third-party:

- camouflage textures;
- game textures;
- logos;
- fonts;
- photographs;
- graphics;
- other copyrighted assets

unless their license allows the intended use and redistribution.

Do not assume an asset found online is reusable.

For imported assets:

- record the source;
- verify licensing;
- retain attribution where required;
- document relevant restrictions.

When licensing is unclear, do not treat the asset as production-ready.

Reference material may be used to understand a pattern or visual style, but do not knowingly copy another game's or mod's protected texture into PaintZ without permission.

---

# Paint consumption

Spray cans are consumable resources.

Consumption behavior must be deterministic and server-authoritative.

Do not allow:

- negative quantity;
- painting without sufficient remaining paint;
- duplicated quantity changes caused by client/server action execution;
- ruined cans to continue applying paint.

If consumption amount varies by target category or size, keep that logic generic and documented.

Do not create per-class consumption tables unless deliberately required.

---

# Paint stripping

Paint stripping, when implemented, is a separate generic interaction.

It must restore the object's intended unpainted state without replacing the object.

Current design direction may include requirements such as:

- alcohol/disinfectant;
- rags;
- different rag consumption for weapons and magazines.

Treat exact quantities as configuration/design data where appropriate rather than scattering magic numbers across action code.

A ruined required stripping item must not count as a valid usable resource unless specifically designed otherwise.

Do not redesign stripping while implementing unrelated painting work.

---

# Persistence

Persistence is a separate design problem and must not be implemented casually.

The historical Reskin Manager approach of appending custom state through broad base-class:

- `OnStoreSave()`;
- `OnStoreLoad()`;

overrides must **not** be copied blindly.

Persistence changes can affect existing server databases and other mods participating in serialization.

Before implementing or materially changing PaintZ persistence:

1. inspect current DayZ persistence APIs and patterns;
2. document the intended serialization design;
3. consider installation onto an existing live server;
4. test objects saved before PaintZ installation;
5. test objects saved after PaintZ installation;
6. test server restart/reload;
7. consider PaintZ removal after painted objects exist;
8. test interaction with other mods overriding persistence;
9. consider data-format versioning before release.

Do not hide a persistence redesign inside an unrelated feature branch.

---

# Scope discipline

Do not opportunistically expand the requested task.

For example, work on runtime include/exclude rules should not silently turn into:

- persistence redesign;
- attachment painting;
- an RGB editor;
- material replacement;
- compatibility patches;
- texture extraction;
- inventory redesign;
- broad generator rewrites.

Small supporting refactors needed for a clean implementation are acceptable.

Large independent work belongs in a separate branch.

---

# Git and development workflow

## Branch roles

### `main`

`main` is the production-quality branch.

It should contain code that is:

- coherent;
- buildable;
- reasonably tested;
- suitable for normal server use;
- free from known integration-only hacks.

Do not use `main` as a development scratch branch.

Do not merge unfinished work into `main` merely to simplify testing.

### `dev`

`dev` is the shared integration-testing branch.

Use it to combine current development branches for:

- complete builds;
- dedicated-server testing;
- in-game testing;
- interaction testing between concurrent features/fixes.

`dev` is not the source of truth for individual work.

Do not normally develop features directly on `dev`.

Do not normally merge `dev` wholesale into `main`.

Stable work should reach `main` from its own branch.

---

## One logical change per branch

Each significant independently reviewable change should normally have its own branch.

Use names such as:

- `feature/runtime-paint-rules`;
- `feature/paint-stripping`;
- `feature/runtime-config`;
- `fix/selection-detection`;
- `fix/generated-texture-size`;
- `refactor/finish-registry`;
- `chore/build-tooling`;
- `docs/runtime-config`.

Tightly related follow-up work should reuse the appropriate existing branch.

Do not create a new branch for every small correction to an unfinished feature.

Do not accumulate unrelated work into an existing branch merely because it is already checked out.

---

## Base new work on `main`

New independent work should normally start from current `main`.

Avoid basing new work on:

- `dev`;
- unrelated feature branches;
- unrelated fix branches.

This keeps branches independently:

- understandable;
- testable;
- reviewable;
- mergeable;
- discardable.

---

## Avoid unnecessary stacked dependencies

Do not stack branches merely for convenience.

If feature B appears to require unfinished feature A:

1. verify that the dependency is real;
2. prefer completing A first when practical;
3. otherwise keep the dependency explicit and minimal;
4. document it when reporting the work.

Never make a normal feature depend on `dev`.

If several features need common foundation work, consider extracting that foundation into its own focused branch rather than building a long branch chain.

---

## Integration testing through `dev`

When several branches require combined testing:

1. bring `dev` up to date with `main`;
2. merge/integrate the required branches into `dev`;
3. build and test from `dev`;
4. determine which originating branch owns each defect;
5. fix the defect on that branch;
6. reintegrate the corrected branch into `dev`.

Do not leave required feature fixes only on `dev`.

Otherwise `dev` becomes the only branch where the feature actually works.

---

## Promoting work to `main`

A feature or fix should normally reach `main` from its own branch after sufficient validation.

Before considering the branch ready:

- inspect the full diff against `main`;
- remove accidental/unrelated changes;
- verify generated files are intentional;
- run relevant static/build checks;
- perform server/in-game testing where applicable;
- document known limitations;
- check applicable release acceptance requirements.

Testing the combination on `dev` may be required, but `dev` itself is not normally the production merge source.

---

## Keep branches focused

Avoid branches that gradually accumulate:

- unrelated bug fixes;
- broad cleanup;
- speculative refactoring;
- formatting churn;
- unrelated experimental systems;
- several unrelated features.

If scope materially expands, split it.

---

## Git safety

Before modifying the repository, inspect:

- current branch;
- `git status`;
- relevant uncommitted changes;
- relevant history/diff when needed.

Never discard user work.

Do not use destructive operations such as:

- `git reset --hard`;
- destructive checkout/restore;
- branch deletion;
- force push;
- history rewriting;

unless explicitly requested and clearly appropriate.

Do not automatically stash unrelated user work merely to make the workspace convenient.

If unrelated local modifications exist, leave them alone.

---

## Commits

Keep commits coherent and reviewable.

Prefer:

- one conceptual change per commit;
- descriptive commit messages;
- minimal unrelated formatting changes;
- no temporary/generated/debug files unless intentionally tracked.

Do not amend or rewrite existing shared commits unless explicitly requested.

---

# DayZ source and API discipline

Do not invent:

- DayZ APIs;
- callbacks;
- inheritance relationships;
- replication behavior;
- serialization behavior;
- engine guarantees.

When uncertain, inspect current authoritative/public sources first.

Preferred reference:

- `BohemiaInteractive/DayZ-Script-Diff`

Historical mods may be inspected for behavior or ideas, but their architecture must not automatically be copied.

Code from another mod proves what that mod did, not what PaintZ should do.

When verified current APIs conflict with assumptions in PaintZ documentation, report the conflict and update the relevant design rather than forcing obsolete assumptions.

---

# Third-party mod compatibility

Treat third-party content as unknown until inspected.

Do not assume a modded weapon follows vanilla hidden-selection conventions.

Compatibility should come primarily from:

- runtime type relationships;
- actual runtime hidden selections;
- generic safe heuristics;
- optional administrator policy overrides.

Do not add a compatibility patch merely because PaintZ initially fails on a specific mod.

First determine why generic detection failed.

If a third-party model cannot be painted safely using PaintZ's runtime approach, report it as unsupported.

---

# Coding standards

PaintZ modifies objects owned by DayZ and other mods. Favor predictability and conservative behavior.

- Keep helpers small and auditable.
- Prefer explicit names.
- Keep responsibilities narrow.
- Avoid unnecessary inheritance.
- Avoid unnecessary framework layers.
- Avoid unnecessary dependencies.
- Avoid speculative abstractions.
- Centralize shared rules.
- Validate external data at boundaries.
- Fail safely.
- Keep server/client responsibilities explicit.
- Avoid per-frame work when cached/event-driven logic is sufficient.
- Avoid repeated file parsing in hot paths.
- Avoid unnecessary RPC traffic.
- Log useful decisions in debug builds/modes.
- Avoid noisy production logging.

Comments should explain **why**, especially around DayZ engine behavior or unusual constraints.

Do not merely restate obvious code.

---

# Error handling

Different failure causes should remain distinguishable.

Useful reasons include:

- spray can is ruined;
- target item is ruined;
- insufficient paint remains;
- unsupported target type;
- target excluded by configuration;
- target has no hidden selections;
- all candidate selections are blocked;
- several ambiguous selections remain;
- finish definition is invalid;
- texture is unavailable;
- runtime configuration reload failed.

Do not collapse materially different problems into an unexplained generic `false`.

Player-facing messages should remain concise.

Detailed diagnostics belong in debug/server logs.

---

# Debugging

Debug logging should make PaintZ decisions traceable.

Useful information may include:

- target classname/type;
- runtime target category;
- target health state;
- spray-can health state;
- discovered hidden selections;
- rejected selections and reasons;
- selected hidden selection/index;
- finish ID;
- include/exclude policy result;
- matched runtime configuration rule;
- configuration reload outcome;
- synchronization/persistence information where relevant.

Do not enable uncontrolled high-frequency debug spam in production.

---

# Testing and definition of done

## Never claim tests that were not performed

Clearly distinguish between:

- code inspection;
- static validation;
- generator execution;
- successful PBO/build;
- automated tests;
- dedicated-server startup;
- multiplayer testing;
- actual in-game interaction testing.

Do not describe code as "tested" merely because it compiles or looks correct.

If the environment prevents a test, state what was not tested.

---

## Validate the focused branch first

Test a change on its own development branch where practical.

This confirms that it works independently of unrelated changes.

Use `dev` afterward when combined/integration testing is needed.

---

## Runtime painting tests

For changes affecting paint actions, test representative cases where practical:

- valid usable spray can;
- ruined spray can;
- valid weapon;
- ruined weapon;
- valid detachable magazine;
- ruined magazine;
- target with one obvious body selection;
- target with several hidden selections;
- target with glass/optic/emissive selections;
- incompatible target;
- excluded target;
- wildcard-included target;
- wildcard-excluded target;
- modded weapon with nonstandard selection names;
- insufficient spray-can quantity;
- server/client synchronization.

A single successful test on one AK/M4 family is not proof of generic compatibility.

---

## Runtime configuration tests

When changing configuration logic, test at least:

- no configuration file where supported;
- valid configuration;
- empty include/exclude lists;
- wildcard include;
- wildcard exclude;
- overlapping include/exclude;
- exclusion precedence;
- type/category filtering;
- malformed JSON;
- malformed rule;
- unknown target category;
- duplicate/conflicting rules;
- reload of valid configuration;
- reload of invalid configuration;
- retention of last-known-good configuration after failure.

Do not claim runtime configuration is robust after testing only the happy path.

---

## Release acceptance

Before claiming production readiness, check applicable requirements in:

`docs/RELEASE_ACCEPTANCE.md`

Do not bypass an acceptance requirement because the current environment cannot perform it.

Report the missing validation instead.

---

# Documentation

Keep `AGENTS.md` focused on durable project rules.

Do not continually add temporary state such as:

- current number of paints;
- today's active branch;
- one-off bugs;
- temporary test results;
- current generator output count.

Put detailed evolving specifications in dedicated documentation.

Examples:

- `docs/ARCHITECTURE.md`
- `docs/RUNTIME_CONFIG.md`
- `docs/RELEASE_ACCEPTANCE.md`
- feature-specific design documents where useful.

When architecture changes deliberately, update the relevant documentation as part of that logical change.

If a rule in this file becomes obsolete because the architecture intentionally changes, update the rule rather than coding around stale instructions.

---

# Agent operating procedure

Before beginning substantial work:

1. read this file;
2. inspect the current Git branch;
3. inspect `git status`;
4. determine whether an existing branch already owns the requested work;
5. inspect relevant implementation and documentation;
6. identify generated files;
7. verify uncertain DayZ APIs before relying on them;
8. understand the requested behavior before editing.

For substantial or risky work, form an implementation plan before making changes.

During implementation:

- stay within scope;
- preserve unrelated user work;
- reuse established architecture where appropriate;
- verify assumptions rather than guessing;
- keep branch responsibilities clear;
- update documentation/config/tests alongside code when required.

Before finishing:

1. inspect `git status`;
2. inspect the complete diff;
3. check for unintended files or unrelated edits;
4. run available relevant validation;
5. compare against applicable acceptance requirements;
6. report what was and was not tested.

---

# Final work report

When completing development work, report concisely:

- branch used;
- major files changed;
- behavior implemented or changed;
- validation actually performed;
- tests that could not be performed;
- known limitations or risks;
- whether `dev` integration testing is recommended.

Do not claim completion merely because code was written.

---

# Forbidden shortcuts

Unless explicitly requested as part of a deliberate architecture change, do not:

- create per-weapon painted subclasses;
- replace target objects during painting;
- create a growing per-class compatibility database;
- hard-code support solely to make one specific weapon pass;
- allow configuration to bypass safe hidden-selection detection;
- trust client-provided texture/material paths;
- trust client-provided target eligibility;
- allow ruined spray cans to paint;
- allow ruined targets to be painted;
- silently replace valid runtime configuration with malformed configuration;
- create an unnecessarily complex rule language;
- regenerate different production assets from identical inputs;
- manually patch generated files instead of their sources;
- redesign persistence during unrelated work;
- commit unrelated cleanup;
- develop normal features directly on `main`;
- use `dev` as the permanent home of feature-specific fixes;
- hide failed or unperformed testing;
- work around an engine limitation by violating PaintZ's generic runtime architecture.

When the generic architecture genuinely cannot support a target or feature, report the limitation and propose the smallest extension that preserves PaintZ's design.