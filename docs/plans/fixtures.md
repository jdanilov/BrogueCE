# Plan: Dungeon Fixtures

## Goal

Add 20-25 small decorative "fixtures" — memorable landmark features placed in rooms and caverns using existing tile types and the machine/blueprint system. Fixtures appear roughly 1 per 1-3 levels, use only existing tiles/DFs arranged in themed clusters, and range from tiny (1-3 cells) to small (4-9 cells). Some may have minor gameplay effects using existing DF mechanics.

## Context

BrogueCE already has "thematic machines" (MT_SHRINE_AREA, MT_CAMP_AREA, MT_REMNANT_AREA, etc.) that place decorative tile clusters via the blueprint system. These are 40-120 cells — much larger than what we want. Fixtures are the same mechanism but smaller and more focused: a forge, a fountain, a bone throne.

**Existing infrastructure we reuse:**
- `blueprint` struct with `machineFeature` entries (up to 20 features per blueprint)
- `autoGenerator` catalog for frequency/depth control
- `BP_NO_INTERIOR_FLAG` for area machines that don't mark tiles as "in a machine" (or we can use it to mark them — TBD)
- `MF_NOT_IN_HALLWAY`, `MF_BUILD_AT_ORIGIN`, `MF_NEAR_ORIGIN`, `MF_FAR_FROM_ORIGIN` for placement control
- `MF_TREAT_AS_BLOCKING` to ensure fixtures don't break connectivity
- All existing tile types: STATUE_INERT, LAVA, SHALLOW_WATER, DEEP_WATER, BONES, RUBBLE, ASH, EMBER, HAY, JUNK, CARPET, MARBLE_FLOOR, GRASS, FUNGUS_FOREST, DEAD_GRASS, CRYSTAL_WALL, GRANITE_COLUMN, etc.
- All existing DFs: DF_GRASS, DF_BONES, DF_RUBBLE, DF_ASH, DF_SUNLIGHT, DF_LUMINESCENT_FUNGUS, DF_SHALLOW_WATER_POOL, DF_DEEP_WATER_POOL, DF_REMNANT, DF_STEAM_PUFF, etc.

**Key constraint:** No new tile types. Every fixture is a unique arrangement of existing tiles, DFs, and terrain layers.

## Design Decisions

- **Size**: Mix of tiny (1-3 cells) and small (4-9 cells), specified per fixture
- **Effects**: Minor gameplay effects allowed (steam, light, fire) via existing DFs
- **System**: Machine/blueprint system with auto-generators
- **Depth**: Mix of universal fixtures + depth-exclusive ones
- **Placement**: Context-sensitive per fixture (room-preferring, cavern-preferring, or anywhere)
- **Frequency**: ~1 per 1-3 levels. Auto-generator frequency tuned so most levels get one, some get zero, rarely two

## Fixture Catalog (32 fixtures)

Following fixtures are suggestions. Implementation agent may alter their properties.

### Placement Semantics

Fixtures use thematic placement to feel intentional rather than random:
- **Room center** (`MF_BUILD_AT_ORIGIN` with origin at room center): Fountain, Bone Throne, Altar — things that command a space
- **Room corner/wall-adjacent** (`MF_BUILD_IN_WALLS` or `MF_NEAR_ORIGIN` with wall-adjacent origin): Cobweb Corner, Claw Marks, Weapon Rack — things that cling to edges
- **Room interior** (`MF_NOT_IN_HALLWAY`): Forge, Collapsed Pillar — need floor space but not center
- **Cavern organic** (large `roomSize` footprint, no wall constraint): Mushroom Circle, Crystal Outcrop — natural formations
- **Anywhere** (no constraints): Rubble Heap, Sunlit Patch — can appear in corridors or rooms

### Thematic Loot

Some fixtures can spawn a thematic item via `MF_GENERATE_ITEM`. Loot should be **optional** (not every instance spawns one) and **thematic** (connects the item to the place). This makes fixtures feel like discoverable locations rather than random item drops.

Loot is controlled by the `MF_GENERATE_ITEM` flag on a machineFeature entry with `itemCategory` and optionally `itemKind`. Not all fixtures have loot — most are purely decorative. When loot is present, it should feel like a natural find, not a guaranteed reward.

