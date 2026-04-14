# Add Dungeon Fixture

Add one fixture to the dungeon fixture system, end-to-end: design → implement → test → find seed → print ASCII.

**Target fixture:** $ARGUMENTS
If no argument given, find the first fixture without ✅ in `docs/plans/fixtures.md`.

---

## Step 1 — Identify the fixture

Read `docs/plans/fixtures.md`. If `$ARGUMENTS` is empty, scan the catalog tables for the first entry that does **not** have ✅ in its name. Extract:
- Name, size range, tiles/DFs listed, placement type, depth range, notes (loot, effects)

---

## Step 2 — Design: think, then ask

Think through the visual design before asking the user. Consider:

**ASCII character reference (terminal mode, lower priority number = displayed on top):**

| Tile(s) | Char | Priority | Notes |
|---------|------|----------|-------|
| STATUE_INERT | `ß` | 0 | Always wins; T_OBSTRUCTS_SURFACE_EFFECTS — foliage physically cannot grow on it |
| STEAM_VENT tile | `=` | 15 | Always visible |
| SPIDERWEB | `:` | 19 | Always visible — even over deep water |
| DEEP_WATER / LAVA | `~` | 40 | Always visible over foliage (lava has red bg) |
| CHASM | `⁘` | 40 | U_FOUR_DOTS glyph; always visible |
| FOLIAGE / FUNGUS_FOREST | `^` | 45 | **Visibility threshold** — anything priority > 45 may be hidden on D1-D8 |
| MUD | `~` | 55 | G_BOG — **same char as water**, brown background; hidden by foliage without purge |
| SHALLOW_WATER | `.`+tint | 55 | Char 0 (inherits floor dot), blue background only; hidden by foliage without purge |
| OBSIDIAN | `.` | 50 | G_FLOOR — **same char as floor**, dark bg; hidden by foliage without purge |
| GRASS / DEAD_GRASS / LUMINESCENT_FUNGUS / HAY | `"` | 60 | G_GRASS; hidden by foliage without purge |
| EMBERS | `'` | 70 | G_ASHES; hidden by foliage without purge |
| JUNK | `,` | 70 | G_BONES — same char as bones; hidden by foliage without purge |
| RED_BLOOD | `.` | 80 | G_FLOOR_ALT — **same char as floor**; always hidden by foliage |
| ASH | `'` | 80 | G_ASHES — same char as embers; always hidden by foliage |
| CARPET / MARBLE_FLOOR | `.` | 85 | G_CARPET — same char as floor, different color; always hidden by foliage |
| RUBBLE / BONES | `,` | surface | Both G_RUBBLE/G_BONES → `,`; hidden by foliage without purge |
| WALL / GRANITE / CRYSTAL_WALL | `#` | 0–varies | Columns look identical to walls in terminal |
| ALTAR_INERT | `\|` | dungeon | G_ALTAR |

**Rule of thumb:** use `BP_PURGE_INTERIOR` for any fixture where the interesting tiles have priority > 45. Purge runs before features are placed, wiping pre-existing foliage. It sets DUNGEON→FLOOR, LIQUID→NOTHING, SURFACE→NOTHING — safe since machine footprint cells are already floor tiles. The only tiles that never need purge are STATUE_INERT, STEAM_VENT, SPIDERWEB, DEEP_WATER, LAVA, CHASM, and FOLIAGE/FUNGUS_FOREST itself.

**Prepare 2–3 ASCII layout options.** Each should be ~9×7 cells showing wall context. Use the chars above. Note which tiles correspond to which symbol if ambiguous. Note any BP_PURGE_INTERIOR recommendation. Make sure to use appropriate console colors (blue for water, orange for lava etc.) when presenting your ASCII designs.

**Then call AskUserQuestion** with:
- The 2–3 layouts side by side or sequentially
- A brief note on what each communicates thematically
- Any design trade-offs (e.g. "option A is subtle, option B has a clear anchor tile")
- Ask: which layout to use, or any modifications?

Wait for the user's answer before proceeding.

---

## Step 3 — Implement

### 3a. Current state (read before editing)

Current last fixture in the enum: `MT_FIXTURE_PUDDLE = 87`
Blueprint catalog currently has **88 entries** (indices 0–87).
New fixture will be: enum value **88**, catalog index **88**.

