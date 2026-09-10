# Paint Pack API v1 runtime acceptance

This checklist is the runtime acceptance gate for PaintZ's registry and content-pack interoperability. It does not replace `PAINT_PACK_API.md` or `PAINT_PACK_CONFIG_V1.md`.

Static review alone is insufficient for integration. Enforce Script and DayZ config must ultimately be built and loaded with DayZ Tools/DayZDiag.

## 1. Static preflight

Before runtime testing verify:

- PaintZ core declares exactly one official `PZ` namespace owner named `PZ_PaintZOfficial`;
- PaintZ core contains no individual official finish catalogue/assets/cans;
- an official PackKit-generated content pack emits no `CfgPaintZPacks` declaration;
- official `PZ-*` finishes use `owner = "PZ_PaintZOfficial"`;
- official content packs depend on `PaintZ_DynamicPaint`, not on another official content pack;
- normal third-party owner/satellite behavior remains available and separate from the official model.

## 2. Build PaintZ

From the current runtime branch:

```powershell
pwsh -File .\tools\build.ps1 -SkipPaintZGen
```

Acceptance:

- AddonBuilder completes successfully;
- PaintZ PBO is freshly produced;
- no PaintZ Enforce/config error occurs when loaded;
- the core `PZ_PaintZOfficial` owner is discoverable;
- PaintZ loads correctly with no official content pack installed.

## 3. Build external API fixture

```powershell
pwsh -File .\tools\sandbox\Build-PaintPackApiFixture.ps1
```

The fixture should contain valid third-party content plus deliberately invalid registrations for namespace/owner/finish collision testing.

## 4. Run runtime/smoke tests

Use the normal PaintZ sandbox launcher with the fixture and, where appropriate, Standard Pack or a temporary official test pack.

Search server/client logs for:

```text
[PaintZ][Smoke]
[PaintZ][PackAPI Smoke]
[PaintZ][WARNING] paint_pack_registry
SCRIPT (E)
SCRIPT (W)
```

There must be no unexpected PaintZ `FAIL` or PaintZ-caused VM/config exception.

## 5. Namespace and collision expectations

The runtime must produce deterministic results independent of mod load order.

Required behavior:

```text
PZ               active, owned by PZ_PaintZOfficial from PaintZ core
TST              active when exactly one valid fixture owner exists
DUP              disabled when two owner declarations claim it
PZA              rejected when a third-party declaration attempts to claim reserved PZ*
unsupported API  rejected
owner mismatch    finish rejected
duplicate ID      duplicated finish rejected
```

There must be no first-loaded-wins or last-loaded-wins overwrite behavior.

An external content pack that attempts to redeclare `PZ` must collide with the core owner and must not become an alternate owner.

## 6. Official-pack independence

Validate at least one official content pack generated with PackKit `--official`.

Its generated config must:

- omit `CfgPaintZPacks`;
- require `PaintZ_DynamicPaint`;
- register every finish against `PZ_PaintZOfficial`;
- use unique local config-child names;
- contain no dependency on Standard Pack or another official content pack.

Run three combinations where practical:

```text
PaintZ only
PaintZ + official test pack
PaintZ + Standard Pack + official test pack
```

Acceptance:

- PaintZ starts without Standard Pack;
- the official test pack works without Standard Pack;
- multiple official peer packs can contribute distinct `PZ-*` finishes simultaneously;
- a deliberate duplicate complete `PZ-*` ID is rejected without disabling unrelated unique finishes;
- no official content pack redeclares `PZ`.

## 7. Generic action/state results

The Pack API fixture must prove at least:

- an external spray can resolves its `paintzFinish` through the registry;
- the single generic `ActionPaintZPaint` accepts external-pack cans;
- painting stores the complete logical finish ID;
- explicitly registered runtime surfaces are applied;
- paint quantity uses current size-based tuning;
- Strip Paint removes external/official finishes through the same generic path;
- painting sets synchronized PaintZ state and stripping clears it;
- unresolved historical IDs retain logical state and remain strippable.

Official and third-party finishes must use the same generic runtime path after registry resolution.

## 8. Network safety

A registered finish may synchronize only when its finish-ID hash is unique among all valid registered finishes.

An unresolved finish must not synchronize an arbitrary raw hash that could accidentally resolve to an unrelated active finish. Historical PaintZ-state presence must remain representable so stripping remains possible.

A stripped/unpainted item must synchronize an unpainted state with no active finish resolution.

## 9. Manual visual checks

Verify at least:

1. one official `PZ-*` can paints a supported target;
2. one third-party test can paints through the same generic action;
3. patterned content resolves only explicitly registered scale variants;
4. Paint Stripper restores original appearance for official and third-party finishes;
5. unsupported/relevant-target feedback remains governed by current PaintZ policy/model-safety behavior.

## 10. Persistence / missing-pack tests

Before calling missing-pack persistence release-tested, use a disposable persistent server:

1. load PaintZ plus an official or third-party API-v1 content pack;
2. paint items in player inventory, nested storage and vehicle cargo;
3. restart and verify the registered finish restores;
4. remove only the content pack while retaining PaintZ + CF;
5. restart and verify underlying items load, unresolved logical IDs remain stored and stripping remains available;
6. restore the same content pack and restart;
7. verify the original finish ID resolves again on items that were not stripped;
8. verify stripped items remain stripped.

For official `PZ-*` finishes, removing Standard/Military/etc. must not remove the `PZ` namespace itself because the namespace belongs to PaintZ core.

## 11. Catalogue-move persistence check

To validate official package reorganization:

1. persist an item with a test `PZ-*` finish from official pack A;
2. remove that finish registration from pack A;
3. add the unchanged complete finish ID to official pack B;
4. load PaintZ + pack B;
5. verify the persisted finish resolves without migration/alias data.

The content-pack boundary must have no effect on canonical identity.

## Acceptance decision

The runtime is ready for integration review only when:

- PaintZ builds/loads with `PZ_PaintZOfficial` as the one official owner;
- PaintZ works with no official content pack installed;
- official PackKit output contributes to `PZ` without declaring an owner or requiring another official pack;
- third-party owner/satellite behavior remains valid;
- collision handling remains deterministic;
- generic paint/strip and synchronization behavior pass;
- persistence/missing-pack cases are either tested or explicitly recorded as still requiring live-server verification.