| Fixture | Loot | Rationale |
|---------|------|-----------|
| **Forge** | WEAPON (random) | Left behind by the smith. |
| **Toppled Bookcase** | SCROLL (random) | Survived among the wreckage. |
| **Abandoned Camp** | FOOD or POTION (random) | Supplies left by a prior explorer. |
| **Altar Nook** | SCROLL or POTION (random) | Offering left at the altar. |
| **Garden Patch** | POTION (random) | Herbal ingredients growing wild. |
| **Weapon Rack** | WEAPON (random) | One still serviceable piece. |
| **Warding Circle** | CHARM or SCROLL (random) | Residual magic condensed into form. |
| **Bone Throne** | RING or GOLD (random) | Pried from dead fingers. |
| **Sacrificial Slab** | POTION (random) | Ritual remnant, still potent. |
| **Crystal Outcrop** | GEM (random) | A loose crystal worth taking. |

**Loot frequency**: Use `MF_ALTERNATIVE` flags so each fixture has e.g. a 30-50% chance of the loot feature firing vs. an empty alternative. This keeps fixtures from being reliable loot sources — you find something maybe half the time.

### Universal (Any Depth)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 1 | **Rubble Heap** ✅ | 3-5 | STATUE_INERT, RUBBLE, BONES | Anywhere | Broken column with scattered debris. BP_PURGE_INTERIOR. |
| 2 | **Lone Statue** ✅ | 1-2 | STATUE_INERT, MARBLE_FLOOR | Room center | Imposing figure on marble pedestal. |
| 3 | **Collapsed Pillar** ✅ | 2-4 | STATUE_INERT (1), RUBBLE (around) | Room interior | Broken column with scattered debris. BP_PURGE_INTERIOR. |
| 4 | **Drainage Channel** ✅ | 3-6 | SHALLOW_WATER in a line, RUBBLE at ends | Anywhere | Water seeping through cracked floor. |
| 5 | **Mossy Alcove** ✅ | 3-5 | GRASS, FOLIAGE, SHALLOW_WATER (1) | Cavern wall-adjacent | Overgrown nook with seeping water. BP_PURGE_INTERIOR. |
| 6 | **Cobweb Corner** | 2-4 | SPIDERWEB (2-3), BONES (1) | Room corner | Old webs with ancient remains. |
| 7 | **Crumbled Wall** | 2-4 | RUBBLE (2-3), GRANITE_COLUMN (1) | Wall-adjacent | Partially collapsed wall section. |
| 8 | **Dust Motes** | 2-3 | DF_ASH (2), DEAD_GRASS (1) | Anywhere | Undisturbed for ages. |

### Early Levels (D1-D8)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 9 | **Garden Patch** ✅ | 12-21 | FOLIAGE, SHALLOW_WATER (custom layout: 3-wide alternating rows) | Cavern organic | Overgrown garden with water channels. Custom `applyGardenLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 10 | **Fountain** ✅ | 3-5 | STATUE_INERT (center), SHALLOW_WATER (ring), MARBLE_FLOOR (border) | Room center | Classic dungeon fountain. Centerpiece. BP_PURGE_INTERIOR clears foliage. |
| 11 | **Mushroom Circle** | 3-6 | FUNGUS_FOREST in ring, GRASS center | Cavern organic | Fairy ring in a damp hollow. |
| 12 | **Sunlit Patch** | 3-5 | DF_SUNLIGHT, GRASS, DF_FOLIAGE | Anywhere | Shaft of light from a crack above. |
| 13 | **Bird Nest** | 1-2 | HAY (cluster), DF_FOLIAGE around | Cavern corner | Organic nest tucked into a crevice. |
| 14 | **Vine Trellis** | 3-5 | DF_FOLIAGE (2-3), GRASS, DEAD_GRASS (1) | Wall-adjacent | Creeping vines along a wall. |
| 15 | **Puddle** | 2-3 | SHALLOW_WATER (1-2), MUD (1) | Anywhere | Stagnant water collecting in a depression. |

