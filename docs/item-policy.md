# Runtime item policy

PaintZ uses `$profile:PaintZ/paintz_items.json` as an administrative policy for **new** paint applications. This policy is intentionally separate from generic model safety and from persisted PaintZ state.

## Core rule

**Domains are data, not code.** Weapons and detachable magazines are only the shipped defaults. Adding another ordinary `ItemBase`-derived inventory category should require JSON changes only.

## JSON contract

```json
{
  "version": 1,
  "reload_seconds": 60,
  "default_action": "allow",
  "domains": [
    { "type": "Weapon_Base" },
    { "type": "Magazine_Base" }
  ],
  "rules": [
    {
      "action": "exclude",
      "type": "Weapon_Base",
      "class_pattern": "*crossbow*"
    }
  ]
}
```

- `version` must be `1`.
- `reload_seconds`: positive integer for periodic reload, `-1` for startup-only. `0` and values below `-1` are normalized to `-1` with a warning.
- `default_action`: `allow` or `exclude`.
- `domains`: positive OR-list controlling whether PaintZ considers a target relevant for new painting/feedback.
- `rules`: ordered include/exclude rules. Last matching rule wins.

## Domains

Each domain may contain:

- `type`: DayZ base/config class matched by inheritance;
- `class_pattern`: case-insensitive runtime classname glob using `*` and `?`;
- or both (AND).

Generic script roots recognized by PaintZ include `ItemBase`, `InventoryItemBase`, `InventoryItemSuper`, `Weapon_Base` and `Magazine_Base`. `Magazine_Base` excludes ammo piles.

Example expansion with no code change:

```json
"domains": [
  { "type": "Weapon_Base" },
  { "type": "Magazine_Base" },
  { "type": "SomeSuppressorBase" },
  { "type": "SomeClothingBase" }
]
```

A `class_pattern`-only domain is valid when a useful common base class is unavailable.

Missing or empty domains currently select the shipped weapon/magazine defaults for backward compatibility.

## Rules

Each rule has:

- `action`: `include` or `exclude`;
- optional `type`: `all`, any valid DayZ base/config class, or legacy `weapon` / `magazine` aliases;
- exactly one selector: `class_pattern` or `inherits`.

Examples:

```json
{
  "action": "exclude",
  "type": "Weapon_Base",
  "class_pattern": "TTC_*"
}
```

```json
{
  "action": "exclude",
  "type": "SomeClothingBase",
  "inherits": "SomeJacketBase"
}
```

```json
{
  "action": "exclude",
  "class_pattern": "BrokenPaint_*"
}
```

The final example applies to every configured target because omitted `type` is normalized to `all`.

## Separation from persistence

Domains and rules decide whether a **new** paint application may occur. They do not define which categories can carry PaintZ state.

If an item was painted while eligible and later its domain/rule changes:

- existing paint remains;
- persistence restoration remains active;
- Strip Paint remains available;
- only new/replacement painting follows the current policy.

## Reload behavior

PaintZ validates a detached candidate config and swaps it atomically only after successful validation. A failed periodic reload retains the previous valid policy. If no valid startup policy exists, PaintZ fails closed for new painting.

## Troubleshooting

- Use strict JSON; comments and trailing commas are invalid.
- Put a narrower exception after a broad rule because the last match wins.
- `include` never bypasses hidden-selection safety.
- An item can be domain-relevant but technically unsupported if PaintZ cannot infer a safe body selection.
