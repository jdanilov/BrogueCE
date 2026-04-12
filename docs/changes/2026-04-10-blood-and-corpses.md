# Blood and Corpses

## Overview

Added cosmetic gore effects: persistent corpse glyphs when creatures die, blood trails from severely wounded creatures, increased blood splatter on hits, and directional blood splatter away from the attacker on melee strikes. All changes are purely visual -- no gameplay impact. Corpses do not block movement, interact with items, or affect pathfinding.

## Gameplay Effects

### Corpses on Death

When a non-inanimate creature with a blood type dies, a corpse tile (`%` glyph via `G_CORPSE`) is placed on the SURFACE layer at the death location. The corpse is colored according to the creature's blood type (red, green, purple, worm, acid, ash, ember, ectoplasm, rubble, or rot). Corpses have draw priority 75, which is higher than blood (80), so they visually override blood on the same tile. Corpses persist until the player leaves the level.

Corpse placement writes directly to the surface layer (`pmap[x][y].layers[SURFACE]`) rather than calling `spawnDungeonFeature()`, which avoids triggering spreading logic and avoids perturbing the game RNG.

### Directional Blood Splatter

Melee hits (attacker adjacent to defender) now spawn blood on 1-2 tiles behind the defender, extending the direction of the attack. The first tile behind the defender uses a moderate probability formula; the second tile uses a reduced formula. Tiles behind walls are skipped. Gas-layer blood types (e.g., rot gas) are excluded from directional splatter.

### Increased Blood Intensity

The base blood splatter formula in `inflictDamage()` was increased from `startProbability * (15 + min(damage, currentHP) * 3/2) / 100` to `startProbability * (30 + min(damage, currentHP) * 2) / 100`, producing noticeably more blood on hits.

### Blood Trails

Creatures (both player and monsters) below 25% HP leave blood on the tile they vacate when moving. This triggers with a 50% chance per move (`rand_percent(50)`). The trail blood uses a fixed `startProbability` of 30 with `probabilityDecrement` of 100, so it always produces a single-tile blood spot with no spread. Gas-layer blood types are excluded.

## New Terrain Types

Ten new `tileType` entries in `tileCatalog[]`, one per blood type:

| Tile Type | Glyph | Color Source | Description |
|-----------|-------|-------------|-------------|
| `CORPSE_RED` | `G_CORPSE` | `humanBloodColor` | "a corpse" |
| `CORPSE_GREEN` | `G_CORPSE` | `insectBloodColor` | "a corpse" |
| `CORPSE_PURPLE` | `G_CORPSE` | `poisonGasColor` | "a corpse" |
| `CORPSE_WORM` | `G_CORPSE` | `wormColor` | "worm remains" |
| `CORPSE_ACID` | `G_CORPSE` | `acidBackColor` | "acidic remains" |
| `CORPSE_ASH` | `G_BONES` | `ashForeColor` | "ash remains" |
| `CORPSE_EMBER` | `G_BONES` | `fireForeColor` | "ember remains" |
| `CORPSE_ECTOPLASM` | `G_FLOOR_ALT` | `ectoplasmColor` | "ectoplasmic remains" |
| `CORPSE_RUBBLE` | `G_BONES` | `gray` | "rubble remains" |
| `CORPSE_ROT` | `G_FLOOR_ALT` | `gray` | "putrid remains" |

All are SURFACE-layer tiles with `TM_STAND_IN_TILE` (no movement obstruction) and draw priority 75.

Ten corresponding `dungeonFeatureTypes` entries (`DF_CORPSE_RED` through `DF_CORPSE_ROT`) were added, each with 100% start probability and 0% decrement (single-tile, guaranteed spawn).

## New Display Glyph

`G_CORPSE` was added to the `displayGlyph` enum, mapped to Unicode `%` in `platformdependent.c`.

## Files Changed

| File | Changes |
|------|---------|
| `src/brogue/Rogue.h` | Added `G_CORPSE` to `displayGlyph` enum; added 10 corpse `tileType` entries (`CORPSE_RED` through `CORPSE_ROT`); added 10 corpse `dungeonFeatureTypes` entries (`DF_CORPSE_RED` through `DF_CORPSE_ROT`) |
| `src/brogue/Globals.c` | Added 10 corpse tile definitions in `tileCatalog[]` with per-blood-type glyphs, colors, and flavor text; added 10 corpse dungeon feature definitions in `dungeonFeatureCatalog[]` |
| `src/brogue/Combat.c` | Increased blood splatter formula in `inflictDamage()`; added directional blood splatter logic for melee hits (spawns blood on 1-2 tiles behind defender); added `bloodTypeToCorpseDF()` mapping function; added corpse placement in `killCreature()` for non-inanimate creatures with a blood type |
| `src/brogue/Monsters.c` | Added blood trail spawning in `moveMonster()` at both the swarming and normal movement code paths, for monsters below 25% HP |
| `src/brogue/Movement.c` | Added blood trail spawning in `playerMoves()` for the player when below 25% HP |
| `src/platform/platformdependent.c` | Added `G_CORPSE` to `glyphToUnicode()`, mapped to `%` |

## Implementation Details

### RNG Considerations

Blood trails and directional splatter call `spawnDungeonFeature()` and `rand_percent()`, which consume the game RNG. This changes the RNG sequence compared to before the feature, which breaks seed catalog determinism tests and recording playback. The plan doc notes three mitigation options; the corpse placement itself avoids this by writing directly to the pmap surface layer.

### Corpse Placement Strategy

Rather than using `spawnDungeonFeature()` for corpse placement, the code sets `pmap[x][y].layers[SURFACE]` directly to the corpse tile type. This bypasses the DF spreading/probability system entirely, ensuring exactly one corpse tile appears at the death location with no RNG consumption.

### Blood Trail Duplication

The blood trail logic is duplicated in two places in `Monsters.c` (swarming path and normal movement path) and once in `Movement.c` (player movement). All three use the same parameters: 50% trigger chance, `startProbability` 30, `probabilityDecrement` 100.