### Mid Levels (D5-D18)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 16 | **Forge** | 4-6 | LAVA (1), RUBBLE (surround), STATUE_INERT (1 anvil) | Room interior | Ancient smithy. Minor effect: ember DF. Loot: WEAPON (~40%). |
| 17 | **Altar Nook** | 2-4 | ALTAR_INERT (1), CARPET (2-3 around) | Room center | Devotional space. Loot: SCROLL or POTION (~30%). |
| 18 | **Crystal Outcrop** | 2-4 | CRYSTAL_WALL (1-2), LUMINESCENT_FUNGUS around | Cavern wall-adjacent | Glowing mineral vein. Light effect. Loot: GEM (~30%). |
| 19 | **Steam Vent** | 1-3 | CHASM (1), DF_STEAM_PUFF periodic | Anywhere | Geothermal fissure. Minor effect: occasional steam. |
| 20 | **Abandoned Camp** | 4-7 | HAY (2), JUNK (1-2), BONES (1), EMBERS (1) | Room interior | Someone stayed here once. Loot: FOOD or POTION (~40%). |
| 21 | **Weapon Rack** | 2-3 | STATUE_INERT (1), JUNK (1-2) | Wall-adjacent | Broken equipment on wall. Loot: WEAPON (~30%). |
| 22 | **Scorched Earth** | 3-5 | ASH (2-3), EMBERS (1), DEAD_GRASS (1) | Anywhere | Aftermath of an old fire. |
| 23 | **Lichen Garden** | 3-5 | FUNGUS_FOREST (2), DEAD_GRASS (2), SHALLOW_WATER (1) | Cavern organic | Bioluminescent growth around moisture. |
| 24 | **Toppled Bookcase** | 2-3 | JUNK (2), RUBBLE (1) | Room wall-adjacent | Rotting shelves. Loot: SCROLL (~40%). |

### Deep Levels (D10+)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 25 | **Bone Throne** | 3-5 | BONES (cluster), STATUE_INERT (1 center) | Room center | Grim seat of power. Loot: RING or GOLD (~30%). |
| 26 | **Blood Pool** | 2-4 | DF_RED_BLOOD (spread), SHALLOW_WATER (1 center) | Cavern organic | Something died here, slowly. |
| 27 | **Obsidian Formation** | 2-4 | OBSIDIAN (2-3), ASH around | Anywhere | Cooled lava remnants from an ancient flow. |
| 28 | **Ember Pit** | 3-5 | EMBERS (center), ASH (ring), BONES (1) | Cavern organic | Still smoldering. Minor effect: embers DF. |
| 29 | **Claw Marks** | 1-3 | RUBBLE (wall-adjacent), BONES (1) | Wall-adjacent | Deep gouges — something large passed through. |
| 30 | **Sacrificial Slab** | 2-4 | MARBLE_FLOOR (1), DF_RED_BLOOD (around), BONES (1) | Room center | Dark stains on polished stone. Loot: POTION (~30%). |
| 31 | **Sulfur Crust** | 2-4 | ASH (2), EMBERS (1), DF_STEAM_PUFF | Cavern organic | Yellow deposits around a fumarole. Minor effect: steam. |
| 32 | **Warding Circle** | 3-5 | CARPET (ring), BONES (1 center), ASH (1) | Room center | Ancient inscription, long faded. Loot: CHARM or SCROLL (~30%). |

## Implementation Approach

### Machine Blueprint Design

