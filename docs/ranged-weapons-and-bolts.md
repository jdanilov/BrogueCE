# BrogueCE Ranged Weapons & Bolt System

## Thrown Weapons

### Throwing Mechanics
- **Main functions:** `throwCommand()` (`Items.c:6284`), `throwItem()` (`Items.c:6080`), `hitMonsterWithProjectileWeapon()` (`Items.c:5999`)

### Throwing Distance
```
maxDistance = 12 + 2 * max(rogue.strength - player.weaknessAmount - 12, 2)
```
- Base range: 12 cells minimum
- Each strength point above 12 adds 2 to max distance
- Weakness penalty reduces the strength modifier

### Thrown Weapon Types
- **DART** — lightweight, short range
- **JAVELIN** — medium weight
- **INCENDIARY_DART** — special: ignites flammable terrain and detonates, hits flammable obstructions instead of stopping before them (`Items.c:6142-6146`)

### Path Calculation
- Uses `getLineCoordinates()` with `BOLT_NONE` (no special pathfinding flags)
- Follows straight line-of-sight, not magical bolt pathfinding

### Hit Probability
- Uses `attackHit(thrower, monst)` — treats thrown weapon as if equipped for accuracy
- Weapon enchantment (`netEnchant()`) affects accuracy
- Auto-hit against stuck, paralyzed, or captive targets

## Bolt System (Unified for Staffs, Wands, & Monster Spells)

### Bolt Structure (`Rogue.h:1887-1900`)
Each bolt has: name, description, display glyph, colors, boltEffect, magnitude, pathDF (dungeon feature along path), targetDF (dungeon feature at target), forbiddenMonsterFlags, and behavioral flags.

### Bolt Types (30 total, `Rogue.h:918-949`)

**Offensive:**
- BOLT_LIGHTNING — passes through creatures, electric, magnitude 10
- BOLT_FIRE — fiery, magnitude 4
- BOLT_POISON — poison damage
- BOLT_DRAGONFIRE — powerful fire variant
- BOLT_SPARK — distance-scaled damage
- BOLT_DISTANCE_ATTACK — arrows/ranged physical attacks
- BOLT_POISON_DART — causes weakness
- BOLT_WHIP — melee-range extension

**Control:**
- BOLT_SLOW / BOLT_SLOW_2 — reduce speed
- BOLT_POLYMORPH — random creature transformation
- BOLT_NEGATION — strips all buffs/enchantments
- BOLT_DOMINATION — chance-based mind control
- BOLT_BECKONING — pulls target toward caster
- BOLT_ENTRANCEMENT — forces target to walk toward caster
- BOLT_DISCORD — makes target attack everyone
- BOLT_SPIDERWEB — ensnares target

**Utility:**
- BOLT_TELEPORT — random teleportation
- BOLT_BLINKING — controlled short-range teleport
- BOLT_TUNNELING — destroys walls, passes through creatures
- BOLT_OBSTRUCTION — creates crystal wall
- BOLT_CONJURATION — summons spectral blades

**Support:**
- BOLT_HEALING — restores HP
- BOLT_HASTE — increases speed
- BOLT_SHIELDING — adds temporary shield HP
- BOLT_PLENTY — splits creature into two
- BOLT_INVISIBILITY — grants invisibility
- BOLT_EMPOWERMENT — permanently enhances ally

### Bolt Flags (`Rogue.h:1874-1885`)
| Flag                           | Effect                                                            |
|--------------------------------|-------------------------------------------------------------------|
| `BF_PASSES_THRU_CREATURES`     | Bolt continues through creatures (lightning, tunneling)           |
| `BF_HALTS_BEFORE_OBSTRUCTION`  | Takes effect before obstacle (conjuration, obstruction, blinking) |
| `BF_TARGET_ALLIES`             | Auto-targets allies                                               |
| `BF_TARGET_ENEMIES`            | Auto-targets enemies                                              |
| `BF_FIERY`                     | Ignites flammable terrain and fire-sensitizes creatures           |
| `BF_NEVER_REFLECTS`            | Won't be deflected by reflective armor/creatures                  |
| `BF_NOT_LEARNABLE`             | Cannot be absorbed by empowered allies                            |
| `BF_NOT_NEGATABLE`             | Won't be erased by negation                                       |
| `BF_ELECTRIC`                  | Activates terrain with TM_PROMOTES_ON_ELECTRICITY                 |
| `BF_DISPLAY_CHAR_ALONG_LENGTH` | Shows bolt character along entire path                            |

## Bolt Pathfinding (`Items.c:3415-3560`)

`getLineCoordinates()` uses a sophisticated algorithm:
- Evaluates 21 candidate waypoints in a diamond shape around the target
- For magical bolts: scores all candidate paths and returns the best
- For thrown weapons (NULL bolt): returns center-of-target path

