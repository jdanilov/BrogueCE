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
| WALL / GRANITE / GRANITE_COLUMN / CRYSTAL_WALL | `#` | 0–varies | Columns look identical to walls in terminal |
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

Current last fixture in the enum: `MT_FIXTURE_FOUNTAIN = 73`
Blueprint catalog currently has **74 entries** (indices 0–73).
New fixture will be: enum value **74**, catalog index **74**.

Key files:
- `src/brogue/Rogue.h` — machineTypes enum
- `src/variants/GlobalsBrogue.c` — blueprintCatalog_Brogue[] and autoGeneratorCatalog_Brogue[]
- `src/test/test_fixtures.c` — test suite
- `src/test/test_main.c` — suite registration (only if adding a new suite; fixtures suite already exists)

### 3b. Rogue.h — add to enum

Find the block:
```c
    // Fixture machines
    MT_FIXTURE_FOUNTAIN
};
```
Change to:
```c
    // Fixture machines
    MT_FIXTURE_FOUNTAIN,
    MT_FIXTURE_NAME
};
```

### 3c. GlobalsBrogue.c — blueprint

Append after the fountain blueprint entry (before `};`):

```c
{"Fixture: NAME -- description",
{minDepth, maxDepth},  {roomSizeMin, roomSizeMax},  0,  N,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    //         DF          terrain        layer          count      minInst  iCat iKind mKind  space  hFlg iFlg  featureFlags
    {           0,          TILE,          LAYER,         {mn, mx},  1,       0,   -1,   0,     0,     0,   0,   (FLAGS)},
    ...
}},
```

**roomSize guidance:** `{3,6}` tiny · `{4,8}` small · `{6,12}` medium

**Depth → frequency guidance for auto-generator:**
- Universal (all depths): frequency 30–40
- Early only (D1–8): frequency 35–45
- Mid (D5–18): frequency 25–35
- Deep (D10+): frequency 20–30
- Always maxNumber 1

**Feature flags quick reference:**
- `MF_BUILD_AT_ORIGIN` — place at machine origin
- `MF_NEAR_ORIGIN` — place in closest quarter to origin
- `MF_NOT_IN_HALLWAY` — avoid corridors
- `MF_TREAT_AS_BLOCKING` — abort placement if tile would disconnect level (use on any pathing blocker)
- `MF_BUILD_IN_WALLS` — place in impassable tile adjacent to interior (wall-adjacent fixtures)
- `MF_GENERATE_ITEM` + itemCategory — loot drop

**Layer constants:** `DUNGEON=0  LIQUID=1  GAS=2  SURFACE=3`

### 3d. GlobalsBrogue.c — auto-generator

In the `// Fixture machines` comment block at the bottom of autoGeneratorCatalog_Brogue[]:
```c
{0, 0, 0, MT_FIXTURE_NAME, FLOOR, NOTHING, minDepth, maxDepth, frequency, 0, 0, 1},
```

### 3e. test_fixtures.c — add 4 tests

Follow the established pattern in the file. Four tests minimum:
1. Blueprint depth range matches plan
2. Blueprint featureCount > 0
3. `buildAMachine(MT_FIXTURE_NAME, ...)` succeeds and a key tile appears in pmap
4. `blueprintCatalog[MT_FIXTURE_NAME].feature[0]` has the expected terrain/layer

Register all four in the `SUITE(fixtures)` block at the bottom of the file.

---

## Step 4 — Build and test

```
make test
```

All 194+ tests must pass. Fix any failures before continuing.

---

## Step 5 — Find a seed and print ASCII

`tools/scan_fixture_seeds.c` is a **generic scanner** — no editing required. It reads the blueprint at runtime and detects any fixture by its feature tiles.

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

**Scan for seeds** (replace 74 with the new fixture's enum index):
```bash
./bin/scan-fixture 74 200
# Output: lists matching seeds, then prints the command to display the first hit
```

**Display a specific seed:**
```bash
./bin/scan-fixture 74 -seed 6
# Prints 13x9 ASCII map centered on the fixture's first terrain tile
```

Detection uses `blueprintCatalog[mt].feature[0].terrain` as the anchor tile and `feature[1].terrain` as a nearby confirmation — no hardcoding needed.

---

## Step 6 — Report

Print:
1. The ASCII map of the fixture as it appears on the level
2. The legend
3. The seed and depth (`./bin/brogue --seed N` to verify in-game)
4. Mark the fixture as ✅ in `docs/plans/fixtures.md`

---

## Reference: existing fixture as template

```c
// MT_FIXTURE_FOUNTAIN = index 73
{"Fixture: Fountain -- marble statue in a pool, foliage purged for visibility",
{1, 8},  {4, 8},  0,  3,  0,  (BP_NO_INTERIOR_FLAG | BP_PURGE_INTERIOR), {
    {0, STATUE_INERT,  DUNGEON, {1,1}, 1, 0,-1,0, 0, 0,0, (MF_BUILD_AT_ORIGIN|MF_NOT_IN_HALLWAY|MF_TREAT_AS_BLOCKING)},
    {0, SHALLOW_WATER, LIQUID,  {2,4}, 1, 0,-1,0, 1, 0,0, (MF_NEAR_ORIGIN|MF_NOT_IN_HALLWAY)},
    {0, MARBLE_FLOOR,  DUNGEON, {1,3}, 0, 0,-1,0, 1, 0,0, (MF_NEAR_ORIGIN|MF_NOT_IN_HALLWAY)}}},
```
Auto-generator: `{0, 0, 0, MT_FIXTURE_FOUNTAIN, FLOOR, NOTHING, 1, 8, 40, 0, 0, 1}`