Each fixture becomes a small `blueprint` entry:
- `roomSize[2]`: footprint in cells, e.g. `{3, 6}` for a 3-6 cell fixture
- `flags`: `BP_NO_INTERIOR_FLAG` (don't mark as machine room) + context flags
- `depthRange[2]`: depth bounds from the catalog above
- `featureCount`: 2-4 machineFeatures per fixture, OR 0 for custom layout fixtures
- `frequency`: 0 (driven by auto-generators, not direct machine placement)

**Two placement approaches:**

**Standard (blueprint features):** Each fixture's `machineFeature` entries place tiles:
- Origin feature: the "centerpiece" tile (statue, lava, deep water)
- Surrounding features: decorative tiles around it (rubble, grass, carpet)
- Optional: surface-layer DFs for flavor (blood, ash, embers)

**Custom layout (featureCount == 0):** For precise geometric patterns (rows, grids, rings). Blueprint claims the room; a dedicated function in `Architect.c` places tiles programmatically using the machine interior grid. See `applyGardenLayout()` as reference. Custom layouts can abort the machine if the interior shape doesn't fit the pattern.

### Machine Placement Tracking

Every successful `buildAMachine()` call records a `placedMachineInfo` entry in `rogue.placedMachines[]` containing `{ blueprintIndex, machineNumber, origin }`. This enables:
- `tools/scan_fixture_seeds.c` to find fixtures by blueprint index (instant lookup, no tile scanning)
- Tests to verify fixture placement via `rogue.placedMachineCount`
- Custom layouts to store an effective origin different from the machine origin

Defined in `Rogue.h`, reset in `digDungeon()`, max `MAX_PLACED_MACHINES` (50) entries per level.

### Auto-Generator Integration

Add one auto-generator entry per fixture in the variant globals:
```c
// Example: Fountain (early levels, rooms)
{FLOOR, DUNGEON, 0, MT_FIXTURE_FOUNTAIN, FLOOR, NOTHING, 1, 8, 6, 0, 0, 1},
```

Alternatively, create a **single meta-generator** that picks a random fixture from the eligible pool for the current depth. This keeps the auto-generator table small and controls overall frequency centrally:
- Frequency ~15-20 (comparable to existing thematic machines)
- The meta-generator selects from depth-eligible fixtures via a weighted raffle

**Recommended: single meta-generator approach.** This makes frequency tuning trivial — one knob for "how often do fixtures appear" rather than 22 separate frequency values that interact unpredictably.

### Placement Context

Use existing machine feature flags for context sensitivity:
- **Rooms only**: Use `BP_ROOM` flag or `MF_NOT_IN_HALLWAY`
- **Caverns preferred**: Place with larger `roomSize` range to prefer open areas
- **Anywhere**: `BP_NO_INTERIOR_FLAG` with no additional constraints
- **Wall-adjacent** (claw marks, cobwebs): `MF_BUILD_IN_WALLS` for wall-touching features

### Connectivity Safety

All multi-cell fixtures use `MF_TREAT_AS_BLOCKING` on obstructing tiles (statues, columns, crystal walls) so the pathfinding system rejects placements that would block the level.

## Acceptance Criteria

- [ ] 20+ fixture blueprints defined in variant globals
- [ ] Fixtures appear ~1 per 1-3 levels during normal play
- [ ] No new tile types — only existing tiles, DFs, and terrain layers
- [ ] Fixtures never block level connectivity
- [ ] Fixtures don't overlap with existing machines
- [ ] Depth-restricted fixtures only appear in their designated range
- [ ] Context-sensitive placement works (room/cavern/anywhere)
- [ ] Minor gameplay effects (steam, light, embers) work via existing DFs
- [ ] Each fixture has appropriate flavor text visible on mouse-over
- [ ] All existing tests pass (`make test`, seed catalogs updated)
- [ ] Determinism maintained — same seed produces same fixtures

## Tasks

- [ ] **Task 1: Define fixture machine types** — Add `MT_FIXTURE_*` enum values (32 entries) to `Rogue.h` machine types enum
- [ ] **Task 2: Create fixture blueprints** — Add 32 blueprint entries to `GlobalsBrogue.c` (and variant globals if needed), each with 2-4 machineFeatures using existing tiles/DFs. Use placement flags per the Placement Semantics section
- [ ] **Task 3: Add auto-generator entries** — Either one meta-generator or per-fixture generators in the auto-generator catalog. Tune frequency for ~1 per 1-3 levels
- [ ] **Task 4: Test placement** — Verify fixtures spawn correctly across depth ranges, don't break connectivity, don't overlap machines. Visual inspection via gameplay
- [ ] **Task 5: Update seed catalogs** — Run `test/update_seed_catalogs.py` since dungeon generation changes
- [ ] **Task 6: Write change doc** — Document the fixture system in `docs/changes/`

## Ask User
(empty)

## Critic Findings
(empty)