Key files:
- `src/brogue/Rogue.h` — machineTypes enum
- `src/variants/GlobalsBrogue.c` — blueprintCatalog_Brogue[] and autoGeneratorCatalog_Brogue[]
- `src/test/test_fixtures.c` — test suite
- `src/test/test_main.c` — suite registration (only if adding a new suite; fixtures suite already exists)

### 3b. Tile placement: blueprint features vs custom layout

**IMPORTANT: blueprint features scatter tiles semi-randomly.** `MF_NEAR_ORIGIN` only means "in the closest quarter of the interior" — it does NOT guarantee tiles end up adjacent to each other or to the origin tile. Multi-tile fixtures using blueprint features often look disjointed: water in one corner, foliage in another, with empty floor between them. This is fine for fixtures where scatter IS the aesthetic (rubble, bones), but bad for fixtures that need spatial coherence.

**When to use blueprint features:**
- The fixture has a single anchor tile (statue, altar) that stands alone
- Surrounding tiles are thematically "scattered" (rubble, bones, ash, dead grass)
- It's OK if tiles land 2-4 cells apart with gaps between them
- Examples: Rubble Heap (statue + scattered rubble), Lone Statue (statue + nearby marble)

**When to use custom layout:**
- Tiles must be adjacent to each other (water surrounded by foliage, ring patterns)
- The fixture tells a spatial story (gradient from dry→wet, concentric rings)
- Geometric precision matters (lines, rows, grids, L-shapes)
- Multiple tile types need to relate to each other spatially
- Examples: Garden Patch (alternating rows), Drainage Channel (water line with rubble ends), Mossy Alcove (water with adjacent vegetation)

**Default to custom layout** unless the fixture is clearly a "central anchor + random scatter" pattern. The visual coherence of custom layouts far outweighs the implementation cost.

**Custom layout approach** — Set `featureCount = 0` in the blueprint and add a custom layout function in `Architect.c` (see `applyMossyAlcoveLayout` for a small fixture, `applyGardenLayout` for a larger one).

Custom layouts:
- Are called from `buildAMachine()` after the interior is established, keyed by machine type enum
- Receive `originX`, `originY`, and the `interior[DCOLS][DROWS]` grid
- Return `boolean` — false aborts the machine (restores level backup)
- Can return an effective origin via `pos *outCenter` for accurate scanner display
- Must only place tiles on cells where `interior[x][y]` is true
- Blueprint still needs appropriate `roomSize` to ensure the interior is large enough

**Verified tileType enum names and their correct layers:**

| Tile | Enum name | Layer | Notes |
|------|-----------|-------|-------|
| Statue/pillar | `STATUE_INERT` | `DUNGEON` | Blocks pathing, use MF_TREAT_AS_BLOCKING |
| Crystal wall | `CRYSTAL_WALL` | `DUNGEON` | Blocks pathing |
| Altar | `ALTAR_INERT` | `DUNGEON` | |
| Marble floor | `MARBLE_FLOOR` | `DUNGEON` | |
| Carpet | `CARPET` | `DUNGEON` | |
| Obsidian | `OBSIDIAN` | `DUNGEON` | |
| Shallow water | `SHALLOW_WATER` | `LIQUID` | |
| Deep water | `DEEP_WATER` | `LIQUID` | |
| Lava | `LAVA` | `LIQUID` | |
| Chasm | `CHASM` | `LIQUID` | |
| Mud | `MUD` | `LIQUID` | |
| Grass | `GRASS` | `SURFACE` | |
| Dead grass | `DEAD_GRASS` | `SURFACE` | |
| Foliage | `FOLIAGE` | `SURFACE` | |
| Fungus forest | `FUNGUS_FOREST` | `SURFACE` | |
| Luminescent fungus | `LUMINESCENT_FUNGUS` | `SURFACE` | |
| Hay | `HAY` | `SURFACE` | |
| Spiderweb | `SPIDERWEB` | `SURFACE` | |
| Rubble | `RUBBLE` | `SURFACE` | |
| Bones | `BONES` | `SURFACE` | |
| Embers | `EMBERS` | `SURFACE` | |
| Ash | `ASH` | `SURFACE` | |
| Junk | `JUNK` | `SURFACE` | |
| Blood | `RED_BLOOD` | `SURFACE` | |

