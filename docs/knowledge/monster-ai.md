# BrogueCE Monster AI System

## Creature States (`Rogue.h:2228-2234`)

Five core behavioral states:

| State                      | Behavior                                               |
|----------------------------|--------------------------------------------------------|
| **MONSTER_SLEEPING**       | Dormant until player approaches within awareness range |
| **MONSTER_TRACKING_SCENT** | Actively hunts player using scent map navigation       |
| **MONSTER_WANDERING**      | Patrols waypoints without player awareness             |
| **MONSTER_FLEEING**        | Retreats from threats using safety maps                |
| **MONSTER_ALLY**           | Follows and assists the player                         |

Additionally, **MODE_PERM_FLEEING** forces permanent flee behavior (e.g., thief monsters).

## State Transitions (`Monsters.c:1708-1822`)

```
Sleeping → Tracking/Wandering    (when detected via awareOfTarget())
Wandering → Tracking Scent       (when player enters FOV + alertMonster())
Tracking Scent → Wandering       (when awareOfPlayer becomes false)
Tracking/Wandering → Fleeing     (feared enemy within 3 tiles, or HP < 75% with FLEES_NEAR_DEATH)
Fleeing → Tracking Scent         (fear threshold >= 3 and no STATUS_MAGICAL_FEAR)
```

**Overrides:**
- `MONST_ALWAYS_HUNTING` — forces TRACKING_SCENT always
- `MODE_PERM_FLEEING` — forces FLEEING always

## Turn Decision Priority (`monstersTurn()`, `Monsters.c:3318-3624`)

Each monster's turn follows this priority order:

1. **Status checks** — corpse absorption, instant tile effects (paralysis, confusion)
2. **Skip if disabled** — paralyzed, entranced, captive, or sleeping
3. **State update** — `updateMonsterState()`
4. **Immobile monsters** — can only cast spells
5. **Discordant monsters** — attack nearest valid target
6. **TRACKING_SCENT:**
   - Try magic first (`monstUseMagic()`)
   - Blink toward player (30% chance if have BE_BLINKING)
   - Attack adjacent allies if player too far
   - Move toward player via scent maps
   - If no scent trail: switch to WANDERING
7. **FLEEING:**
   - Blink to safety
   - Summon minions
   - Move via safety maps
   - Attack nearby enemies if cornered
8. **WANDERING:**
   - Escape harmful terrain first
   - Attack captive leaders if healthy
   - **Follower logic:** if > 2 tiles from leader, path toward leader; else mill about (30% movement)
   - **Non-followers:** pathfind to waypoints, random movement as fallback
9. **ALLY:** calls `moveAlly()`

## Ally AI (`moveAlly()`, `Monsters.c:3039-3246`)

