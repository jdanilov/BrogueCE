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
| 6 | **Cobweb Corner** ✅ | 2-4 | SPIDERWEB (2-3), BONES (1) | Room corner | Old webs with ancient remains. Custom `applyCobwebCornerLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 7 | **Crumbled Wall** ✅ | 2-4 | RUBBLE (2-3), STATUE_INERT (1) | Wall-adjacent | Partially collapsed wall section. Custom `applyCrumbledWallLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 8 | **Dust Motes** ✅ | 7-11 | ASH (4-6), DEAD_GRASS (3-5) | Anywhere | Undisturbed for ages. Scattered ash and dried grass. BP_PURGE_INTERIOR. |

### Early Levels (D1-D8)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 9 | **Garden Patch** ✅ | 12-21 | FOLIAGE, SHALLOW_WATER (custom layout: 3-wide alternating rows) | Cavern organic | Overgrown garden with water channels. Custom `applyGardenLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 10 | **Fountain** ✅ | 3-5 | STATUE_INERT (center), SHALLOW_WATER (ring), MARBLE_FLOOR (border) | Room center | Classic dungeon fountain. Centerpiece. BP_PURGE_INTERIOR clears foliage. |
| 11 | **Mushroom Circle** ✅ | 12-21 | FUNGUS_FOREST + LUMINESCENT_FUNGUS ring, GRASS interior | Cavern organic | Glowing fairy ring in a damp hollow. Custom `applyMushroomCircleLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 12 | **Sunlit Patch** ✅ | 3-5 | DF_SUNLIGHT, GRASS, FOLIAGE | Anywhere | Shaft of light from a crack above. DF_SUNLIGHT + FOLIAGE combined in one feature; GRASS scattered nearby. |
| 13 | **Bird Nest** ✅ | 1-2 | HAY (cluster), FOLIAGE, BONES around | Cavern corner | Organic nest tucked into a crevice with cracked egg shells. |
| 14 | **Vine Trellis** ✅ | 2-8 | FOLIAGE (2-8 line) | Wall-adjacent | Creeping vines hugging a wall. Custom `applyVineTrellisLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 15 | **Puddle** ✅ | 2-3 | MUD (1), SHALLOW_WATER (1-3), GRASS, FOLIAGE ring | Anywhere | Stagnant water in a muddy depression with overgrowth. Custom `applyPuddleLayout()` in Architect.c. BP_PURGE_INTERIOR. |

### Mid Levels (D5-D18)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 16 | **Forge** ✅ | 15-25 | LAVA (1), OBSIDIAN (1), STATUE_INERT (1 anvil), EMBERS (10), RUBBLE (2) | Room interior | Ancient smithy with lava trough and obsidian platform. Custom `applyForgeLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 17 | **Altar Nook** ✅ | 10-13 | ALTAR_INERT (1), MARBLE_FLOOR (4), CARPET (4), EMBERS (2) | Room center | Processional hall: carpet runner to altar on marble dais, flanked by embers. Custom `applyAltarNookLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 18 | **Crystal Outcrop** ✅ | 2-4 | CRYSTAL_WALL (1-2), LUMINESCENT_FUNGUS around | Cavern wall-adjacent | Glowing mineral vein. Light effect. Loot: GEM (~30%). |
| 19 | **Steam Vent** ✅ | 1-3 | STEAM_VENT (1), EMBERS (1-2), RUBBLE (1-2) | Anywhere | Geothermal fissure venting scalding steam. Uses existing STEAM_VENT tile with periodic DF_STEAM_PUFF. BP_PURGE_INTERIOR. |
| 20 | **Abandoned Camp** ✅ | 9-10 | STATUE_INERT (1 post), HAY (2 bedroll), EMBERS (1 fire), RUBBLE (3 fire ring), JUNK (1), BONES (1) | Room interior | Fire ring camp with bedroll and marker post. Custom `applyAbandonedCampLayout()` in Architect.c. BP_PURGE_INTERIOR. Loot: FOOD or POTION (~40%). |
| 21 | **Weapon Rack** ✅ | 2-3 | STATUE_INERT (1), JUNK (1-2) | Wall-adjacent | Broken equipment on wall. Custom `applyWeaponRackLayout()` in Architect.c. BP_PURGE_INTERIOR. Loot: WEAPON (~30%). |
| 22 | **Scorched Earth** ✅ | 7-11 | ASH (4-6), EMBERS (1-2), DEAD_GRASS (2-3) | Anywhere | Aftermath of an old fire. BP_PURGE_INTERIOR. |
| 23 | **Lichen Garden** ✅ | 20-30 | SHALLOW_WATER (pools), LUMINESCENT_FUNGUS (bridges), FUNGUS_FOREST (ring), DEAD_GRASS (fringe) | Cavern organic | Grand bioluminescent fungal archipelago. Custom `applyLichenGardenLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 24 | **Toppled Bookcase** ✅ | 2 | STATUE_INERT (1 bookcase), JUNK (1 books) | Wall nook (3-wall alcove) | Bookcase in wall nook with scattered books. Custom `applyToppledBookcaseLayout()` in Architect.c. BP_PURGE_INTERIOR. Loot: SCROLL (~40%). |

### Deep Levels (D10+)

| # | Name | Size | Tiles/DFs Used | Placement | Notes |
|---|------|------|---------------|-----------|-------|
| 25 | **Bone Throne** ✅ | 15-20 | STATUE_INERT (1 throne), MARBLE_FLOOR (4), CARPET (3 runner), BONES (3-5 random), RED_BLOOD (3-5 random) | Room center | Grand throne on marble dais with carpet runner, bones and blood randomly scattered. Custom `applyBoneThroneLayout()` in Architect.c. BP_PURGE_INTERIOR. Loot: RING or GOLD (~30%). |
| 26 | **Blood Pool** ✅ | 20-25 | ALTAR_INERT (1 center), RED_BLOOD (large irregular spread), BONES (3-6 scattered) | Cavern organic | Grand altar rising from a lake of ancient blood. Custom `applyBloodPoolLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 27 | **Obsidian Formation** ✅ | 20-25 | OBSIDIAN (core), EMBERS (ring), ASH (perimeter) | Anywhere | Cooled lava remnants with concentric thermal rings. Custom `applyObsidianFormationLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 28 | **Ember Pit** ✅ | 15-20 | STATUE_INERT (1 stake), EMBERS (4-6), ASH (4-6), DEAD_GRASS (6-8), BONES (2-3) | Cavern organic | Smoldering pyre with burnt stake. Custom `applyEmberPitLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 29 | **Claw Marks** ✅ | 20-50 | RUBBLE, RED_BLOOD, ASH (drag trail), BONES (lair) | Anywhere | Drag trail stretching 20-50 tiles across rooms, ending at bone pile with depth-appropriate monster and item. Custom `applyClawMarksLayout()` in Architect.c. BP_PURGE_INTERIOR. |
| 30 | **Sacrificial Slab** ✅ | 20-25 | MARBLE_FLOOR (5 cross), RED_BLOOD (ring), LUMINESCENT_FUNGUS (perimeter), BONES (2-4), EMBERS (2-3) | Room center | Grand ritual stone with marble cross, blood ring, eerie fungal glow, scattered bones and ritual candles. Custom `applySacrificialSlabLayout()` in Architect.c. BP_PURGE_INTERIOR. Loot: POTION (~30%). |
| 31 | **Sulfur Crust** ✅ | 5 | STEAM_VENT (1 center), ASH (4 cardinal ring), EMBERS (1 diagonal) | Cavern organic | Yellow deposits around a geothermal fumarole. Ring pattern: vent center, ash cardinals, embers diagonal. Custom `applySulfurCrustLayout()` in Architect.c. BP_PURGE_INTERIOR. |

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

