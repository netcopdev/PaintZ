# Release acceptance criteria

A release is accepted only when all mandatory checks below pass.
For server-only finish, stripper, and sandbox test results, see `FINISH_VALIDATION.md`.

## Mandatory runtime checks

- [ ] Mod compiles with the target DayZ version.
- [ ] `PaintZ_SprayCan_WDL` spawns and can be held.
- [ ] A vanilla weapon with a suitable hidden selection shows `Paint Woodland`.
- [ ] A detachable magazine with a suitable hidden selection shows `Paint Woodland`.
- [ ] A weapon/magazine with no safe hidden selection shows `Cannot Paint`.
- [ ] `Cannot Paint` gives a useful player message.
- [ ] Painting changes the existing object; classname is unchanged.
- [ ] Health is unchanged.
- [ ] Magazine ammo count/type is unchanged.
- [ ] Weapon chamber/magazine/attachments are unchanged.
- [ ] Dropping/picking up the item does not remove the appearance during the same server session.
- [ ] Another connected player sees the painted texture.
- [ ] Debug log identifies classname, all hidden selections, chosen selection/index, and reason.

## Architecture checks

- [ ] No specific weapon classname appears in PaintZ compatibility logic.
- [ ] No painted weapon/magazine subclasses exist.
- [ ] No per-weapon registration file exists.
- [ ] Test at least one third-party weapon unknown to PaintZ source code.
- [ ] If that third-party model has a suitable hidden selection, PaintZ paints it without a compatibility patch.

## Runtime item-policy checks

- [ ] A missing `$profile:PaintZ/paintz_items.json` is created from the bundled default; an existing file is not overwritten.
- [ ] Ordered `exclude TTC_*` then `include TTC_AK*` rules produce last-match-wins behavior.
- [ ] `*` and `?` are case-insensitive; patterns without wildcards match exact classnames only.
- [ ] Weapon-only rules do not affect magazines, and magazine-only rules do not affect weapons.
- [ ] `reload_seconds = -1` schedules no reload; a positive value reloads at approximately that interval.
- [ ] Malformed JSON or an invalid rule retains the previous valid policy during runtime reload.
- [ ] Exclusion during a running paint action prevents completion without consuming paint.
- [ ] Excluding an already-painted item leaves its finish unchanged and still allows Strip Paint.

## Not currently included

- persistence across server restart;
- uninstallation safety;
- production Woodland texture;
- arbitrary colors;
- optics/attachments/clothing;
- pattern copying from another item;
- workshop packaging/signing.

These are future enhancements outside the current release scope.

## Finish and stripper regression checks

- [ ] Woodland, FDE, Black, OD, and Multicam Tropic cans spawn and offer their matching paint action.
- [ ] Paint cans never offer Strip Paint, including on painted items.
- [ ] The dedicated stripper offers Strip Paint only on painted compatible items; stripping restores the default texture and hides the action again.
- [ ] The stripper removes every PaintZ finish and consumes quantity only on success; insufficient quantity rejects the action.
- [ ] If a second player strips first, completion rejects the stale strip action.
- [ ] Woodland shows an organic multicolor pattern, including after client synchronization.
- [ ] FDE is visually reviewed beside the stock SCR17 in the same lighting.
- [ ] OD is solid olive drab and distinct from patterned Woodland.
- [ ] Multicam Tropic shows its green tropical pattern after client synchronization.
- [ ] Player and fixtures spawn on runway concrete; every catalogued paint has a ground fixture, and the player has two full primary paint cans plus two strippers.
- [ ] Weapons and magazines lie in their normal resting pose without sinking into the runway.
- [ ] Magazine paint and strip availability agree on server, painting client, another client, and after network relevancy changes (existing native magazine synchronization limitation).

`tools/sandbox/PaintZ_FinishSmokeTest.c` provides opt-in server tests of every generated finish, exclusive stripper action conditions, paint/strip transitions, completion consumption, and the player loadout. It does not verify client action menus or multiplayer replication.
