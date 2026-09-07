# Paint coats: feasibility review

Requested coverage is interpreted as 20%, 40%, 60%, 80%, then 100% paint over
the original finish. Literal repeated 20%-opacity compositing only reaches
about 67.2% coverage after five coats; a five-step system needs an explicit
total coverage value against the original texture.

## Finding

No supported lightweight way was found to do this generically with the current
DayZ 1.29 script API. The installed `dta/scripts.pbo` exposes
`EntityAI.SetObjectTexture(index, path)` and `SetObjectMaterial(index, path)`.
These replace the selection's texture or material. They do not expose an
original-texture/paint-texture blend factor or a script texture compositor.

Changing a paint texture's alpha is not equivalent to retaining the original
color texture beneath it. Alpha is interpreted by the active material; it can
affect surface visibility or be ignored. It does not create a second original
texture layer. Keeping the original normal/specular maps also does not preserve
the original diffuse texture's markings or color beneath a replacement.

Bohemia documents [procedural textures](https://community.bistudio.com/wiki/Procedural_Textures)
and [multi-layer materials](https://community.bistudio.com/wiki/Multimaterial),
but these are primarily RV/Arma references, not evidence that newer Arma UI or
extension texture generators work in DayZ. No such workaround was implemented.

## Cost of alternatives

- Baking blends offline needs the underlying source textures and many
  original-texture/finish/coverage combinations. That undermines compatibility
  with previously unknown modded items and is outside PaintZ's architecture.
- A material-based approach would need a demonstrated DayZ-compatible layered
  shader/material setup, both texture inputs, and preservation of the target's
  material maps. Five generic transparent materials alone do not solve this.
- Mixing finishes has the same rendering prerequisite, plus extra blend state
  and synchronization. It is not a small extension to the existing paint path.

One limited exception is mixing two known solid paint RGB colors: calculating
another procedural solid color is inexpensive. It would not reveal the original
texture, support camouflage pattern mixing, or provide the requested coats.
It would also need paint-state/strip detection updates because the resulting
color would no longer equal one of the five fixed finish textures. Discuss this
restricted behavior before implementing it; it has not been added.

Coat counts, transparent texture variants, replacement materials, and mixing
were deliberately not added. Painting remains one opaque application. Discuss
and prove a separate rendering experiment before changing that behavior.

## Changes made independently

- Painting remains unchanged: a different can overwrites the current finish.
- Test fixtures now use engine setup and rotation flags with `RF_DEFAULT`.
  The forced `90 0 0` orientation was removed because it overwrote the model's
  resting pose after its ground position had already been calculated.
  The helper explicitly applies the transform from `PlaceOnSurfaceRotated`,
  which is also used by the inventory drop path; spawn flags alone did not
  rotate the sampled items in the diagnostic test.

Placement references in the installed 1.29 scripts:
`3_game/ce/centraleconomy.c` documents `ECE_ROTATIONFLAGS` and `RF_DEFAULT`;
`3_game/systems/inventory/inventory.c` uses native drop placement transforms.

