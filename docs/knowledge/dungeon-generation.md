# BrogueCE Dungeon Generation Algorithm

## Top-Level Flow: `digDungeon()` (`Architect.c:2877-2976`)

1. **clearLevel()** — fill all cells with GRANITE
2. **carveDungeon(grid)** — generate room structure
3. **addLoops(grid, 20)** — add secondary connections (min pathing distance 20)
4. **Convert grid to pmap** — grid=1→FLOOR, grid=2→DOOR(60%) or FLOOR(40%)
5. **finishWalls(false)** — convert exposed GRANITE to WALL
6. **designLakes(lakeMap)** — generate lake shapes
7. **fillLakes(lakeMap)** — fill with liquids
8. **runAutogenerators(false)** — non-machine terrain (grass, fungus, torches)
9. **removeDiagonalOpenings()** — fix diagonal pathfinding exploits
10. **addMachines()** — place treasure rooms/puzzles/vaults
11. **runAutogenerators(true)** — machine-related terrain
12. **cleanUpLakeBoundaries()** — smooth lake edges
13. **buildABridge()** loop — add bridges over chasms
14. **finishDoors()** — remove orphans, create secret doors
15. **finishWalls(true)** — final wall/granite pass

## Room Types (18 total)

### Room Design Functions (`Architect.c`)

| Type | Function                       | Size                       | Description                                                       |
|------|--------------------------------|----------------------------|-------------------------------------------------------------------|
| 0    | `designCrossRoom()`            | 3-12 × 3-7                 | Two intersecting rectangles forming a cross                       |
| 1    | `designSymmetricalCrossRoom()` | 4-8 × 4-5                  | Perfectly symmetric cross                                         |
| 2    | `designSmallRoom()`            | 3-6 × 2-4                  | Simple rectangle                                                  |
| 3    | `designCircularRoom()`         | r=2-4 (90%) or 4-10 (10%)  | Circle, optionally with inner ring                                |
| 4    | `designChunkyRoom()`           | Variable                   | Central circle + 2-8 additional r=2 chunks                       |
| 5    | Cave (via `designCavern()`)    | 3-78 × 4-29                | Cellular automata blob (3 sub-variants by size)                   |
| 6    | Cavern                         | 50-78 × 20-29              | Full-level cellular automata blob                                 |
| 7    | `designEntranceRoom()`         | 8×10 + 20×5                | Depth 1 only, two stacked rectangles                              |
| 8    | `designLShapedRoom()`          | Arms: 3-6 × 5-10           | Two rectangles at a corner, 4 orientations                        |
| 9    | `designPillaredHall()`         | 7-13 × 5-9 (odd)           | Rectangle with interior pillar grid, 1-2 tile margin              |
| 10   | `designNestedRoom()`           | Outer: 7-11 × 5-9          | Donut room with hollow center (3-5 × 1-3) and one gap entry      |
| 11   | `designTrefoilRoom()`          | r=2-3 per circle            | Three overlapping circles in a triangle                           |
| 12   | `designCathedralRoom()`        | Main: 9-15 × 5-7           | Cross-shaped pillared halls. 2/3 open nave, 1/3 full pillars     |
| 13   | `designAlcoveRoom()`           | 7-13 × 4-7 (width odd)     | Rectangle with 1-cell niches along top/bottom walls every 2 cells |
| 14   | `designZShapedRoom()`          | Arms: 4-7 × 3-5            | Two rectangles on opposite corners, 2-3 wide connector            |
| 15   | `designTShapedRoom()`          | Bar: 7-12 × 2-4            | Bar with perpendicular stem (3-5 × 3-6), 4 orientations          |
| 16   | `designDiamondRoom()`          | r=3-5 (manhattan)           | Diamond shape via manhattan distance                              |
| 17   | `designDumbbellRoom()`         | r=2-4 per circle            | Two circles connected by narrow corridor (1-2 wide, 2-4 long)    |

### Cellular Automata Blobs (`Grid.c:417`)
`createBlobOnGrid()`: 5 iterations, 55% threshold, configurable seed patterns. Produces organic cave shapes.

