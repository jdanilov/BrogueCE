# BrogueCE Vision, Stealth & Awareness Systems

## Vision / Field of View

### FOV Calculation
- **Primary function:** `getFOVMask()` (`Movement.c:2396-2404`)
- Uses **recursive shadowcasting** across 8 octants via `scanOctantFOV()` (lines 2407-2490)
- Maximum radius: `(DCOLS + DROWS) * FP_FACTOR` (~107 cells in fixed-point)
- Blocked by terrain with `T_OBSTRUCTS_VISION` flag (`Rogue.h:1950`)

### Visibility Flags (per cell, `pmap[x][y].flags`)
| Flag                   | Meaning                                          |
|------------------------|--------------------------------------------------|
| `IN_FIELD_OF_VIEW`     | Unobstructed line of sight (regardless of light) |
| `VISIBLE`              | In FOV AND sufficient light                      |
| `WAS_VISIBLE`          | Was visible previous turn                        |
| `DISCOVERED`           | Player has ever seen this cell (persists)        |
| `CLAIRVOYANT_VISIBLE`  | Seen via ring of clairvoyance                    |
| `CLAIRVOYANT_DARKENED` | Blocked by cursed clairvoyance ring              |
| `TELEPATHIC_VISIBLE`   | Seen via telepathy                               |
| `IS_IN_SHADOW`         | No light sources reaching this cell              |

**Helper macros** (`Rogue.h:1289-1290`):
- `playerCanDirectlySee(x, y)` = `pmap[x][y].flags & VISIBLE`
- `playerCanSee(x, y)` = `pmap[x][y].flags & ANY_KIND_OF_VISIBLE`
- `ANY_KIND_OF_VISIBLE` = `VISIBLE | CLAIRVOYANT_VISIBLE | TELEPATHIC_VISIBLE`

### Vision Update Cycle (`updateVision()`, `Time.c:742-801`)
Each turn:
1. `demoteVisibility()` — move current visibility to "was_visible" flags
2. Clear all `IN_FIELD_OF_VIEW` flags
3. Calculate player's FOV using shadowcasting on `T_OBSTRUCTS_VISION`
4. Mark calculated cells with `IN_FIELD_OF_VIEW`
5. If clairvoyance: `updateClairvoyance()`
6. `updateTelepathy()` for telepathic visibility
7. `updateLighting()` — calculate light values, set `VISIBLE` where light exceeds threshold
8. `updateFieldOfViewDisplay()` — finalize display, trigger discovery messages, store memories

## Lighting System

### Light Update (`updateLighting()`, `Light.c:208-281`)
Runs once per turn. Zeros all light values, marks all cells `IS_IN_SHADOW`, then paints light sources:

### Light Sources (in priority order)
1. **Glowing terrain tiles** — defined in tileCatalog with `glowLight` property
2. **Creature intrinsic light** — `monst->info.intrinsicLightType`
3. **Monster mutations** — some mutations have associated lights
4. **Burning creatures** — `STATUS_BURNING` emits light
5. **Telepathy light** — faint overlay for telepathically visible creatures
6. **Miner's light (player)** — main player light source

### Light Painting (`paintLight()`, `Light.c:54-116`)
- Uses `getFOVMask()` to determine affected cells
- Distance-based falloff: `lightMultiplier = 100 - (100 - fadeToPercent) * distance / radius`
- Dispels `IS_IN_SHADOW` flag if non-miner's-light

### Visibility Threshold
**`VISIBILITY_THRESHOLD = 50`** (`Rogue.h:184`)
- Cumulative RGB light must exceed 50 for a cell to be marked `VISIBLE`
- Calculated in `updateFieldOfViewDisplay()` (`Movement.c:2257-2259`)

### Miner's Light (`updateMinersLightRadius()`, `Light.c:120-154`)
Affected by:
- **Ring of Illumination** — positive `rogue.lightMultiplier` increases radius
- **Scroll of Darkness** — negative `rogue.lightMultiplier` reduces radius
- **STATUS_DARKNESS** — cubic scaling reduction, minimum 5%
- **Water submersion** — halves radius if `rogue.inWater`

