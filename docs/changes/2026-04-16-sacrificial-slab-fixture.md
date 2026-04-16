# Sacrificial Slab Fixture

**Date:** 2026-04-16

## Summary

Added the Sacrificial Slab fixture (#30 in the fixture catalog) — a grand ritual site found on deep levels (D10+).

## Design

Custom layout (`applySacrificialSlabLayout()` in Architect.c) placing:
- **Marble cross** (5 tiles, plus shape) at center — the ritual slab
- **Blood ring** (4+ tiles) surrounding the cross with decreasing probability outward
- **Luminescent fungus** (perimeter) for eerie bioluminescent glow
- **Bones** (2-4) scattered among the blood — remains of the sacrificed
- **Embers** (2-3) as ritual candles at outer positions
- **Potion loot** (~30% chance) near the slab

Requires a 5x5+ open interior area. Uses `BP_PURGE_INTERIOR` to clear foliage.

## Files Changed

- `src/brogue/Rogue.h` — Added `MT_FIXTURE_SACRIFICIAL_SLAB` to machineTypes enum
- `src/brogue/Architect.c` — Added `applySacrificialSlabLayout()` and dispatch in `buildAMachine()`
- `src/variants/GlobalsBrogue.c` — Blueprint entry and auto-generator (freq 17, D10+)
- `src/test/test_fixtures.c` — 4 tests (depth range, custom layout, placement, cross pattern)
- `docs/plans/fixtures.md` — Marked #30 as ✅

## Verification

- All 315 tests pass
- Seed scanner finds hits (seeds 100, 104, 167, 274, etc. on D10)
- Example: `./bin/scan-fixture 102 -seed 274`
