# BrogueCE Status Effects System

## Status Array (`Rogue.h:2324-2325`)

Every creature has:
- `status[NUMBER_OF_STATUS_EFFECTS]` — countdown timer per effect (decrements each turn)
- `maxStatus[NUMBER_OF_STATUS_EFFECTS]` — starting/max value (used for status bar display)

## All Status Effects (27 total, `Rogue.h:2015-2044`)

### Buffs
| Status | Negatable | Effect |
|--------|-----------|--------|
| **STATUS_TELEPATHIC** | Yes (→1) | Sense monsters through walls |
| **STATUS_LEVITATING** | Yes (→1) | Float above water/hazards/traps |
| **STATUS_HASTED** | Yes | Increased movement/attack speed |
| **STATUS_IMMUNE_TO_FIRE** | Yes (→1) | Complete fire immunity |
| **STATUS_SHIELDED** | Yes | Damage absorption (decays 5%/turn) |
| **STATUS_INVISIBLE** | Yes | Hidden from monster sight (stealth range → 1) |
| **STATUS_EXPLOSION_IMMUNITY** | No | Blast immunity (5 turns, non-negatable) |

### Debuffs
| Status | Negatable | Effect |
|--------|-----------|--------|
| **STATUS_WEAKENED** | No | Reduced strength (`player.weaknessAmount`) |
| **STATUS_HALLUCINATING** | Yes | False monster names/appearances |
| **STATUS_SLOWED** | Yes | Doubled movement/attack speed values |
| **STATUS_CONFUSED** | Yes | Random movement direction |
| **STATUS_NAUSEOUS** | Yes | Vision distortion |
| **STATUS_DISCORDANT** | Yes | Hostile to all nearby creatures |
| **STATUS_DARKNESS** | Yes | Reduced vision radius |
| **STATUS_AGGRAVATING** | No | Increases monster detection range |

### Damage Over Time
| Status | Negatable | Effect |
|--------|-----------|--------|
| **STATUS_BURNING** | No | 1-3 fire damage/turn, emits light |
| **STATUS_POISONED** | No | `poisonAmount` damage/turn, blocks regen |

### Control
| Status | Negatable | Effect |
|--------|-----------|--------|
| **STATUS_PARALYZED** | No | Cannot move or act |
| **STATUS_STUCK** | No | Entangled in terrain (web/net) |
| **STATUS_ENTRANCED** | Yes | Monster pacified/walks toward caster |
| **STATUS_MAGICAL_FEAR** | Yes | Monster flees from player |
| **STATUS_ENRAGED** | No | Ignores corridor avoidance (4 turns) |

### Mechanical
| Status | Negatable | Effect |
|--------|-----------|--------|
| **STATUS_SEARCHING** | No | Search action intensity (0-5) |
| **STATUS_DONNING** | No | Armor equip penalty (counts down) |
| **STATUS_NUTRITION** | No | Food clock (0-2150) |
| **STATUS_ENTERS_LEVEL_IN** | No | Monster level transition timer |
| **STATUS_LIFESPAN_REMAINING** | No | Summoned creature death countdown |

## Stacking Rules

Most statuses use `max()` to keep the longer duration:
```c
monst->status[STATUS_CONFUSED] = max(monst->status[STATUS_CONFUSED], duration);
monst->maxStatus[STATUS_CONFUSED] = max(monst->maxStatus[STATUS_CONFUSED], duration);
```
Exception: `STATUS_DONNING` sets duration directly.

## Status Decrement

### Player (`decrementPlayerStatus()`, `Time.c:1969-2070`)

Each turn, active statuses decrement by 1 with special handling:
- **Nutrition**: Stops when paralyzed (unless holding amulet)
- **Telepathy/Darkness**: Updates vision on expiry
- **Haste/Slow**: Recalculates speed on expiry
- **Weakness**: Resets `weaknessAmount` on expiry
- **Donning**: Recalculates equipment on expiry
- **Stuck**: Only clears if terrain no longer entangles
- **Shield**: Decays at 5%/turn (`maxStatus/20`) instead of flat 1

### Monsters (`decrementMonsterStatus()`, `Monsters.c:1824-1974`)

