# Weapon Rack Fixture

Added the **Weapon Rack** dungeon fixture (MT_FIXTURE_WEAPON_RACK).

## Design

- **Depth**: D5–D18
- **Size**: 2–3 cells
- **Tiles**: STATUE_INERT (rack against wall) + JUNK (1–2 broken equipment at base)
- **Loot**: ~30% chance to spawn a random WEAPON item
- **Placement**: Wall-adjacent via custom layout (`applyWeaponRackLayout()`)

Custom layout finds the best wall-adjacent interior cell (preferring corners), places the rack there, then adds 1–2 junk tiles on adjacent non-wall floor cells. Connectivity check prevents blocking the level.

## Files Changed

- `src/brogue/Rogue.h` — Added `MT_FIXTURE_WEAPON_RACK` to machineTypes enum
- `src/brogue/Architect.c` — Added `applyWeaponRackLayout()` and dispatch in `buildAMachine()`
- `src/variants/GlobalsBrogue.c` — Blueprint and auto-generator entries
- `src/test/test_fixtures.c` — 4 tests (depth range, custom layout, placement, wall adjacency)
- `docs/plans/fixtures.md` — Marked as complete