### Display Detail Levels
`displayDetail` distinguishes: `DV_DARK`, `DV_UNLIT`, `DV_LIT` — player color changes based on darkness (`playerInDarknessColor`, `playerInShadowColor`, `playerInLightColor`).

## Stealth System

### Stealth Range Calculation (`currentStealthRange()`, `Time.c:676-716`)

**Base stealth range: 14 cells**

Modifiers applied sequentially:

| # | Modifier            | Effect                                         |
|---|---------------------|------------------------------------------------|
| 1 | **Invisibility**    | Override → stealth range = **1**               |
| 2 | **Darkness**        | Halve range if `playerInDarkness()`            |
| 3 | **Shadow**          | Halve again if standing in `IS_IN_SHADOW` cell |
| 4 | **Heavy armor**     | Add `max(0, strengthRequired - 12)`            |
| 5 | **Resting**         | Halve range (rounded up) if `rogue.justRested` |
| 6 | **Aggravation**     | Add `player.status[STATUS_AGGRAVATING]`        |
| 7 | **Ring of Stealth** | Subtract `rogue.stealthBonus`                  |

**Hard limits:** minimum 2 (minimum 1 if just rested), absolute minimum 1.

Darkness and shadow **stack multiplicatively** — standing in a dark shadow gives ×0.25 stealth range.

### How Stealth Range Is Used
Monsters compare awareness distance against `rogue.stealthRange * 2`. Updated every turn in `updateScent()` (`Time.c:649-665`).

## Scent System

### Scent Map
- Global 2D array `scentMap[x][y]` holding turn numbers
- One scent map per dungeon level, stored in `levels[]`

### Scent Spreading (`updateScent()`, `Time.c:649-665`)
- Called once per turn via `updateEnvironment()`
- Calculates FOV from player position using `T_OBSTRUCTS_SCENT` terrain flag
- `T_OBSTRUCTS_SCENT` includes: passability blockers, vision blockers, auto-descent, lava, deep water, spontaneously igniting terrain (`Rogue.h:1972`)

### Scent Addition (`addScentToCell()`, `Movement.c:2492-2496`)
```
value = rogue.scentTurnNumber - distance
scentMap[x][y] = max(value, scentMap[x][y])
```
Only affects cells that don't obstruct passability/scent.

### Scent Distance Metric (`scentDistance()`, `Time.c:641-647`)
Quasi-Euclidean weighted Manhattan:
```
if (|dx| > |dy|) return 2*|dx| + |dy|
else return |dx| + 2*|dy|
```
Returns approximately double the true distance.

### Monster Scent Tracking
- Monsters in `MONSTER_TRACKING_SCENT` state follow scent via `scentDirection()` (`Monsters.c:2832-2885`)
- Checks all 8 adjacent tiles for highest scent value
- Handles diagonal stalls by diffusing scent into kinks

## Noise & Aggravation

### Alarm Traps
- Tile types: `ALARM_TRAP` and `ALARM_TRAP_HIDDEN` (`Globals.c:393-394`)
- Triggers `DF_AGGRAVATE_TRAP` when stepped on

### Aggravation Mechanics (`aggravateMonsters()`, `Items.c:3358-3405`)
- Wakes all sleeping monsters within distance
- Alerts all non-ally monsters via `alertMonster()`
- Sets `STATUS_AGGRAVATING` on player (increases stealth range)
- Resets scent map and re-adds scent with modified distance
- Creates visual flash effect

### Monster Wake-Up Chain
- `wakeUp()` (`Monsters.c:1586-1605`) — sets `ticksUntilTurn = 100` (immediate action), alerts all teammates with same leader
- `alertMonster()` (`Monsters.c:1581-1584`) — transitions to TRACKING_SCENT, records `lastSeenPlayerAt`

## Invisibility