Same countdown, but with permanent-flag overrides:
- `MONST_FLIES` → levitation never decrements (stays 1000)
- `MONST_FIERY` → burning never decrements
- `MONST_IMMUNE_TO_FIRE` → fire immunity never decrements
- `MONST_INVISIBLE` → invisibility never decrements
- Discordant allies revert to MONSTER_ALLY on expiry
- Magical fear reverts to prior state on expiry

## Key Status Interactions

| Interaction | Behavior |
|-------------|----------|
| Levitation + Water | Prevents drowning/entering water state |
| Levitation + Fire terrain | Fire on ground doesn't ignite levitating creatures |
| Levitation + Traps | Avoids auto-descent trap triggers |
| Confusion + Movement | Random direction for both player and monsters |
| Paralysis + Nutrition | No food consumed while paralyzed |
| Poison + Regeneration | Blocks HP recovery for both player and monsters |
| Invisible + Gas | Gas terrain reveals invisible creatures |
| Hallucination + Telepathy | Telepathy overrides hallucination (true names shown) |
| Shield + Damage | 10 shield points absorb 1 damage point |
| Darkness + Shadow | Stack multiplicatively for stealth range reduction |

## Nutrition / Food Clock

### Constants (`Rogue.h:1137-1140`)
```
STOMACH_SIZE         = 2150
HUNGER_THRESHOLD     = 350    (STOMACH_SIZE - 1800)
WEAK_THRESHOLD       = 150
FAINT_THRESHOLD      = 50
```

### Progression (`Time.c:814-843`)
- **2150-351**: Normal (no message)
- **350**: "you are hungry"
- **150**: "you feel weak with hunger" (requires acknowledgment)
- **50**: "you feel faint with hunger" (requires acknowledgment)
- **1**: Forces automatic eating of inventory food
- **0**: "you are starving to death!" → game over

Decrements by 1/turn. No consumption while paralyzed.

## Regeneration

### Player (`Time.c:2273-2287`)
- **Base**: Full HP in `TURNS_FOR_FULL_REGEN` (300) turns
- Uses `turnsUntilRegen` countdown (1000x fixed-point)
- **Bonus regen**: `player.regenPerTurn` from ring of regeneration
- **Blocked by**: `STATUS_POISONED`

### Monsters (`Monsters.c:1830-1838`)
- Same system with per-creature-type `turnsBetweenRegen`
- Also blocked by poison

## Poison Mechanics

### Application (`Combat.c:1641-1660`)
```c
void addPoison(creature *monst, short durationIncrement, short concentrationIncrement)
```
- Duration stacks additively
- Concentration (`poisonAmount`) increases damage rate
- Display bar: `maxStatus = maxHP / poisonAmount`

### Damage (`Monsters.c:1906-1920`)
- Per-turn damage = `poisonAmount` (flat, not scaling)
- Can kill creatures
- `poisonAmount` resets to 0 when status expires

## Negation

### Negatable Statuses (`Monsters.c:2462-2528`)
Telepathic, Hallucinating, Levitating, Slowed, Hasted, Confused, Discordant, Immune to Fire, Magical Fear, Entranced, Darkness, Shielded, Invisible

### Player Negation
Some statuses reduce to 1 instead of 0 when player is negated:
- STATUS_LEVITATING → 1 (prevents instant drowning)
- STATUS_TELEPATHIC → 1 (minimal telepathy)
- STATUS_IMMUNE_TO_FIRE → 1 (brief protection)

## Status Display (`IO.c:4609-4655`)

Sidebar shows progress bars for active statuses:
- HP bar (always shown)
- Nutrition bar with threshold labels
- Per-status bars with `statusEffectCatalog[i].name`
- Special labels: Weakened shows `-X`, Poisoned shows `X%`

## Key Files

| Component | File | Lines |
|-----------|------|-------|
| Status enum | Rogue.h | 2015-2044 |
| Status metadata struct | Rogue.h | 2046-2050 |
| Status catalog | Globals.c | 1819-1846 |
| Player decrement | Time.c | 1969-2070 |
| Monster decrement | Monsters.c | 1824-1974 |
| Nutrition thresholds | Time.c | 814-843 |
| Regeneration | Time.c | 2273-2287 |
| Poison application | Combat.c | 1641-1660 |
| Poison damage | Monsters.c | 1906-1920 |
| Negation | Monsters.c | 2462-2528 |
| Shield mechanics | Combat.c | 1533-1540 |
