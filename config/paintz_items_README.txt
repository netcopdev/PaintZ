PaintZ runtime item policy
==========================

paintz_items.json controls whether weapons and detachable magazines may receive
new PaintZ paint applications. PaintZ creates both files in $profile:PaintZ on
first server startup. Existing files are never overwritten.

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
rules            JSON array of rule objects. Missing or empty means that
                 default_action is the complete policy.

Rules
-----
Rules run from top to bottom. Every matching rule replaces the current result,
so the last matching rule wins.

action            "include" or "exclude".
type              "weapon", "magazine", or "all".
class_pattern     Case-insensitive classname match. * matches zero or more
                  characters; ? matches exactly one character. With no wildcard,
                  the whole classname must match.
inherits          Case-insensitive DayZ config base class. A target matches when
                  it inherits from this class.

Each rule must contain exactly one selector: class_pattern or inherits.

Valid JSON examples
-------------------
Exclude a weapon family:
{
  "action": "exclude",
  "type": "weapon",
  "class_pattern": "TTC_*"
}

Allow a narrower family again by placing this later in rules:
{
  "action": "include",
  "type": "weapon",
  "class_pattern": "TTC_AK*"
}

Exclude descendants of a configured magazine base class:
{
  "action": "exclude",
  "type": "magazine",
  "inherits": "Example_MagazineBase"
}

Failure and existing-item behavior
----------------------------------
Invalid JSON, an invalid field value, or any invalid rule rejects the entire new
configuration. A failed periodic reload keeps the last valid policy. If no valid
startup policy can be loaded, new painting fails closed.

Exclusion prevents the Paint action from being offered and prevents server-side
completion. It does not remove or reset paint already on an item. Already-painted
excluded items may still be stripped. Allowing the class later permits painting
and repainting again.

Full documentation and troubleshooting: docs/item-policy.md in the PaintZ source
repository.
