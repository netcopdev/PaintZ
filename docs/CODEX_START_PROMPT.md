# Initial Codex task

Copy the task below into the first Codex thread for this repository.

---

Read `AGENTS.md`, `README.md`, and `docs/RELEASE_ACCEPTANCE.md` before editing anything.

Audit the current PaintZ implementation against the current public BohemiaInteractive/DayZ-Script-Diff API, then make the smallest changes needed for a compile-ready DayZ mod.

The goal is a production-ready painting mod with generic runtime compatibility detection that never names individual weapon classes.

Hard constraints:

- no per-weapon `modded class` blocks;
- no weapon classname allowlist/registry;
- no painted item subclasses;
- preserve target object/classname;
- target only weapons and detachable magazines;
- inspect `hiddenSelections[]` from the actual target at runtime;
- use only a global selection-name heuristic;
- compatible target -> `Paint Woodland`;
- incompatible target -> `Cannot Paint` + reason;
- re-inspect on server when completing the paint action;
- synchronize the visual to clients in-session;
- do not implement persistence yet;
- do not invent APIs: verify uncertain calls/classes against current DayZ-Script-Diff.

Woodland now uses a generic camouflage PAA; preserve it and the shared FDE, Black, and OD finish path.

First, report any API/compile problems you find. Then patch them. Finally, give me:

1. changed-file summary;
2. exact DayZ Tools build/test steps;
3. expected RPT debug lines;
4. a test matrix using at least one vanilla rifle, one vanilla magazine, and one third-party weapon that is never named in PaintZ source.

Do not broaden scope unless required to make the mod compile and run.

---
