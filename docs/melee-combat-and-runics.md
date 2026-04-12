# BrogueCE Melee Combat & Runic Effects

## Damage Calculation

### Core Formula
```
damage = randClump(attacker.info.damage) * monsterDamageAdjustmentAmount(attacker) / FP_FACTOR
```

**Components:**
1. **Clumped random** (`Math.c:34-59`): Simulates dice rolls (clumpFactor=2 → 2d(range/2)), giving bell-curve distributions
2. **Damage fraction** (`PowerTables.c:138-159`): Table-based `1.065^x` scaling per 0.25 enchantment points, range ~0.2x–20x
3. **Strength modifier** (`Combat.c:60-67`):
   - Above requirement: **+0.25** per point
   - Below requirement: **-2.5** per point (10x harsher penalty)

### Sneak Attack Multipliers (`Combat.c:1101-1119`)
- **3x damage** if target is: sleeping, wandering (unaware), paralyzed, or lunge attack
- **5x damage** if using dagger with `ITEM_SNEAK_ATTACK_BONUS`
- Victim also loses a turn (`ticksUntilTurn += max(movementSpeed, attackSpeed)`)

### Net Enchant (`Combat.c:69-76`)
```c
netEnchant = enchant1 * FP_FACTOR + strengthModifier(theItem)
// Clamped to [-20, 50] * FP_FACTOR
```
Affects both accuracy and damage via lookup tables.

## Attack Resolution Flow (`attack()`, `Combat.c:1017-1273`)

1. **Pre-attack**: Kamikaze handling, seizing mechanics, status tracking
2. **Hit check**: Auto-hit if sneak/sleep/paralysis/lunge; else `rand_percent(hitProbability)`
3. **Damage roll**: Clumped random × sneak multiplier × enchantment scaling
4. **Armor runic**: `applyArmorRunicEffect()` may reduce/redirect damage
5. **Ring of Reaping**: Recharges items by damage dealt
6. **Damage applied**: `inflictDamage()` — returns true if target dies
7. **Post-attack**: Weapon degradation, special hit abilities, weapon runic via `magicWeaponHit()`

### Hit Probability (`Combat.c:105-133`)
```
hitProbability = accuracy * defenseFraction(defense) / FP_FACTOR
```
- Accuracy: base × `accuracyFraction(netEnchant)` (1.065^x per 0.25 points)
- Defense: `0.877^x` per 0.25 armor points (diminishing returns)
- Auto-hit: stuck, paralyzed, captive, slaying weapon vs vorpal enemy

## Weapon Runics (`Combat.c:591-786`)

All weapon runics defined in `Rogue.h:847-860`. Trigger chance via `runicWeaponChance()` (doubled on backstab).

| Runic | Effect | Duration/Details |
|-------|--------|-----------------|
| **W_SPEED** | Free turn (`ticksUntilTurn = -1`) | Instant |
| **W_QUIETUS** | Instant kill vs vorpal enemy | 100% trigger on vorpal match |
| **W_SLAYING** | Instant kill vs vorpal enemy | 100% trigger + 100% hit on vorpal |
| **W_PARALYSIS** | Paralyzes defender | `max(2, 2 + enchant/2)` turns |
| **W_MULTIPLICITY** | Spawns 1-7 spectral weapon images | 3-turn lifespan, `enchant/3` images |
| **W_SLOWING** | Slows defender | `(enchant+2)*(enchant+2)/3` turns |
| **W_CONFUSION** | Confuses defender | `max(3, enchant*3/2)` turns |
| **W_FORCE** | Knockback up to N cells | `max(4, enchant*2 + 2)` distance |
| **W_MERCY** | Heals defender | `onHitMercyHealPercent` (15%) of max HP |
| **W_PLENTY** | Clones defender | Creates full duplicate |

## Armor Runics (`Combat.c:808-981`)

Defined in `Rogue.h:872-886`.

### Good Runics
| Runic | Activation | Effect |
|-------|------------|--------|
| **A_MULTIPLICITY** | 33% on melee hit | Spawns 1-5 spectral clones of attacker (3 turns, 1 HP) |
| **A_MUTUALITY** | Always on melee hit | Shares damage equally with all adjacent enemies |
| **A_ABSORPTION** | Always | Reduces damage by `rand(1, enchant)` |
| **A_REPRISAL** | Melee hit only | Attacker takes `max(1, enchant*5% * damage)` counter-damage |
| **A_IMMUNITY** | Always vs vorpal | Complete immunity to vorpal enemy type |
| **A_REFLECTION** | Spell hit only | Deflects spells: `100 - 100*0.85^enchant`% chance |
| **A_RESPIRATION** | Passive | Immune to poison gas and steam |
| **A_DAMPENING** | Passive | Absorbs explosion knockback |
| **A_BURDEN** | 10% on hit | Permanently increases armor strengthRequired |

### Bad Runics (Cursed)
| Runic | Effect |
|-------|--------|
| **A_VULNERABILITY** | Doubles all incoming damage |
| **A_IMMOLATION** | 10% chance to ignite player on hit |

## Armor Defense System

### Defense Calculation (`Items.c:7704-7712`)
```
player.info.defense = (armor.armor + enchant * 10) / FP_FACTOR
```

### Base Armor Values (`Globals.c:1630-1637`)
| Armor | Defense | Strength Required |
|-------|---------|-------------------|
| Leather | 30 | 10 |
| Scale Mail | 40 | 12 |
| Chain Mail | 50 | 13 |
| Banded Mail | 70 | 15 |
| Splint Mail | 90 | 17 |
| Plate Mail | 110 | 19 |

### Donning Penalty
New armor has `STATUS_DONNING = armor/10` turns, reducing defense by 1 per remaining turn.

## Strength System

- **Starting strength**: 12
- **Increased by**: Potions of Strength
- **Reduced by**: `STATUS_WEAKENED` (effective = `rogue.strength - player.weaknessAmount`)
- **Asymmetric scaling**: Meeting requirements is critical — 1 point below costs 10x more than 1 point above benefits

## Lunge Attack (Rapier Special)
- Triggered when adjacent to enemy with 1-space gap, moving directly toward them
- Always hits, always triggers sneak attack bonuses (3x+ damage)
- Set via `lungeAttack=true` in `attack()` call

## Key Files

| File | Key Functions |
|------|--------------|
| `Combat.c` | `attack()` (1017), `hitProbability()` (105), `attackHit()` (136), `magicWeaponHit()` (591), `applyArmorRunicEffect()` (808), `strengthModifier()` (60), `netEnchant()` (69) |
| `PowerTables.c` | `damageFraction()` (138), `accuracyFraction()` (161), `defenseFraction()` (184), `runicWeaponChance()` (220) |
| `Math.c` | `randClump()` (34), `randClumpedRange()` (39) |
| `Items.c` | `recalculateEquipmentBonuses()` (7687) |
| `Globals.c` | `weaponTable[]` (1607), `armorTable[]` (1630) |
