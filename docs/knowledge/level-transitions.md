# BrogueCE Level Transition System

## Staircases Per Level

**Exactly 1 up + 1 down per level.** Stored in `levelData` as `upStairsLoc` and `downStairsLoc`. They must be at least `DCOLS / 3` (~26 tiles) apart.

**Special cases:**
- **Level 1**: Up stairs is `DUNGEON_EXIT` (escape route)
- **Deepest level**: Down stairs is `DUNGEON_PORTAL` (not regular stairs)

## Descending

**Via stairs** — walk onto the down staircase tile. Detected in `playerMoves()` at `Movement.c:1174-1176`, calls `useStairs(1)`.

**Via pit traps / chasms** — tiles with `T_AUTO_DESCENT` flag. Drops exactly **1 level** (not multiple) and deals **2d6 fall damage** (reduced if landing in liquid). Player gets a confirmation prompt: *"Dive into the depths?"* — unless confused. Handled by `playerFalls()` in `Time.c:977-1028`.

You always go down exactly one level at a time, whether by stairs or falling.

## Ascending

Walk onto the up staircase. `useStairs(-1)` decrements depth and restores the previous level's saved state.

## Level State Management

**Only the current level is ever simulated.** When you leave a level, it's frozen — the full map state is saved into `levels[].mapStorage` and the departure turn is recorded in `levels[].awaySince`.

### `startLevel()` — `RogueMain.c:547-803`

**Saving the old level (lines 639-663):**
- All 4 terrain layers, volume, flags, machine numbers saved per cell
- Remembered appearance (fog of war memory) preserved
- `awaySince` timestamp recorded

**Loading a new (unvisited) level (lines 674-739):**
1. `digDungeon()` — generate dungeon layout
2. `placeStairs()` — position up/down staircases
3. `initializeLevel()` — place monsters, items, machines
4. Amulet failsafe: if amulet level and no amulet was placed, one is dropped randomly

**Loading a revisited level (lines 745-784):**
- Full map state restored from `levels[].mapStorage`
- Monsters, items, dormant monsters restored from `levels[]` lists

### Environmental Catch-Up Simulation (lines 786-800)

After loading, the environment is simulated to make the level feel alive:

- **New level**: 50 turns of `updateEnvironment()` — swamp gas accumulates, brimstone percolates, fires spread, etc.
- **Revisited level**: `timeAway = absoluteTurnNumber - awaySince`, **clamped to max 100 turns**

During catch-up, the player is temporarily placed at `(0,0)` so environmental hazards don't damage them. **Off-screen levels are completely frozen** — no monsters move, no fires spread, no gas dissipates.

### Ally/Monster Following (lines 593-626)

Separate from environment simulation. Monsters near the stairs get `STATUS_ENTERS_LEVEL_IN` set based on their pathfinding distance to stairs divided by movement speed. They appear on the new level after that many turns — it's a timer, not actual simulation of them walking.

**Who follows:**
- Allies (`MONSTER_ALLY`)
- Monsters tracking scent (`MONSTER_TRACKING_SCENT`) — only via stairs, not pits (unless levitating)
- The Yendor Warden
- Must not be: captive, entranced, paralyzed, restricted to liquid, or a non-stair-using type
- Must be within pathfinding range (~30000 distance cap, waived for allies and warden)

## Staircase Placement — `Architect.c:3690-3761`

**Valid location (`validStairLoc`):** Must be in a wall tile where 3 of 4 cardinal neighbors are walls and 1 is passable (alcove shape). None of the 8 neighbors can be in a machine.

**Placement strategy:**
- Down stairs placed near previous level's down stairs location (visual continuity)
- Falls back to any valid location if that fails
- Torch walls added as decoration
- Surrounding walls marked impregnable
- 5-tile exclusion radius prevents stairs from being too close together

## Victory Conditions

- **Ascending from level 1 with the Amulet** → win (escaped the dungeon)
- **Descending at deepest level with the Amulet** → also triggers victory
- **Without the Amulet** → blocked with a message

## Key Files

| File                    | Key Functions                                            |
|-------------------------|----------------------------------------------------------|
| `RogueMain.c:547-803`   | `startLevel()` — main transition logic                   |
| `Movement.c:2191-2238`  | `useStairs()` — stair direction handling                 |
| `Movement.c:1173-1180`  | Stair detection in `playerMoves()`                       |
| `Architect.c:3604-3761` | `validStairLoc()`, `prepareForStairs()`, `placeStairs()` |
| `Time.c:977-1028`       | `playerFalls()` — pit descent                            |
| `Rogue.h:2603-2604`     | `levelData` struct with stair positions                  |
