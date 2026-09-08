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
Separate entries are OR. Within one entry, type and class_pattern are AND. At
least one field is required.

type              DayZ base/config class matched by inheritance. PaintZ also
                  recognizes the generic script roots ItemBase,
                  InventoryItemBase, InventoryItemSuper, Weapon_Base and
                  Magazine_Base. Magazine_Base excludes ammo piles.
class_pattern     Case-insensitive runtime classname match using * and ?.

The shipped defaults are {"type":"Weapon_Base"} OR
{"type":"Magazine_Base"}. They are defaults only, not hard-coded PaintZ
categories. A normal inventory family can be added by adding another domain,
for example {"type":"SomeClothingBase"}, with no PaintZ script change.
A class_pattern-only domain is also valid when a useful config base is not
available.

Domains control new painting and feedback only. To intentionally match no
objects, use a valid nonmatching domain such as
{"class_pattern":"PaintZ_Disabled_*"}.

Rules
-----
Rules run from top to bottom. Every matching rule replaces the current result,
so the last matching rule wins.

action            "include" or "exclude".
type              Optional scope. Omit it or use "all" for all configured
                  domains. Otherwise use any valid DayZ base/config class, such
                  as Weapon_Base, Magazine_Base, Clothing, ItemBase, etc.
                  Legacy values "weapon" and "magazine" remain accepted for
                  existing version-1 configs.
class_pattern     Case-insensitive classname match. * matches zero or more
                  characters; ? matches exactly one character. With no wildcard,
                  the whole classname must match.
inherits          DayZ config base class. A target matches when its runtime
                  classname inherits from this class.

Each rule must contain exactly one selector: class_pattern or inherits.

Valid JSON examples
-------------------
Exclude a weapon family:
{
  "action": "exclude",
  "type": "Weapon_Base",
  "class_pattern": "TTC_*"
}

Allow a narrower family again by placing this later in rules:
{
  "action": "include",
  "type": "Weapon_Base",
  "class_pattern": "TTC_AK*"
}

Exclude a configured clothing family:
{
  "action": "exclude",
  "type": "SomeClothingBase",
  "class_pattern": "Example_*"
}

A rule that applies to every configured target domain can omit type:
{
  "action": "exclude",
  "class_pattern": "BrokenPaint_*"
}

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
