PaintZ runtime item policy
==========================

paintz_items.json controls which targets receive PaintZ interaction feedback and
whether they may receive NEW PaintZ paint applications. PaintZ creates both
files in $profile:PaintZ on first server startup. Existing files are never
overwritten.

Keep paintz_items.json as strict JSON. JSON property names and string values need
double quotes. This README is documentation; do not copy explanatory text into
the JSON file.

Top-level fields
----------------
version          Must be 1.
reload_seconds   Positive whole number: reload at approximately that interval.
                 -1: load only at server startup. 0 and values below -1 are
                 normalized to -1 with a warning.
default_action   "allow" or "exclude". This is the initial result.
domains          Target-domain array. Missing or empty uses the shipped
                 weapon/detachable-magazine defaults.
rules            JSON array of rule objects. Missing or empty means that
                 default_action is the complete policy.

Target domains
--------------
Separate entries are OR. Within one entry, all supplied fields are AND. At
least one field is required.

type                    DayZ base/config class matched by inheritance. PaintZ
                        also recognizes ItemBase, InventoryItemBase,
                        InventoryItemSuper, Weapon_Base and Magazine_Base.
                        Magazine_Base excludes ammo piles.
class_pattern           Case-insensitive runtime classname match using * and ?.
inventory_slot          Case-insensitive exact match against one of the item's
                        declared compatible inventorySlot values.
inventory_slot_pattern  Case-insensitive * / ? match against the item's declared
                        compatible inventorySlot values.

Slot selectors inspect the class config, not the item's current attachment
location. A loose stock, optic, suppressor or flashlight on the ground can still
match the slots it declares it can occupy.

The shipped config includes Weapon_Base, Magazine_Base, normal weapon/pistol
attachment slot families, and SmallProtectorCase. These are defaults only, not
hard-coded PaintZ categories. Other ordinary ItemBase families can be added in
JSON with no new PaintZ state/persistence/dispatch code.

Examples:
  {"inventory_slot_pattern":"weapon*"}
  {"inventory_slot_pattern":"pistol*"}
  {"inventory_slot":"weaponOptics"}
  {"class_pattern":"SmallProtectorCase"}

Domains control new painting and feedback only. To intentionally match no
objects, use a valid nonmatching domain such as
{"class_pattern":"PaintZ_Disabled_*"}.

Rules
-----
Rules run from top to bottom. Every matching rule replaces the current result,
so the last matching rule wins.

action                  "include" or "exclude".
type                    Optional scope. Omit it or use "all" for all configured
                        domains. Otherwise use any valid DayZ base/config class.
                        Legacy values "weapon" and "magazine" remain accepted.
class_pattern           Case-insensitive classname selector using * and ?.
inherits                DayZ config inheritance selector.
inventory_slot          Exact declared-slot selector.
inventory_slot_pattern  Wildcard declared-slot selector.

Each rule must contain exactly one selector from class_pattern, inherits,
inventory_slot, or inventory_slot_pattern. type is a scope, not a selector.

Valid JSON examples
-------------------
Exclude a weapon family:
{
  "action": "exclude",
  "type": "Weapon_Base",
  "class_pattern": "TTC_*"
}

Exclude everything declaring a particular attachment slot:
{
  "action": "exclude",
  "inventory_slot": "weaponOptics"
}

Allow a narrower slot family again by placing this later in rules:
{
  "action": "include",
  "inventory_slot_pattern": "weaponOptics*"
}

A rule that applies to every configured target domain can omit type:
{
  "action": "exclude",
  "class_pattern": "BrokenPaint_*"
}

Model safety
------------
Policy decides relevance/eligibility; the actual model still decides whether an
item can be painted. PaintZ does not hard-exclude item families such as optics or
flashlights. If an eligible item exposes a safe body/camo/housing selection it may
paint. Functional selections such as glass, lenses, reticles, displays and
emissive surfaces remain protected by the generic model-safety heuristic.

Failure and existing-item behavior
----------------------------------
Invalid JSON, an invalid field value, or any invalid rule rejects the entire new
configuration. A failed periodic reload keeps the last valid policy. If no valid
startup policy can be loaded, new painting fails closed.

Exclusion prevents the Paint action from being offered and prevents server-side
completion. It does not remove or reset paint already on an item. Already-painted
items may still be stripped after exclusion or removal from all domains. Allowing
the class later permits painting and repainting again.

Persistence is independent from domains. Any ordinary ItemBase-derived inventory
item that acquires PaintZ state uses the same persistence mechanism; adding a new
such category does not require a new persistence hook.

Full documentation and troubleshooting: docs/item-policy.md in the PaintZ source
repository.