## Room Frequency by Depth

Base profile **DP_BASIC** (`Globals.c`):
```
{Cross=2, SymCross=1, Small=1, Circular=1, Chunky=7, Cave=1, Cavern=0, Entrance=0,
 L-shaped=2, Pillared=1, Nested=1, Trefoil=1, Cathedral=1, Alcove=2, Z-shaped=2, T-shaped=2, Diamond=1, Dumbbell=1}
corridorChance=10
```

### Depth Adjustment (`Architect.c`)
```
descentPercent = 100 * (depth-1) / (amuletLevel-1)
```

| Room Type           | Shallow Bonus | Deep Bonus |
|---------------------|---------------|------------|
| Cross               | +20           | +0         |
| SymCross            | +10           | +0         |
| Circular            | +7            | +0         |
| Cave                | +0            | +10        |
| Cavern (first room) | +0            | +50        |
| L-shaped            | +3            | +0         |
| Pillared hall       | +2            | +0         |
| Alcove              | +3            | +0         |
| Z-shaped            | +2            | +0         |
| T-shaped            | +3            | +0         |
| Corridor chance     | +80           | +0         |

**Result**: Shallow levels → structured rooms with corridors. Deep levels → open caves and caverns.

### First Room Profile **DP_BASIC_FIRST_ROOM**:
```
{Cross=10, SymCross=0, Small=0, Circular=3, Chunky=7, Cave=10, Cavern=10, Entrance=0,
 L-shaped=3, Pillared=2, Nested=2, Trefoil=2, Cathedral=3, Alcove=2, Z-shaped=3, T-shaped=3, Diamond=2, Dumbbell=2}
corridorChance=0
```
Depth 1 overrides all to entrance room (type 7).

## Room Placement: `attachRooms()` (`Architect.c:2367-2423`)

1. Place first room using `designRandomRoom()` with first-room profile
2. Loop up to 35 attempts:
   - Create random room in "hyperspace" grid
   - Try placing at each coordinate (shuffled order)
   - Check room fits via `roomFitsAt()` — 3×3 boundary must be empty
   - Check door site alignment via `directionOfDoorSite()`
   - If match, insert via `insertRoomAt()` (recursive flood-fill)

## Corridor Generation (`Architect.c:2205-2258`)

### Parameters (`Rogue.h:1150-1153`)
- Horizontal: 5-15 cells long
- Vertical: 2-9 cells long
- Width: 1 cell
- 15% chance for oblique (perpendicular) exit branching

Corridors are attached to rooms via `attachHallwayTo()` when `corridorChance` passes.

## Secondary Connections: `addLoops()` (`Architect.c:340-395`)

Creates alternative paths between rooms:
1. For each empty cell between two floor cells on opposite sides
2. Measure Dijkstra pathing distance if door were created
3. If distance > 20 (minimum), create door (grid=2)
4. Prevents dead-end dungeons

## Liquid Placement

### Lake Generation: `designLakes()` (`Architect.c:2638-2687`)
- Generates blob-based lakes of decreasing size
- 20 random placement attempts per lake
- `lakeDisruptsPassability()` flood-fill check prevents disconnecting the level

### Liquid Type by Depth: `liquidType()` (`Architect.c:2518-2550`)

| Depth Range                         | Options                                      |
|-------------------------------------|----------------------------------------------|
| Before minimumLavaLevel (< 4)       | 100% DEEP_WATER + SHALLOW_WATER wreath       |
| Before minimumBrimstoneLevel (< 17) | 50% DEEP_WATER, 50% CHASM                    |
| From minimumBrimstoneLevel (17+)    | 25% each: LAVA, DEEP_WATER, CHASM, BRIMSTONE |
| Amulet level (26)                   | Always DEEP_WATER                            |

### Lake Filling: `fillLakes()` (`Architect.c:2710-2732`)
- Flood-fill from one cell with deep liquid
- Wreath of shallow liquid around filled area
- Bridges added later via `buildABridge()`