### Player Invisibility
- `STATUS_INVISIBLE` reduces stealth range to 1 (near-undetectable)

### Invisible Creature Detection (`monsterIsHidden()`, `Monsters.c:203-215`)
- Invisible creatures remain hidden **unless exposed by gas terrain**
- Check: invisible AND no gas in cell's GAS layer → hidden

### Other Hidden States
- **Dormant** (`MB_IS_DORMANT`) — hidden regardless, revealed when stepping on tile or attacking
- **Submerged** (`MB_SUBMERGED`) — invisible unless observer is in deep water and not levitating (`Monsters.c:179-192`)

## Telepathy

### Telepathy Update (`updateTelepathy()`, `Time.c:600-639`)
Each turn:
1. Resets all `TELEPATHIC_VISIBLE` flags
2. Iterates all monsters and dormant monsters
3. For each `monsterRevealed()` creature: marks 2-cell radius around monster as `TELEPATHIC_VISIBLE`

### Monster Revelation (`monsterRevealed()`, `Monsters.c:166-177`)
Returns true if:
1. Monster has `MB_TELEPATHICALLY_REVEALED` flag
2. Monster is `STATUS_ENTRANCED`
3. Player has `STATUS_TELEPATHIC` AND monster is not inanimate

## Clairvoyance

### Clairvoyance Update (`updateClairvoyance()`, `Time.c:552-598`)

**Positive clairvoyance** (Ring of Clairvoyance):
- Radius = `clairvoyance + 1`
- Creates circular blob of visibility: `dx² + dy² < radius²`
- Sets `CLAIRVOYANT_VISIBLE` and `DISCOVERED` flags

**Negative clairvoyance** (cursed ring):
- Radius = `|clairvoyance| - 1`
- Creates "blindness" effect
- Sets `CLAIRVOYANT_DARKENED` flag (prevents normal vision)

## Monster Awareness

### Awareness Distance (`awarenessDistance()`, `Monsters.c:1620-1644`)
Uses minimum of:
1. **Scent distance:** `rogue.scentTurnNumber - scentMap[observer.x][observer.y]`
2. **Direct distance:** if target is in observer's FOV or open path exists

### Awareness Decision (`awareOfTarget()`, `Monsters.c:1648-1682`)

| Condition                       | Result                          |
|---------------------------------|---------------------------------|
| `MONST_ALWAYS_HUNTING`          | Always aware                    |
| `MONST_IMMOBILE` (turrets)      | Aware only within stealth range |
| Already tracking + beyond range | 97% chance to maintain          |
| In FOV but not hunting          | 25% chance to detect            |
| Beyond 3× stealth range         | Always unaware                  |

### Monster FOV
Monsters calculate their own FOV for visibility checks:
```c
getFOVMask(monstFOV, monst->loc.x, monst->loc.y, DCOLS * FP_FACTOR, T_OBSTRUCTS_VISION, 0, false)
```
Used in pathfinding and ability target selection (`Monsters.c:1151`).

## Key Files

| Component            | File       | Lines           |
|----------------------|------------|-----------------|
| Vision update        | Time.c     | 742-801         |
| FOV shadowcasting    | Movement.c | 2396-2490       |
| FOV display          | Movement.c | 2247-2346       |
| Lighting system      | Light.c    | 208-281, 54-116 |
| Miner's light        | Light.c    | 120-154         |
| Scent update         | Time.c     | 649-665         |
| Scent distance       | Time.c     | 641-647         |
| Stealth range        | Time.c     | 676-716         |
| Clairvoyance         | Time.c     | 552-598         |
| Telepathy            | Time.c     | 600-639         |
| Monster awareness    | Monsters.c | 1620-1682       |
| Monster hidden state | Monsters.c | 179-215         |
| Alarm/aggravation    | Items.c    | 3358-3405       |
| Visibility flags     | Rogue.h    | 1092-1134       |
| Terrain flags        | Rogue.h    | 1948-1982       |
