# Ranged Weapons System

Added slings, bows, and crossbows as a new `RANGED` item category with cooldown-based recharging, distance-based damage falloff, and curated weapon runics.

## New item category: RANGED

Three ranged weapons with distinct identities:
- **Sling** — Fast, short-range chip damage (2-turn cooldown, range 6)
- **Bow** — Balanced mid-range weapon that scales well with enchantment (3-turn cooldown, range 12)
- **Crossbow** — Slow, powerful burst damage with fast cooldown scaling (12-turn cooldown, range 9)

## Core mechanics

- **Stationary reload**: Cooldown only ticks down when the player does not move. Moving pauses (but doesn't reset) the timer.
- **Distance falloff**: Damage decreases linearly with distance (`damage * (maxRange - distance + 1) / maxRange`).
- **Point-blank penalty**: Slings and bows deal 50% damage at distance 1. Crossbows are exempt.
- **Enchant scaling**: Each weapon type scales differently per +1 enchant:
  - Sling: +15% damage, -0.15 turns cooldown, +1 range
  - Bow: +25% damage, -0.15 turns cooldown, +1 range
  - Crossbow: +10% damage, -1.0 turn cooldown, +1 range

## Runics

Six existing weapon runics work on ranged weapons: Force, Slaying, Speed, Slowing, Confusion, Paralysis.

Four new ranged-specific runics:
- **Piercing** — Projectile passes through the first target and continues along its path
- **Sniper** — Disables damage falloff (full damage at any range)
- **Explosive** — AoE damage in a 3x3 area at the impact point
- **Chain** — After hitting, automatically fires at the nearest secondary target within 3 tiles

## Files changed

- `Rogue.h` — RANGED category, rangedKind enum, runic enums, item struct fields, function declarations
- `Globals.h/c` — rangedWeaponTable pointer, rangedRunicNames, itemCategoryNames updated
- `GlobalsBrogue.c`, `GlobalsRapidBrogue.c`, `GlobalsBulletBrogue.c` — Weapon tables and generation probabilities for all variants
- `Items.c` — Item generation, fireRangedWeapon(), cooldown/range calculation, itemName/description, enchanting, apply/negation support
- `Combat.c` — hitMonsterWithRangedWeapon(), magicRangedWeaponHit(), netEnchant() for RANGED
- `Time.c` — Ranged weapon cooldown recharge (stationary-only), playerMovedThisTurn reset
- `Movement.c` — playerMovedThisTurn flag set on player movement
- `IO.c` — RANGED added to inventory apply button
- `platformdependent.c` — G_RANGED glyph mapping (↑ arrow)
- `curses-platform.c` — G_RANGED terminal glyph (})
- `test_ranged.c` — 18 new tests for item creation, cooldown/range calculation, enchant scaling, cooldown recharge behavior
- Seed catalogs updated for all three variants
