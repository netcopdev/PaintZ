# Runtime item policy

PaintZ uses `$profile:PaintZ/paintz_items.json` as an administrative policy for **new** paint applications. This policy is intentionally separate from generic model safety and from persisted PaintZ state.

## Core rule

**Domains are data, not code.** Weapons, magazines, attachments, optics, flashlights, containers, clothing and other ordinary `ItemBase` families must not require category-specific PaintZ state/persistence/dispatch code.

An item family may be introduced by JSON when its runtime model exposes a safe paintable hidden selection.

## JSON contract

```json
{
  "version": 1,
  "reload_seconds": 60,
  "default_action": "allow",
  "domains": [
    { "types": ["Weapon_Base", "Magazine_Base"] },
    { "inventory_slot_patterns": ["weapon*", "pistol*", "suppressor*"] },
    { "class_pattern": "SmallProtectorCase" }
  ],
  "rules": []
}
```

- `version` must be `1`.
- `reload_seconds`: positive integer for periodic reload, `-1` for startup-only. `0` and values below `-1` are normalized to `-1` with a warning.
- `default_action`: `allow` or `exclude`.
- `domains`: positive OR-list controlling whether PaintZ considers a target relevant for new painting/feedback.
- `rules`: ordered include/exclude rules. Last matching rule wins.

## Selector consistency

Where a selector can sensibly accept either one value or several alternatives, PaintZ exposes both a singular field and a plural array field. If both are present, they are one OR group.

For selector families shared by domains and rules, the JSON names and matching semantics must stay identical:

- `type` / `types`;
- `class_pattern` / `class_patterns`;
- `inventory_slot` / `inventory_slots`;
- `inventory_slot_pattern` / `inventory_slot_patterns`.

This is a schema invariant for future extensions. A new selector family that is valid in both domains and rules must not be given different names or different singular/plural behavior in the two places. When multiple alternatives make sense, add the plural form together with the singular form rather than forcing administrators to duplicate whole domain/rule objects.

Different selector families inside one object are **AND**. Values within one selector family are **OR**.

## Domains

Each domain may contain any combination of these selector families:

- `type`: one DayZ base/config class matched by inheritance;
- `types`: OR-list of DayZ base/config classes;
- `class_pattern`: one case-insensitive runtime classname glob using `*` and `?`;
- `class_patterns`: OR-list of classname globs;
- `inventory_slot`: one case-insensitive exact declared compatible `inventorySlot`;
- `inventory_slots`: OR-list of exact declared compatible slots;
- `inventory_slot_pattern`: one case-insensitive `*` / `?` glob against declared compatible `inventorySlot` values;
- `inventory_slot_patterns`: OR-list of declared-slot globs.

All supplied selector families inside one domain are AND. Singular and plural values inside one family are OR. Separate domain objects are OR.

For example:

```json
{
  "types": ["ItemBase", "Inventory_Base"],
  "class_patterns": ["TTC_*", "MMG_*"],
  "inventory_slot_patterns": ["*optic*", "*scope*"]
}
```

means:

`(ItemBase OR Inventory_Base) AND (TTC_* OR MMG_*) AND (*optic* OR *scope*)`

Singular and plural forms may also be mixed:

```json
{
  "type": "Weapon_Base",
  "types": ["Magazine_Base"],
  "class_pattern": "Vanilla_*",
  "class_patterns": ["TTC_*", "MMG_*"]
}
```

The type group is `Weapon_Base OR Magazine_Base`; the class group is `Vanilla_* OR TTC_* OR MMG_*`; the two groups are ANDed.

Slot matching uses the **class-declared compatible slots**, not the item's current inventory location. A stock, optic, suppressor or flashlight lying loose on the ground still matches the slot(s) its config says it can occupy.

PaintZ reads these values from the target's inherited config under `CfgWeapons`, `CfgMagazines` or `CfgVehicles`. It does not maintain a classname-to-slot database.

Generic script roots recognized by PaintZ include `ItemBase`, `InventoryItemBase`, `InventoryItemSuper`, `Weapon_Base` and `Magazine_Base`. `Magazine_Base` excludes ammo piles.

Additional examples:

```json
{ "inventory_slot_patterns": ["weapon*", "pistol*"] }
```

```json
{ "types": ["ItemBase", "Inventory_Base"], "inventory_slot_pattern": "weaponOptics*" }
```

```json
{ "class_patterns": ["SmallProtectorCase", "MyMod_*Case"] }
```

Missing or empty domains select the historical weapon/magazine fallback for backward compatibility. The bundled default file is broader and explicitly includes common weapon/pistol/suppressor attachment slot families plus `SmallProtectorCase`.

## Rules

Each rule has:

- `action`: `include` or `exclude`;
- optional type scope using `type`, `types`, or both;
- at least one non-type selector from the remaining selector families below.

Type scope accepts:

- omitted `type` and empty/omitted `types`, which normalize to `all`;
- `all`;
- any valid DayZ base/config class;
- legacy `weapon` / `magazine` aliases for backward compatibility.

