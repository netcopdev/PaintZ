PaintZ stale finish recovery
============================

Runtime config:
  $profile:PaintZ/paintz_stale_finishes.json

PaintZ creates the runtime JSON from the bundled default when it does not exist. An existing administrator JSON is never overwritten. This README is refreshed from the bundled copy at server startup.

Purpose
-------
A stale finish is a persisted PaintZ finish ID that is not registered by any currently loaded and valid PaintZ content pack.

The recovery policy is deliberately narrow. It does not remap registered finishes, does not change item eligibility policy, and does not scan the whole persistence database or every loaded entity when the JSON reloads.

Resolution order on encounter
-----------------------------
1. If the item's stored finish ID is currently registered, do nothing. A migration entry can never override a live registered finish.
2. If the stored finish ID is unregistered and has an exact migration entry, attempt that migration first.
3. A migration succeeds only when its destination finish is currently registered and the item has a safe paintable selection. Success permanently changes the item's logical PaintZ finish ID to the destination and applies that finish. The new ID is written by normal CF ModStorage on the next save.
4. If no migration entry exists and prune_unknown is true, clear the stale PaintZ state. PaintZ restores the configured/original texture when it can safely identify the relevant selection. If no safe selection can be recovered, the logical stale state is still cleared without guessing at a texture selection.
5. Otherwise preserve the stale logical state unchanged.

Migration failure is non-destructive. If the destination is unavailable, the item has no safe selection, or applying the destination fails, PaintZ leaves the stale state intact and logs a warning. prune_unknown is not used as a fallback after a matching migration fails.

JSON fields
-----------
version
  Required schema version. Current value: 1.

reload_seconds
  -1: load only at server startup.
  Positive integer: reload approximately at that interval in seconds.
  0 or values below -1 are treated as -1 with a warning.
  A failed reload leaves the previous valid configuration active.

prune_unknown
  false: preserve unregistered finish IDs that have no migration entry.
  true: clear unregistered finish IDs that have no migration entry when encountered.

  Default is false. Enabling this is intentionally destructive for encountered stale state. Do not enable it merely because a paint pack is temporarily missing unless that loss of historical assignments is intended.

migrations
  JSON object mapping one exact old finish ID to one exact replacement finish ID.

Example:

{
  "version": 1,
  "reload_seconds": 60,
  "prune_unknown": false,
  "migrations": {
    "PZ-C-FLK": "PZ-C-FTN",
    "NCP-S-OLD": "NCP-S-NEW"
  }
}

Migration rules
---------------
- IDs are normalized to uppercase.
- Source and destination must be exact IDs. Wildcards are not supported.
- Source and destination must differ.
- Migration chains are rejected. Do not configure OLD1 -> OLD2 and OLD2 -> NEW. Map both OLD1 and OLD2 directly to NEW.
- A destination does not have to be registered when the JSON is loaded, because loaded content can differ between server deployments. It must be registered when an item is encountered or that item's state is preserved.
- A source that is currently registered is never migrated, even if it appears in migrations.
- A matching migration takes precedence over prune_unknown. If that migration cannot be completed, PaintZ preserves the item rather than pruning it.

Reload and encounter semantics
------------------------------
Reloading this file does not enumerate all objects and does not mutate the persistence database globally. The new policy applies when stale state next passes through PaintZ's normal restoration/state-validation path.

Each item is evaluated at most once per successfully loaded config revision. A later successful JSON reload increments the revision, so the item can be reconsidered on its next encounter. This avoids repeated warnings from action-condition polling while still making runtime policy changes effective.

Typical workflow
----------------
For a deliberate finish-ID replacement:
1. Make the replacement finish available and registered.
2. Add OLD-ID -> NEW-ID to migrations.
3. Leave prune_unknown false unless unrelated stale assignments should also be removed.
4. Keep the mapping active long enough for relevant persisted items to be encountered and saved.
5. Remove the mapping when the migration window is no longer needed. Items already migrated keep the new ID; untouched old items remain stale until another recovery rule handles them.

For deliberate cleanup of abandoned finish IDs, set prune_unknown true only for the cleanup period. Registered finishes are unaffected.