1. **Escape harmful terrain** — first priority
2. **Find enemies** — search all monsters, prioritize closest valid target
3. **Flee check** (`allyFlees()`, lines 2987-3016):
   - Within 10 tiles of enemy
   - HP < 33% AND (has FLEES_NEAR_DEATH flag OR HP fraction < player's)
   - OR target is damage-immune, kamikaze, or maintains distance
4. **Cast spells** before melee
5. **Combat distance leash:**
   - Normal: 4 tiles from player
   - While resting/searching: 10 tiles
   - Adjacent to enemy: always allow attack
   - Faster enemies: never restrict
6. **Blink toward enemies** — uses Dijkstra map for optimal destination
7. **Corpse absorption** — allies can absorb corpses to gain abilities
8. **Following behavior:**
   - Within 3 tiles and visible: mill about (30% movement)
   - Else: follow scent or use `pathTowardCreature()`

## Pathfinding Systems

### A. Scent-Based (`scentDirection()`, `Monsters.c:2832-2885`)
- Checks all 8 adjacent tiles for highest scent value
- Scent map represents decay trail from player position
- Handles diagonal stalls by diffusing scent into kinks
- Returns NO_DIRECTION if stuck at scent maximum

### B. Waypoint Pathing (WANDERING state)
- Each monster has `targetWaypointIndex` tracking current waypoint
- `chooseNewWanderDestination()` (lines 1220-1241) selects random unvisited waypoints
- 50% of waypoints active at any time, cycling creates patrol patterns
- Uses pre-computed `rogue.wpDistance[i]` maps

### C. Direct Pathing (`pathTowardCreature()`, `Monsters.c:2088-2131`)
- For creatures without pathable routes (e.g., separated by walls)
- Maintains/updates `target->mapToMe` grid lazily (recalculates if distance > 3)
- Can blink if target > 10 tiles away

### D. Passive Movement (`moveMonsterPassivelyTowards()`, `Monsters.c:1514-1575`)
- Moves toward target in nearest grid direction
- Non-tracking: tries orthogonal or diagonal (randomized preference)
- Falls back through alternates if primary blocked

## Awareness & Detection

### Awareness Distance (`Monsters.c:1620-1644`)
Returns approximate distance (roughly 2x actual) using minimum of:
1. **Scent distance:** `rogue.scentTurnNumber - scentMap[observer.x][observer.y]`
2. **Direct distance:** if target is in observer's FOV or open path exists

### Awareness Decision (`awareOfTarget()`, `Monsters.c:1648-1682`)
Compares awareness distance against `awareness = rogue.stealthRange * 2`:

| Condition                       | Result                           |
|---------------------------------|----------------------------------|
| MONST_ALWAYS_HUNTING            | Always aware                     |
| MONST_IMMOBILE (turrets)        | Aware only within stealth range  |
| Already tracking + beyond range | 97% chance to maintain awareness |
| In FOV but not hunting          | 25% chance to detect             |
| Beyond 3× stealth range         | Always unaware                   |

### Alert Chain
- `alertMonster()` (lines 1581-1583) — transitions to TRACKING_SCENT, records last seen position
- `wakeUp()` (lines 1586-1605) — full awakening + alerts all teammates with same leader
- Sets `ticksUntilTurn = 100` for immediate action

## Turn Processing (Speed-Based System, `Time.c:2376-2467`)

**Key concept:** `ticksUntilTurn` counter — faster creatures get more turns per player action.

1. Find minimum `ticksUntilTurn` across all creatures
2. Subtract that value from all creatures' counters
3. When `ticksUntilTurn <= 0`: call `monstersTurn()`
4. Set new counter based on action (movementSpeed, attackSpeed, etc.)

**Speed modifiers:**
- Spell casting: `attackSpeed × (CAST_SPELLS_SLOWLY ? 2 : 1)`
- Haste: halves effective speed values
- Slow: doubles effective speed values

## Monster Behavior Flags (`Rogue.h:2084-2122`)

### Combat
| Flag                       | Effect                                       |
|----------------------------|----------------------------------------------|
| `MONST_ALWAYS_HUNTING`     | Never sleeps/wanders, always TRACKING_SCENT  |
| `MONST_NEVER_SLEEPS`       | Always awake initially                       |
| `MONST_FLEES_NEAR_DEATH`   | Flees when HP < 75%, re-engages when > 75%   |
| `MONST_MAINTAINS_DISTANCE` | Keeps 3+ tiles from player (ranged monsters) |
| `MONST_IMMOBILE`           | Can only cast spells, no movement            |
| `MONST_CAST_SPELLS_SLOWLY` | Spells take 2× attack duration               |

### Movement
| Flag                         | Effect                                     |
|------------------------------|--------------------------------------------|
| `MONST_FLIES`                | Permanent levitation                       |
| `MONST_FLITS`                | Moves randomly 1/3 of the time             |
| `MONST_IMMUNE_TO_WEBS`       | Passes through webs                        |
| `MONST_RESTRICTED_TO_LIQUID` | Can only move in deep water (eels/krakens) |
| `MONST_WILL_NOT_USE_STAIRS`  | Won't chase player between levels          |

### Ability Flags (`Rogue.h:2132-2158`)
| Flag                    | Effect                                      |
|-------------------------|---------------------------------------------|
| `MA_ALWAYS_USE_ABILITY` | Never fails to use special ability          |
| `MA_CAST_SUMMON`        | Can summon minions                          |
| `MA_AVOID_CORRIDORS`    | Hunts in open areas, avoids narrow passages |

## Team Dynamics

### Attack Decision (`monsterWillAttackTarget()`, `Monsters.c:327-361`)
- Won't attack self or dying creatures
- Entranced creatures attack non-allies
- Captive creatures never attacked
- Discordant/confused: always attack (except allies)
- Otherwise: only if enemies via `monstersAreEnemies()`

### Team Relationships (`Monsters.c:363-399`)
**Teammates:** shared leader (MB_FOLLOWER flag), player + MONSTER_ALLY, two MONSTER_ALLY creatures

**Enemies:** discordant status overrides all; restricted-to-liquid monsters attack non-immune in deep water; allies vs. non-allies

## Special Behaviors

### Summoning (`monsterSummons()`, `Monsters.c:2408-2457`)
- Counts existing minions before summoning
- Allied summoners: count all allies across adjacent levels
- Max 50 minion cap for allied summoners
- Random chance scales down with existing minion count

### Corpse Absorption (`Monsters.c:3249-3287`)
- Takes 20 ticks to absorb a corpse
- Grants flags, ability flags, or new bolt types
- Used by empowered allies to gain legendary powers

### Blinking (`monsterBlinkToPreferenceMap()`, `Monsters.c:2289-2360`)
- Requires BE_BLINKING bolt effect
- Seeks locations with better "preference" values (scent for hunting, safety for fleeing)
- Costs same as spell casting (attackSpeed)

## Key Files

| File         | Key Functions                                                                                                                                                                                                                 |
|--------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `Monsters.c` | `monstersTurn()` (3318), `updateMonsterState()` (1708), `moveAlly()` (3039), `scentDirection()` (2832), `pathTowardCreature()` (2088), `awareOfTarget()` (1648), `monsterWillAttackTarget()` (327), `monsterSummons()` (2408) |
| `Time.c`     | Turn loop (2376-2467), player turn setup (2188)                                                                                                                                                                               |
| `Rogue.h`    | States (2228-2234), creature struct (2285-2336), flags (2084-2122), abilities (2132-2158)                                                                                                                                     |