**There is NO `GRANITE_COLUMN` tileType.** `DF_GRANITE_COLUMN` exists but it's a DF that places `GRANITE` tiles with propagation. To represent a column/pillar, use `STATUE_INERT` instead — it's visually distinct (`ß`/`S`) and detectable by the scanner.

**`GRANITE` is the wall tile** — do not use it for interior features; it's indistinguishable from walls in display.

### 3c. Rogue.h — add to enum

Add before the closing `};` of the machineTypes enum, after the last `MT_FIXTURE_*` entry:
```c
    MT_FIXTURE_NAME
```
(Add a comma to the previous last entry.)

### 3d. GlobalsBrogue.c — blueprint

Append after the last fixture blueprint entry (before `};` of blueprintCatalog_Brogue[]):

```c
{"Fixture: NAME -- description",
{minDepth, maxDepth},  {roomSizeMin, roomSizeMax},  0,  N,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    //         DF          terrain        layer          count      minInst  iCat iKind mKind  space  hFlg iFlg  featureFlags
    {           0,          TILE,          LAYER,         {mn, mx},  1,       0,   -1,   0,     0,     0,   0,   (FLAGS)},
    ...
}},
```

**roomSize guidance:** `{3,6}` tiny · `{4,8}` small · `{6,12}` medium · `{12,21}` for custom layouts needing space

**Frequency guidance for auto-generator:**
- All fixtures use frequency **17** (uniform). Always maxNumber 1.
- Target distribution: ~75% of levels get ≥1 fixture, ~25% get ≥2.

**How frequency works:** Each auto-generator independently rolls `rand_percent(frequency)`.
If it succeeds AND `buildAMachine` succeeds, one fixture is placed. With N eligible fixtures
at frequency f, the theoretical P(0 attempts) = (1 - f/100)^N. However, `buildAMachine`
has a ~50% failure rate (room too small, connectivity blocked, etc.), so effective per-fixture
placement is roughly half the frequency value.

**Current tuning (12 fixtures on D1-8 at freq 17):**
- Theoretical attempts per level: 12 × 0.17 = 2.0
- After ~50% buildAMachine failure: ~1.0 placed
- Measured: 25% zero, 40% one, 24% two, 11% three+

**When adding more fixtures, recheck the distribution:**
```bash
# Build the counter (one-time):
cat > /tmp/count_fixtures.c << 'CEOF'
#include "test_harness.h"
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    int histogram[20] = {0};
    int total_seeds = 200;
    for (int seed = 1; seed <= total_seeds; seed++) {
        test_init_game(seed);
        int fixture_count = 0;
        for (int i = 0; i < rogue.placedMachineCount; i++) {
            int bp = rogue.placedMachines[i].blueprintIndex;
            if (bp >= MT_FIXTURE_FOUNTAIN && bp <= MT_FIXTURE_VINE_TRELLIS) {
                fixture_count++;
            }
        }
        if (fixture_count < 20) histogram[fixture_count]++;
        test_teardown_game();
    }
    printf("Fixture count distribution on D1 (%d seeds):\n", total_seeds);
    for (int i = 0; i < 10; i++) {
        if (histogram[i] > 0)
            printf("  %d fixtures: %d seeds (%.0f%%)\n", i, histogram[i], 100.0*histogram[i]/total_seeds);
    }
    return 0;
}
CEOF

GAME_SRCS=$(find src/brogue src/variants -name '*.c') && \
cc -DDATADIR=. -DBROGUE_SDL -DBROGUE_EXTRA_VERSION='""' \
   -Isrc/brogue -Isrc/platform -Isrc/variants -Isrc/test \
   -std=c99 -O2 /tmp/count_fixtures.c $GAME_SRCS \
   src/platform/null-platform.c src/platform/platformdependent.c \
   src/test/test_harness.c -o /tmp/count_fixtures && /tmp/count_fixtures
```