**Scoring system:**
- +2 per cell reached
- +50 for passing through flammable terrain (if BF_FIERY)
- +50 for hitting enemies (BF_TARGET_ENEMIES bolts)
- -200 for hitting allies (BF_TARGET_ENEMIES bolts)
- +5000 for reaching target (+2500 through unknown terrain)
- +50 per wall destroyed (tunneling)

## Bolt Travel & Effects

### Bolt Execution (`Items.c:4814-5100+`)
- `zap()` — main bolt execution function
- Bolt length: `5 * magnitude` cells
- Each cell: calls `updateBolt()` to check effects and collision

### Bolt Reflection (`Items.c:4206-4305`)
- `projectileReflects()` — checks reflection chance
  - Automatic: immunity armor vs vorpal enemy, MA_REFLECT_100
  - Chance-based: armor enchantment, exponential decay via `0.85^x`
  - MONST_REFLECT_50 creatures get bonus reflection chance
- `reflectBolt()` — if reflected back at caster, retraces path symmetrically; otherwise picks random perimeter target

### Effect Application (`Items.c:4362-4700+`)
`updateBolt()` applies effects based on `boltEffect`:
- **BE_ATTACK** — direct melee-style attack via `attack()`
- **BE_DAMAGE** — staff damage via `staffDamage()` (scales with enchantment, uses clumped random)
- **BE_TELEPORT** — random teleport or target blink
- **BE_SLOW** — duration = `magnitude * 5` turns
- **BE_HASTE** — duration via `staffHasteDuration()`
- **BE_POLYMORPH** — random creature transformation
- **BE_DOMINATION** — success probability via `wandDominate(monst)`
- **BE_NEGATION** — removes all buffs via `negate()`

## Ranged Attack Resolution

### Hit Probability (`Combat.c:105-133`)
```c
hitProbability = accuracy * defenseFraction(defense) / FP_FACTOR
```
- Accuracy: `monsterAccuracyAdjusted()` — base accuracy reduced by weakness (×3/2 penalty)
- Defense: exponential scaling via `0.877^x` where x is armor enchantment in 0.25-point increments
- Clamped to [0, 100]

### Auto-Hit Conditions
Stuck, paralyzed, or captive targets are always hit.

### Distance-Based Damage (`Items.c:4330`)
Some bolts (BOLT_SPARK) scale damage with distance:
```c
theBolt.magnitude = max(1, (distanceBetween(origin, target) - 2) / 2)
```

## Turrets and Ranged Monsters

### Turret Types
| Monster      | Accuracy | HP  | Bolt                 | Special         |
|--------------|----------|-----|----------------------|-----------------|
| Arrow Turret | 30       | 90  | BOLT_DISTANCE_ATTACK | 2-6 damage      |
| Dart Turret  | 20       | 140 | BOLT_POISON_DART     | Causes weakness |
| Flame Turret | 40       | 150 | BOLT_FIRE            | Fire damage     |

### Ranged Monster AI (`Monsters.c:2754-2804`)
- `monstUseBolt()` — decision logic: iterates valid targets (player first, then others)
- 30% chance per valid target to fire (100% if MONST_ALWAYS_USE_ABILITY)
- `generallyValidBoltTarget()` (`Monsters.c:2533-2560`) — basic validity (not self, open path, not hidden)
- `specificallyValidBoltTarget()` (`Monsters.c:2586-2687`) — bolt-specific checks:
  - Respects BF_TARGET_ALLIES/BF_TARGET_ENEMIES
  - Avoids reflectable bolts at reflective creatures
  - Fire bolts skip fire-immune creatures
  - Beckoning requires distance > 1

### MONST_MAINTAINS_DISTANCE
Ranged monsters like centaurs try to keep 3+ tiles between themselves and the player, retreating if the player closes in.

## Staff/Wand Damage Scaling (`PowerTables.c:49-53`)
```
staffDamageLow  = (2 + enchant) * 3/4
staffDamageHigh = 4 + (5 * enchant / 2)
```
Uses clumped random range (1 + enchant/3 clumps) for more consistent damage at higher enchantments.

## Key Files

| File              | Key Functions                                                                                                                                                              |
|-------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `Items.c`         | `throwCommand()` (6284), `throwItem()` (6080), `zap()` (4814), `updateBolt()` (4362), `projectileReflects()` (4206), `reflectBolt()` (4244), `getLineCoordinates()` (3415) |
| `Combat.c`        | `hitProbability()` (105), `attackHit()` (136), `monsterAccuracyAdjusted()` (98)                                                                                            |
| `Monsters.c`      | `monsterCastSpell()` (2754), `monstUseBolt()` (2776), `generallyValidBoltTarget()` (2533), `specificallyValidBoltTarget()` (2586)                                          |
| `PowerTables.c`   | `accuracyFraction()` (161), `defenseFraction()` (184), `staffDamage()` (51), `reflectionChance()` (109)                                                                    |
| `GlobalsBrogue.c` | `boltCatalog_Brogue[]` (58) — all 30 bolt definitions                                                                                                                      |
