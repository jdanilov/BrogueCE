# Lichen Garden Fixture

Added the Lichen Garden fixture (#23 in the fixture catalog) — a grand bioluminescent fungal archipelago.

## Design

Multiple shallow water pools connected by luminescent fungus bridges, surrounded by concentric rings of fungus forest and dead grass fringe. The luminescent fungus provides actual in-game light, making this a visually striking landmark.

Layout algorithm uses Chebyshev distance from water cells to paint growth rings:
- Distance 0: SHALLOW_WATER (pools)
- Distance 1: LUMINESCENT_FUNGUS (glowing bridges)
- Distance 2: FUNGUS_FOREST (dense growth)
- Distance 3: DEAD_GRASS (60% chance, organic fringe)

## Properties

- Depth range: D5-D18
- Room size: {12, 21} (large)
- Custom layout: `applyLichenGardenLayout()` in Architect.c
- 2-3 water pools with 1-2 satellite pools offset ~3 cells from center
- ~20-30 tiles total
- BP_PURGE_INTERIOR (clears foliage for visibility)

## Files Changed

- `src/brogue/Rogue.h` — Added `MT_FIXTURE_LICHEN_GARDEN` to machineTypes enum
- `src/variants/GlobalsBrogue.c` — Blueprint and auto-generator entries
- `src/brogue/Architect.c` — `applyLichenGardenLayout()` and buildAMachine dispatch
- `src/test/test_fixtures.c` — 4 tests (depth range, custom layout, placement, adjacency pattern)