**If distribution is too high**, lower all fixture frequencies uniformly.
**If distribution is too low on deep levels**, that's expected — more deep fixtures will fill it in.
Update the `MT_FIXTURE_VINE_TRELLIS` reference in the counter script to the latest fixture enum.

**Feature flags quick reference:**
- `MF_BUILD_AT_ORIGIN` — place at machine origin
- `MF_NEAR_ORIGIN` — place in closest quarter to origin
- `MF_NOT_IN_HALLWAY` — avoid corridors
- `MF_TREAT_AS_BLOCKING` — abort placement if tile would disconnect level (use on any pathing blocker)
- `MF_BUILD_IN_WALLS` — place in impassable tile adjacent to interior (wall-adjacent fixtures)
- `MF_GENERATE_ITEM` + itemCategory — loot drop

**Layer constants:** `DUNGEON=0  LIQUID=1  GAS=2  SURFACE=3`

### 3e. GlobalsBrogue.c — auto-generator

In the `// Fixture machines` comment block at the bottom of autoGeneratorCatalog_Brogue[]:
```c
{0, 0, 0, MT_FIXTURE_NAME, FLOOR, NOTHING, minDepth, maxDepth, frequency, 0, 0, 1},
```

### 3f. test_fixtures.c — add tests

Follow the established pattern in the file. Tests per fixture:

**Standard fixtures (blueprint features):** 4 tests minimum:
1. Blueprint depth range matches plan
2. Blueprint featureCount > 0
3. `buildAMachine(MT_FIXTURE_NAME, ...)` succeeds and a key tile appears in pmap
4. `blueprintCatalog[MT_FIXTURE_NAME].feature[0]` has the expected terrain/layer

**Custom layout fixtures (featureCount == 0):** 3-4 tests:
1. Blueprint depth range matches plan
2. featureCount == 0 (confirming custom layout is used)
3. `buildAMachine` succeeds and is recorded in `rogue.placedMachines`
4. The layout pattern is present on the map (e.g. alternating row pattern)

**Test gotchas:**
- `DEEPEST_LEVEL` macro is NOT available in tests. Use `gameConst->deepestLevel` instead.
- `BP_NO_INTERIOR_FLAG` means `machineNumber` won't be set on fixture tiles — don't check it.
- Standard fixtures: use `test_init_arena()` for placement tests (reliable open space).
- Custom layout fixtures: use `test_init_game()` for placement tests (need real room geometry for the interior blob to have the right shape).
- Placement tests use a retry loop (up to 30 attempts).

Register all tests in the `SUITE(fixtures)` block at the bottom of the file.

---

## Step 4 — Build and test

```
make test
```

All tests must pass. Fix any failures before continuing.

---

## Step 5 — Find a seed and print ASCII

`tools/scan_fixture_seeds.c` is a **generic scanner** — no editing required. It uses `rogue.placedMachines[]` metadata recorded by `buildAMachine()` to find fixtures by blueprint index.

**How it works:**
- Each successful `buildAMachine()` call records `{ blueprintIndex, machineNumber, origin }` in `rogue.placedMachines[]`
- The scanner searches this array for the target machine type — instant lookup, no heuristic tile scanning
- For standard fixtures (featureCount > 0): the ASCII display auto-sizes a bounding box around the blueprint's feature terrain tiles near the origin
- For custom layout fixtures (featureCount == 0): the display shows a fixed 7×11 window centered on the effective origin

**Build once** (only needed if game source changed since last build):
```bash
GAME_SRCS=$(find src/brogue src/variants -name '*.c') && \
cc -DDATADIR=. -DBROGUE_SDL -DBROGUE_EXTRA_VERSION='""' \
   -Isrc/brogue -Isrc/platform -Isrc/variants -Isrc/test \
   -std=c99 -O2 \
   tools/scan_fixture_seeds.c $GAME_SRCS \
   src/platform/null-platform.c src/platform/platformdependent.c \
   src/test/test_harness.c \
   -o bin/scan-fixture 2>&1 | grep error
```