## Door Placement

### Structural Doors (grid=2 → pmap)
- 60% become DOOR tiles, 40% become FLOOR (random openings)

### Secret Doors: `finishDoors()` (`Architect.c:2733-2758`)
- Orphaned doors (no passable neighbors on both axes) → FLOOR
- Remaining doors: `secretDoorChance` to become SECRET_DOOR
- `secretDoorChance = clamp((depth-1) * 67 / (amuletLevel-1), 0, 67)`
  - Depth 1: 0%, Depth 13: ~33%, Depth 26: 67%

## Terrain Decoration: `runAutogenerators()` (`Architect.c:1780-1845`)

Called twice: once before machines, once after. Iterates `autoGeneratorCatalog[]`:
- Check depth constraints (minDepth/maxDepth)
- Calculate instance count: `min(intercept + depth*slope/100, max)`
- Spawn dungeon features (grass, fungus, torches, columns, bones, etc.)

## Special Level Features

### Depth 1
- Entrance room guaranteed (type 7: two stacked rectangles)
- All other room types zeroed out

### Amulet Level (26)
- `buildAMachine(MT_AMULET_AREA)` creates amulet holder
- Liquid type forced to DEEP_WATER
- Amulet failsafe: if no amulet placed, one spawns randomly

### Machine Placement: `addMachines()` (`Architect.c:1738-1754`)
- Bullet Brogue: guaranteed weapon vault on depth 1
- All variants: amulet area on amulet level
- Other machines: blueprint raffle based on depth range and frequency

## Dungeon Profile System (`Rogue.h:1903-1908`)

```c
typedef struct dungeonProfile {
    short roomFrequencies[ROOM_TYPE_COUNT];  // 18 room types
    short corridorChance;                     // 0-100
} dungeonProfile;
```

### Profiles (`Globals.c`)
| Profile               | Use              | Room Weights (types 0-7, then 8-17)              | Corridor |
|-----------------------|------------------|--------------------------------------------------|----------|
| DP_BASIC              | Standard dungeon | 2,1,1,1,7,1,0,0, 2,1,1,1,1,2,2,2,1,1           | 10       |
| DP_BASIC_FIRST_ROOM   | First room       | 10,0,0,3,7,10,10,0, 3,2,2,2,3,2,3,3,2,2        | 0        |
| DP_GOBLIN_WARREN      | Goblin machine   | 0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0           | 0        |
| DP_SENTINEL_SANCTUARY | Sentinel machine | 0,5,0,1,0,0,0,0, 0,0,0,0,0,0,0,0,0,0           | 0        |

## Dungeon Dimensions

- **DCOLS** = 78, **DROWS** = 29 (fixed grid)
- **CAVE_MIN_WIDTH** = 50, **CAVE_MIN_HEIGHT** = 20
- Corridor width: 1 cell

## Key Files

| File | Key Functions |
|------|--------------|
| `Architect.c` | `digDungeon()` (2877), `carveDungeon()` (2456), `attachRooms()` (2367), `addLoops()` (340), `designLakes()` (2638), `fillLakes()` (2710), `runAutogenerators()` (1780), `finishDoors()` (2733), `buildABridge()` (2786), `addMachines()` (1738) |
| `Architect.c` | Original rooms: `designCrossRoom()`, `designCircularRoom()`, `designChunkyRoom()`, `designCavern()`, `designEntranceRoom()`. New rooms: `designLShapedRoom()`, `designPillaredHall()`, `designNestedRoom()`, `designTrefoilRoom()`, `designCathedralRoom()`, `designAlcoveRoom()`, `designZShapedRoom()`, `designTShapedRoom()`, `designDiamondRoom()`, `designDumbbellRoom()` |
| `Grid.c` | `createBlobOnGrid()` (417), `drawCircleOnGrid()` (147), `drawRectangleOnGrid()` (137) |
| `Globals.c` | `dungeonProfileCatalog[]` (959) |
| `Rogue.h` | `dungeonProfile` struct (1903), corridor constants (1150) |
