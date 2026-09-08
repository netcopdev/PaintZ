# Runtime item policy

PaintZ uses `$profile:PaintZ/paintz_items.json` as an administrative policy for
new paint applications. This policy is intentionally separate from generic model
safety: an include rule permits PaintZ to consider an item, but cannot make an
unsafe or ambiguous hidden selection paintable.

On first server startup PaintZ copies these bundled files to `$profile:PaintZ/`:

```text
PaintZ/
|-- paintz_items.json
`-- paintz_items_README.txt
```

Neither existing file is overwritten. The JSON stays machine-oriented; operational
instructions and examples belong in the adjacent README.

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
      "type": "weapon",
      "class_pattern": "*crossbow*"
    }
  ]
}
```

- `version` is required and must be `1`.
- `reload_seconds` is required. A positive integer schedules periodic reloads;
  `-1` means startup-only. `0` and values below `-1` are normalized to `-1` with
  a warning, preventing a rapid reload loop.
- `default_action` is required and must be `allow` or `exclude`.
- `domains` is optional for existing version-1 files. Missing or empty uses the
  shipped weapon/detachable-magazine defaults.
- `rules` is an array of rule objects. An absent or empty array means
  `default_action` is the complete policy.

JSON requires double quotes around property names and string values. Comments,
unquoted keys, trailing commas, and pseudo-objects such as
`{ action: exclude, type: weapon }` are invalid.

## Target domains

Domain entries are OR. A `type` and `class_pattern` in one entry are AND. Each
entry requires at least one field. `type` uses DayZ config inheritance;
`class_pattern` uses the same case-insensitive `*` and `?` matcher as policy.

The default `Weapon_Base` domain uses the runtime weapon type. The default
`Magazine_Base` domain uses DayZ's native `Magazine` runtime type and rejects
`IsAmmoPile()`, preserving detachable-magazine scope.

Domains control new-paint relevance only. Policy exclusion does not remove an
object from its domain, and neither domains nor policy are consulted when
stripping existing PaintZ paint.

Because DayZ's JSON loader does not preserve the distinction between an omitted
array and an explicit empty array, both select the safe defaults. To intentionally
match no objects, configure a valid nonmatching positive rule such as
`{"class_pattern":"PaintZ_Disabled_*"}`.

## Rule evaluation and precedence

Rules are evaluated from top to bottom. Each matching rule replaces the current
decision, so the last matching rule wins. This permits a broad exclusion followed
by a narrower inclusion. Duplicate and overlapping rules are legal and follow the
same ordering. A rule that does not match the runtime item category is ignored.

`action` accepts `include` or `exclude`. `type` accepts `weapon`, `magazine`, or
`all`. Category detection uses the runtime DayZ type, not classname text.

Each rule must use exactly one selector:

- `class_pattern` performs case-insensitive matching against the complete runtime
  classname. `*` matches zero or more characters and `?` exactly one. No other
  glob or regular-expression syntax is supported. A pattern without wildcards is
  an exact match.
- `inherits` performs a case-insensitive lookup in the appropriate DayZ config
  hierarchy and matches descendants through `IsKindOf`. The named config class
  must exist for the rule's category.

For example:

```json
{
  "version": 1,
  "reload_seconds": 60,
  "default_action": "allow",
  "rules": [
    {
      "action": "exclude",
      "type": "weapon",
      "class_pattern": "TTC_*"
    },
    {
      "action": "include",
      "type": "weapon",
      "class_pattern": "TTC_AK*"
    },
    {
      "action": "exclude",
      "type": "magazine",
      "inherits": "Example_MagazineBase"
    }
  ]
}
```

## Reload and failure behavior

PaintZ parses and validates a detached candidate configuration. Only a completely
valid candidate replaces the active policy and its decision cache. Invalid JSON,
an unsupported version, an invalid enum value, a null/non-object rule, an invalid
selector, or an unknown inheritance class rejects the whole candidate.

A failed periodic reload retains the last-known-good policy and uses its reload
interval for the next attempt. If startup cannot load any valid policy, PaintZ
fails closed and denies new paint applications. Rejection logs point operators to
`$profile:PaintZ/paintz_items_README.txt`.

## Existing items and client action visibility

Policy changes affect new painting and repainting only. Excluding an already-painted
item does not alter its texture or paint state, and Strip Paint remains available.
Allowing the class later permits painting again.

The server remains authoritative and rechecks policy when the action completes.
The active policy is also synchronized to clients so excluded targets do not offer
a Paint action in the interaction menu. A client that has not yet received a valid
policy fails closed for action visibility.

## Troubleshooting

- If a config is rejected, find the first `Config rejected` line in the server log;
  rule indices are zero-based.
- Validate the file with a strict JSON parser. A `.json` extension does not make
  JavaScript-style object notation valid JSON.
- Put a narrow exception after a broad rule because the last match wins.
- Use `type: "all"` deliberately; category-specific rules are easier to audit.
- An included item can still be unsupported when its runtime model exposes no safe
  paintable hidden selection. That is expected and cannot be bypassed by policy.