**Scan for seeds** (replace INDEX with the new fixture's enum index):
```bash
./bin/scan-fixture INDEX 500
# Output: lists matching seeds, then prints the command to display the first hit
```

**Display a specific seed:**
```bash
./bin/scan-fixture INDEX -seed N
# Prints ASCII map centered on the fixture's origin, with machine number
```

**If no seeds found in 500:** Try 1000. For custom layout fixtures, the layout may reject rooms that are too small — this is expected. Increase roomSize in the blueprint if placement rate is too low.

---

## Step 6 — Report

Print:
1. The ASCII map of the fixture as it appears on the level
2. The legend
3. The seed and depth (`./bin/brogue --seed N` to verify in-game)
4. Mark the fixture as ✅ in `docs/plans/fixtures.md`
5. Update this file's "Current state" section (Step 3a) with the new last enum entry and catalog size.

---

## Reference: existing fixtures as templates

```c
// MT_FIXTURE_FOUNTAIN = index 73
{"Fixture: Fountain -- marble statue in a pool, foliage purged for visibility",
{1, 8},  {4, 8},  0,  3,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    {0, STATUE_INERT,  DUNGEON, {1,1}, 1, 0,-1,0, 0, 0,0, (MF_BUILD_AT_ORIGIN|MF_NOT_IN_HALLWAY|MF_TREAT_AS_BLOCKING)},
    {0, SHALLOW_WATER, LIQUID,  {2,4}, 1, 0,-1,0, 1, 0,0, (MF_NEAR_ORIGIN|MF_NOT_IN_HALLWAY)},
    {0, MARBLE_FLOOR,  DUNGEON, {1,3}, 0, 0,-1,0, 1, 0,0, (MF_NEAR_ORIGIN|MF_NOT_IN_HALLWAY)}}},
```
Auto-generator: `{0, 0, 0, MT_FIXTURE_FOUNTAIN, FLOOR, NOTHING, 1, 8, 40, 0, 0, 1}`

```c
// MT_FIXTURE_RUBBLE_HEAP = index 74
{"Fixture: Rubble Heap -- broken column amid scattered debris",
{1, DEEPEST_LEVEL}, {4, 8},  0,  3,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    {0, STATUE_INERT,  DUNGEON, {1,1}, 1, 0,-1,0, 0, 0,0, (MF_BUILD_AT_ORIGIN|MF_NOT_IN_HALLWAY|MF_TREAT_AS_BLOCKING)},
    {0, RUBBLE,        SURFACE, {2,4}, 0, 0,-1,0, 1, 0,0, (MF_NEAR_ORIGIN)},
    {0, BONES,         SURFACE, {1,1}, 0, 0,-1,0, 1, 0,0, (MF_NEAR_ORIGIN)}}},
```
Auto-generator: `{0, 0, 0, MT_FIXTURE_RUBBLE_HEAP, FLOOR, NOTHING, 1, DEEPEST_LEVEL, 35, 0, 0, 1}`

```c
// MT_FIXTURE_GARDEN_PATCH = index 76 — CUSTOM LAYOUT (featureCount == 0)
// Tile placement handled by applyGardenLayout() in Architect.c.
// Places 3-wide alternating rows of FOLIAGE and SHALLOW_WATER.
// Aborts if the interior doesn't have a 3-wide column of 4+ contiguous rows.
{"Fixture: Garden Patch -- overgrown garden with water channels",
{1, 8},  {12, 21},  0,  0,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    // Tile placement handled by applyGardenLayout() in Architect.c
}},
```
Auto-generator: `{0, 0, 0, MT_FIXTURE_GARDEN_PATCH, FLOOR, NOTHING, 1, 8, 40, 0, 0, 1}`

```c
// MT_FIXTURE_MOSSY_ALCOVE = index 79 — CUSTOM LAYOUT (featureCount == 0)
// Tile placement handled by applyMossyAlcoveLayout() in Architect.c.
// Places shallow water at center with foliage and grass on adjacent cells.
// Small fixture (3-5 tiles) — good template for compact custom layouts.
{"Fixture: Mossy Alcove -- overgrown nook with seeping water",
{1, DEEPEST_LEVEL},  {4, 8},  0,  0,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    // Tile placement handled by applyMossyAlcoveLayout() in Architect.c
}},
```
Auto-generator: `{0, 0, 0, MT_FIXTURE_MOSSY_ALCOVE, FLOOR, NOTHING, 1, DEEPEST_LEVEL, 35, 0, 0, 1}`
