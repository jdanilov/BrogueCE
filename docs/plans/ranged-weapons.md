# Plan: Ranged Weapons

## Goal
Add slings, bows, and crossbows as a new `RANGED` item category with cooldown-based recharging, distance-based damage falloff, and a curated set of weapon runics. Ranged weapons fire from inventory (like wands), reload only while stationary (preventing kiting), and offer a tactical alternative to melee combat.

## Context

### Existing systems to integrate with
- **Item system**: New `RANGED` category alongside WEAPON, ARMOR, etc. Items have `damage`, `enchant1`/`enchant2`, `charges` (for cooldown tracking), `strengthRequired`.
- **Bolt/projectile system**: `zap()` in Items.c handles projectile pathing and animation. Ranged weapons will use a similar projectile path via `getLineCoordinates()`.
- **Combat system**: `hitMonsterWithProjectileWeapon()` in Combat.c handles thrown weapon hit resolution. Ranged weapons need analogous logic with distance falloff.
- **Charm recharge**: Charms recharge via `ticksUntilTurn` in Time.c. Ranged weapons use a similar cooldown but only tick down when player is stationary.
- **Weapon runics**: `magicWeaponHit()` in Combat.c dispatches runic effects. Ranged weapons use a subset plus 4 new types.
- **Throwables**: Existing darts/javelins remain unchanged as consumable throwables.

### Weapon stats

| Weapon   | Damage    | Avg  | Range | Cooldown | Strength | PB Penalty |
|----------|-----------|------|-------|----------|----------|------------|
| Sling    | {1,4,1}   | 2.5  | 6     | 2 turns  | 10       | 50%        |
| Bow      | {3,7,1}   | 5.0  | 12    | 3 turns  | 12       | 50%        |
| Crossbow | {14,26,1} | 20.0 | 9     | 12 turns | 15       | None       |

### Damage modifiers
- **Distance falloff**: Damage decreases linearly with distance (full at point-blank, ~50% at max range). Formula: `damage * (maxRange - distance + 1) / maxRange`.
- **Point-blank penalty**: Slings and bows deal 50% damage at distance 1 (adjacent). Crossbows are exempt (mechanical weapon, effective at close range).
- **Stationary reload**: Cooldown only ticks down on turns where the player does not move. Moving pauses the timer (does not reset it).

### Enchant scaling (per +1 enchant)
Each weapon type has a distinct enchanting philosophy, making them feel different as investment targets:

| Weapon   | Damage scaling | Cooldown reduction | Range | Philosophy                            |
|----------|----------------|--------------------|-------|---------------------------------------|
| Sling    | +15%           | -0.15 turns        | +1    | Balanced, steady improvement          |
| Bow      | +25%           | -0.15 turns        | +1    | Weak→strong; high-investment payoff   |
| Crossbow | +10%           | -1.0 turns         | +1    | Barely hits harder, fires much faster |

**Approximate DPS at base vs +5 enchant** (at optimal range, stationary):

| Weapon   | Base DPS | +5 DPS | Comparison                              |
|----------|----------|--------|-----------------------------------------|
| Sling    | 1.25     | ~3.0   | Rapid chip damage                       |
| Bow      | 1.67     | ~5.5   | Rivals sword at high enchant            |
| Crossbow | 1.67     | ~3.3   | Reliable burst, big hits every ~7 turns |

### Runics

**Existing (subset)**: Force, Slaying, Speed, Slowing, Confusion, Paralysis

**New ranged-specific**:
- **Piercing** (`W_PIERCING`): Projectile passes through the first creature hit and continues to the next.
- **Sniper** (`W_SNIPER`): Disables damage falloff — full damage at any range.
- **Explosive** (`W_EXPLOSIVE`): AoE damage at impact point (3x3 area, reduced damage on edges).
- **Chain** (`W_CHAIN`): After hitting primary target, automatically hits the nearest secondary target within 3 tiles.