Every successful `buildAMachine()` call records a `placedMachineInfo` entry in `rogue.placedMachines[]` containing `{ blueprintIndex, machineNumber, origin, prominentTile }`. This enables:
- `tools/scan_fixture_seeds.c` to find fixtures by blueprint index (instant lookup, no tile scanning)
- Tests to verify fixture placement via `rogue.placedMachineCount`
- Custom layouts to store an effective origin different from the machine origin
- Fixture flavor text display on the prominent tile when the player inspects it

Defined in `Rogue.h`, reset in `digDungeon()`, max `MAX_PLACED_MACHINES` (50) entries per level.

### Fixture Flavor Text

Each fixture has a one-line flavor string displayed when the player inspects (hovers over) the fixture's **prominent tile** — the most visually distinctive tile in the fixture (statue, altar, vent, etc.).

**How it works:**
- `fixtureFlavorText[]` in `Globals.c` maps `(blueprintIndex - MT_FIXTURE_FOUNTAIN)` → flavor string
- `placedMachineInfo.prominentTile` records which cell carries the flavor (defaults to `effectiveOrigin`)
- `fixtureFlavorAt(x, y)` in `Movement.c` scans `rogue.placedMachines[]` for a matching prominent tile
- Overrides both `tileFlavor()` (player standing on tile) and the "you see" path in `describeLocation()` (hovering over empty tile)

**Prominent tile selection — must point at the centerpiece tile, not decoration:**
- **Standard blueprints**: The origin feature (with `MF_BUILD_AT_ORIGIN`) is automatically the prominent tile. Works correctly for STATUE_INERT, STEAM_VENT, FOLIAGE, etc.
- **Custom layouts**: `outCenter` (which becomes `effectiveOrigin` and then `prominentTile`) must point at the centerpiece. For `applyRotatableLayout()`, set `centerDx/centerDy` to the pattern row containing the main tile (e.g. ALTAR_INERT at row 0, not CARPET at row 3).
- **Ambient fixtures** (Dust Motes, Scorched Earth): No singular centerpiece — origin lands on a random scattered tile, which is fine since the flavor text describes the general area.

**Adding flavor text for a new fixture:**
1. Add an entry to `fixtureFlavorText[]` in `Globals.c` using the designated initializer syntax: `[MT_FIXTURE_NAME - MT_FIXTURE_FOUNTAIN] = "Your flavor text."`
2. Ensure the custom layout function sets `outCenter` to the centerpiece tile position
3. Write in Brogue's atmospheric style: one sentence, present tense, evocative sensory detail

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
