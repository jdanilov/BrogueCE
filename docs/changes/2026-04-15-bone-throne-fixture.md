# Bone Throne Fixture

**Date:** 2026-04-15

## Summary

Added the Bone Throne dungeon fixture — a grand, memorable landmark for deep levels (D10+). A STATUE_INERT throne sits on a marble dais with a carpet runner extending from it, while bones and blood are randomly scattered around the surrounding area for organic variety. ~30% chance of RING or GOLD loot.

## Implementation

- **Custom layout** (`applyBoneThroneLayout()` in Architect.c): Uses `applyRotatableLayout` for the fixed structural elements (throne, marble dais, carpet runner), then randomly scatters 3-5 bones and 3-5 blood stains on nearby interior floor cells.
- **Blueprint**: `MT_FIXTURE_BONE_THRONE` (index 97), depth 10–DEEPEST_LEVEL, roomSize {12,21}, BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR.
- **Auto-generator**: frequency 17, matching other fixtures.
- **Scanner update**: `tools/scan_fixture_seeds.c` now handles deep fixtures by regenerating the dungeon at the fixture's minimum depth instead of always scanning D1.

## Files Changed

- `src/brogue/Rogue.h` — Added `MT_FIXTURE_BONE_THRONE` to machineTypes enum
- `src/brogue/Architect.c` — Added `applyBoneThroneLayout()` and buildAMachine dispatch
- `src/variants/GlobalsBrogue.c` — Blueprint and auto-generator entries
- `src/test/test_fixtures.c` — 4 tests (depth range, custom layout, placement, carpet runner)
- `tools/scan_fixture_seeds.c` — Deep fixture depth support
- `docs/plans/fixtures.md` — Marked Bone Throne as ✅