## Acceptance Criteria
- [ ] Three ranged weapons (sling, bow, crossbow) appear in dungeon generation at appropriate depths
- [ ] Player can fire ranged weapons from inventory with directional targeting
- [ ] Projectile animates along path, hits first creature/wall in line
- [ ] Damage falls off linearly with distance
- [ ] Slings and bows deal 50% damage at point-blank (adjacent); crossbows are exempt
- [ ] Cooldown only ticks while player is stationary; moving pauses the timer
- [ ] Enchanting increases damage, range, and reduces cooldown with per-weapon-type scaling (bow: fast damage, crossbow: fast cooldown, sling: balanced)
- [ ] 6 existing runics work on ranged weapons (force, slaying, speed, slowing, confusion, paralysis)
- [ ] 4 new runics implemented (piercing, sniper, explosive, chain)
- [ ] Ranged weapons display in inventory with cooldown status
- [ ] Sidebar shows cooldown progress for equipped/ready ranged weapons
- [ ] Strength requirement affects accuracy (same as melee weapons)
- [ ] Existing throwables (darts, javelins) are unaffected
- [ ] `make test` passes
- [ ] Seed catalogs updated if dungeon generation changes

## Tasks
- [ ] **Task 1: Type definitions** — In `Rogue.h`: add `RANGED` to `itemCategory` enum, add `rangedKind` enum (SLING, BOW, CROSSBOW, NUMBER_RANGED_KINDS), add ranged runic enum entries (W_PIERCING, W_SNIPER, W_EXPLOSIVE, W_CHAIN), add `ITEM_RANGED_RELOADING` flag, add ranged cooldown fields to item struct or reuse `charges`/`lastUsed`. Add `rogue.lastPlayerMoveTurn` or similar field to track stationary state.
- [ ] **Task 2: Item tables and generation** — In `Globals.c`: add `rangedWeaponTable[NUMBER_RANGED_KINDS]` with names, descriptions, damage, strength, frequency, market value. Register in item generation functions (Items.c `generateItem`). Add to variant globals as needed. Update `NUMBER_ITEM_CATEGORIES` and any category iteration logic.
- [ ] **Task 3: Cooldown and recharge system** — In `Time.c` (or wherever charm recharge ticks): add ranged weapon cooldown tick that only decrements when player hasn't moved. Track player movement state (e.g., flag set in `playerMoves()` in Movement.c, checked in `playerTurnEnded()` in Time.c). Cooldown stored in item `charges` field, max cooldown derived from weapon kind + enchantment.
- [ ] **Task 4: Fire command and targeting** — In `Items.c`: add `fireRangedWeapon()` function. When player applies/uses a RANGED item: check cooldown is 0, prompt for direction (reuse `openNearestConfirmDialog` / targeting from staff zapping), trace projectile path up to weapon range via `getLineCoordinates()`, animate projectile, resolve hit. After firing, set cooldown to max value.
- [ ] **Task 5: Ranged hit resolution** — In `Combat.c`: add `hitMonsterWithRangedWeapon()`. Calculate accuracy (strength modifier + enchantment), apply distance-based damage falloff (linear: `damage * (maxRange - distance + 1) / maxRange`), apply 50% point-blank penalty for slings and bows at distance 1, call `inflictDamage()`. Handle miss messaging. Implement per-weapon-type enchant scaling: bow gets +25% damage per enchant (high investment payoff), crossbow gets +10% damage but -1.0 cooldown per enchant, sling gets +15% damage and -0.15 cooldown (balanced).
- [ ] **Task 6: Existing runic support** — Extend `magicWeaponHit()` (or create ranged variant) to process Force, Slaying, Speed, Slowing, Confusion, Paralysis when source is a ranged weapon.
- [ ] **Task 7: New ranged runics** — Implement in Combat.c or Items.c:
  - Piercing: after hitting target, continue projectile along same path for remaining range.
  - Sniper: skip damage falloff calculation.
  - Explosive: on hit, call `inflictDamage()` on all creatures in 3x3 around impact, apply DF for visual effect.
  - Chain: on hit, find nearest enemy within 3 tiles of target, fire secondary bolt at them with same damage (no further chaining).
- [ ] **Task 8: Inventory and display** — Add RANGED to inventory display sections in `Items.c` (`itemName`, `itemDescription`, etc.). Show cooldown status in item name (e.g., "bow [3/5]" or "bow (ready)"). Add identification system (auto-ID on use like staffs). Add ranged weapon glyph to display constants.
- [ ] **Task 9: Sidebar display** — In `IO.c`: show equipped/carried ranged weapons with cooldown bar or counter in the sidebar, similar to charm display.
- [ ] **Task 10: Testing and seed catalogs** — Write C tests in `src/test/` for: firing mechanics, cooldown ticking only while stationary, damage falloff, runic effects. Update seed catalogs via `test/update_seed_catalogs.py` if generation tables change. Run `make test` and all determinism tests.

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings
(empty — populated during execution by the Critic)
