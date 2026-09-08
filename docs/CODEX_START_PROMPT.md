# Initial Codex task

Read `AGENTS.md`, `README.md`, `docs/ARCHITECTURE.md`, `docs/item-policy.md`, and `docs/RELEASE_ACCEPTANCE.md` before editing anything.

PaintZ is a **generic ItemBase painting framework**. Weapons and detachable magazines are only the shipped default domains.

Non-negotiable invariant:

> Adding another ordinary `ItemBase`-derived inventory category must require JSON configuration only. It must not require a new PaintZ state, synchronization, persistence, or dispatch implementation.

Hard constraints:

- no per-weapon/per-magazine compatibility blocks;
- no painted subclasses;
- no category-specific persistence hooks;
- no closed weapon/magazine enum as the extensibility architecture;
- preserve the existing object/classname;
- discover paintability from runtime hidden selections;
- keep hidden-selection safety generic and conservative;
- keep runtime policy separate from persisted historical paint state;
- persistence uses CF ModStorage once at `ItemBase`;
- do not replace CF persistence with broad native serializer appends without proving legacy/subclass stream safety;
- verify uncertain DayZ APIs and hierarchy against current `BohemiaInteractive/DayZ-Script-Diff` and current CF source.

A new suppressor, clothing, backpack, container or other normal inventory domain should be enableable by editing `paintz_items.json` only when its model is technically paintable.

Before completing any task, check that your change does not reintroduce weapon/magazine-specific architecture where a generic ItemBase solution exists.
