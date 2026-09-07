# Persistence notes

Persistence is not currently implemented.

## Why

The historical Reskin Manager writes its custom paint integer from a broad `Weapon_Base` `OnStoreSave()` override and reads it back from `OnStoreLoad()`. That demonstrates the concept, but copying it is not automatically safe for an established server.

Potential problems include:

- installing PaintZ onto persistence created before PaintZ existed;
- removing PaintZ after items were saved with extra fields;
- another mod/subclass serializing fields around the same base call;
- reading the wrong bytes when serialization layout changes;
- load failure if an expected custom field is absent.

## Required persistence research task

Before implementation, Codex should inspect current DayZ/Bohemia persistence patterns and propose at least two approaches, evaluated for:

1. existing-server install compatibility;
2. clean mod removal/revert behavior;
3. weapon and magazine support;
4. interactions with subclass serialization;
5. multiplayer join-in-progress state;
6. migration/versioning.

Only after choosing and documenting a safe mechanism should persistence be implemented.

## Persistence acceptance tests

At minimum:

- baseline persistence created with PaintZ absent -> install PaintZ -> load baseline;
- paint item -> restart -> paint remains;
- painted item in player inventory;
- painted item in ground storage;
- painted item in vehicle cargo;
- painted magazine attached to weapon where applicable;
- painted item in nested storage where allowed;
- remove PaintZ and verify documented rollback behavior on a backup copy.