`type` and `types` are one OR group. They are scope, not a qualifying selector by themselves.

Supported non-type selector families:

- `class_pattern`: one classname glob;
- `class_patterns`: OR-list of classname globs;
- `inherits`: one inheritance class;
- `inherits_any`: OR-list of inheritance classes;
- `inventory_slot`: one exact declared slot;
- `inventory_slots`: OR-list of exact declared slots;
- `inventory_slot_pattern`: one declared-slot glob;
- `inventory_slot_patterns`: OR-list of declared-slot globs.

### Matching semantics

Different selector groups inside the same rule are **AND**.

Multiple values inside the same selector group are **OR**.

If both the singular and plural form of one selector are present, they form one OR group.

Separate rules are still evaluated top-to-bottom and the **last matching rule wins**.

For example:

```json
{
  "action": "exclude",
  "class_patterns": [
    "ACOGOptic",
    "HuntingOptic",
    "MK4Optic*"
  ]
}
```

matches:

`ACOGOptic OR HuntingOptic OR MK4Optic*`

A compound rule:

```json
{
  "action": "exclude",
  "types": ["Weapon_Base", "Magazine_Base"],
  "class_patterns": [
    "TTC_*",
    "MMG_*"
  ],
  "inventory_slot_patterns": [
    "*optics*",
    "*scope*"
  ]
}
```

means:

`(Weapon_Base OR Magazine_Base) AND (TTC_* OR MMG_*) AND (*optics* OR *scope*)`

Singular and plural forms combine as OR:

```json
{
  "action": "exclude",
  "class_pattern": "Vanilla_*",
  "class_patterns": [
    "TTC_*",
    "MMG_*"
  ]
}
```

means:

`Vanilla_* OR TTC_* OR MMG_*`

Type scope behaves the same way:

```json
{
  "action": "exclude",
  "type": "Weapon_Base",
  "types": ["Magazine_Base", "Inventory_Base"],
  "class_pattern": "Example_*"
}
```

means:

`(Weapon_Base OR Magazine_Base OR Inventory_Base) AND Example_*`

Exact slot alternatives:

```json
{
  "action": "exclude",
  "inventory_slots": [
    "weaponOptics",
    "pistolOptics"
  ]
}
```

Wildcard slot alternatives:

```json
{
  "action": "exclude",
  "inventory_slot_patterns": [
    "*optics*",
    "*scope*"
  ]
}
```

Inheritance alternatives:

```json
{
  "action": "include",
  "inherits_any": [
    "Inventory_Base",
    "OpticBase"
  ]
}
```

Inheritance values must resolve to existing DayZ config classes. Empty strings inside plural arrays are invalid. Empty arrays add no alternatives and do not count as a selector.

A rule that applies to every configured target domain can omit both `type` and `types`:

```json
{
  "action": "exclude",
  "class_pattern": "BrokenPaint_*"
}
```

Omitted type scope is normalized to `all`.

## Optics, lights and other functional items

PaintZ does **not** hard-exclude optics, scopes, flashlights or other sensible inventory categories.

Policy may make any of them relevant. The model inspector then decides whether there is a safe paintable surface:

- body/camo/housing-like selections may be painted;
- clearly functional selections such as glass, lens, reticle, display, screen, LED/emissive/glow or flame surfaces remain protected;
- category words such as `optic` and `light` are not themselves grounds for rejection.

Therefore two optics from different mods may behave differently: one may paint because it exposes a body/camo selection, while another may remain unsupported because it exposes only lens/reticle surfaces or an ambiguous set of selections.

An administrative include never bypasses this model-safety layer.

## Separation from persistence

Domains and rules decide whether a **new** paint application may occur. They do not define which ordinary inventory categories can carry PaintZ state.

If an item was painted while eligible and later its domain/rule changes:

- existing paint remains;
- persistence restoration remains active;
- Strip Paint remains available;
- only new/replacement painting follows the current policy.

## Reload behavior

PaintZ validates a detached candidate config and swaps it atomically only after successful validation. A failed periodic reload retains the previous valid policy. If no valid startup policy exists, PaintZ fails closed for new painting.

Policy decisions are cached per runtime classname. This remains correct for slot selectors because declared compatible `inventorySlot` values are class/config data, not per-instance attachment state.

The server synchronizes every supported singular selector and plural selector array to clients so policy evaluation remains identical on both sides.

## Troubleshooting

- Use strict JSON; comments and trailing commas are invalid.
- Put a narrower exception after a broad rule because the last match wins.
- Remember: different selector groups in one object are AND; alternatives inside one group are OR.
- Use a plural field instead of duplicating whole domain/rule objects when the difference is only alternative values of the same selector family.
- `include` never bypasses hidden-selection safety.
- An item can be domain-relevant but technically unsupported if PaintZ cannot infer a safe body selection.
- If a modded attachment is not becoming relevant, inspect its actual declared `inventorySlot[]` values and match those instead of guessing from its classname.
