# Native paint persistence

PaintZ uses native DayZ entity persistence. It has no Community Framework
dependency and does not keep a sidecar database. The assignment remains on the
same physical entity, so the normal classname, health, ammunition, chamber,
attachments, cargo, and inventory relationships remain owned by vanilla DayZ.

## Supported hooks

- `Weapon_Base` appends PaintZ state after its existing weapon/FSM save chain.
- `MagazineStorage` appends PaintZ state after its existing magazine save chain.

Current DayZ 1.29 public scripts define `InventoryItemSuper` as an `ItemBase`
typedef, so state and network synchronization can be reused at the `ItemBase`
layer. Persistence is nevertheless separate because weapons and detachable
magazines implement distinct native save behavior. `Magazine` itself is an
engine class that the script compiler refuses to mod; `MagazineStorage` is its
nearest script-modifiable common descendant and covers the standard detachable
magazine classes. Future paintable inventory categories are not
persistence-enabled until their actual DayZ hierarchy and save behavior are
reviewed and a suitable generic hook is added.

Third-party descendants gain persistence when they use these normal hierarchies
(`Weapon_Base` or `MagazineStorage`) and preserve the vanilla `super` chain. A
magazine derived directly from the unmodifiable engine `Magazine` class, a mod
that omits `super`, or a mod that appends an incompatible custom stream around
the same hook is an external compatibility limitation; flat native serializer
extensions cannot be made order-independent.

## Serialized format

Every supported weapon or magazine writes three values after vanilla state:

1. marker string: `PaintZ.PaintState`;
2. PaintZ schema version: integer `1`;
3. one string-array payload: empty for unpainted, or one canonical finish ID.

The DayZ save version passed to `OnStoreLoad` is not used as PaintZ's schema
version. Texture paths, materials, hidden-selection indexes, eligibility results,
and runtime policy are never stored. Keeping the version payload inside one
serializer value lets a future loader consume an unsupported version without
partially interpreting its fields.

After the vanilla load chain, no remaining serializer value means a normal
pre-feature entity and produces no warning. A valid block restores its logical
finish immediately. `AfterStoreLoad` resolves the current finish catalogue,
redetects the safe hidden selection, applies the appearance once, and publishes
derived state to clients. Current include/exclude policy is deliberately not
consulted during restore, so existing paint survives policy changes and remains
strippable.

Unknown finish IDs remain stored in memory and are written back unchanged. They
do not render or become an implicit strip; PaintZ logs one concise warning during
the restore attempt. Restoring the definition before a later restart allows the
finish to render again.

## Accepted removal limitation

PaintZ must remain loaded during persistence save cycles where its state must be
preserved. If a server loads and saves the database without PaintZ, the native
PaintZ block may be discarded permanently. Re-enabling PaintZ later does not
guarantee recovery. Back up persistence before testing removal; no clean
uninstallation or recovery guarantee is provided.

## Manual restart acceptance sequence

Use a disposable server persistence copy and one known paintable weapon and
detachable magazine.

1. Start with PaintZ enabled. Paint both items with finish A; partially load the
   magazine and insert it into the weapon. Record classnames, health, ammo count,
   chamber/attachment state, and finish.
2. Save, stop, and restart with PaintZ enabled. Verify the same items, vanilla
   state, magazine association, and finish. Join with a client only after server
   load and verify the late client sees both finishes.
3. Repeat with loose items on the ground, in player inventory, persistent and
   nested storage, and vehicle cargo.
4. Repaint A to B, save/restart, and verify B. Strip both, save/restart, and
   verify they remain unpainted.
5. Paint while allowed, save, exclude the classname in runtime policy, restart,
   and verify the old finish restores, replacement follows policy, and stripping
   remains available.
6. On a test build, persist a known ID, remove only its catalogue definition,
   restart, and verify the warning and usable item. Restore the definition and
   restart to verify the finish returns.
7. Load a backup containing items saved before this feature and verify they load
   as unpainted without errors or warning spam.
8. On a separate backup only, save painted items, remove PaintZ, run and save the
   server, then restore PaintZ. Confirm this sequence is treated as destructive:
   recovery is not promised.
