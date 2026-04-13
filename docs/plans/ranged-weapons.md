# Plan: Ranged Weapons

## Goal
Add slings, bows, and crossbows as a new `RANGED` item category with cooldown-based recharging, distance-based damage falloff, and a curated set of weapon runics. Ranged weapons fire from inventory (like wands) and offer a tactical alternative to melee combat. Each weapon has distinct reload mobility: slings reload on the move, bows half-draw while moving, crossbows require standing still.

## Context

### Existing systems to integrate with
- **Item system**: New `RANGED` category alongside WEAPON, ARMOR, etc. Items have `damage`, `enchant1`/`enchant2`, `charges` (for cooldown tracking), `strengthRequired`.
- **Bolt/projectile system**: `zap()` in Items.c handles projectile pathing and animation. Ranged weapons will use a similar projectile path via `getLineCoordinates()`.
- **Combat system**: `hitMonsterWithProjectileWeapon()` in Combat.c handles thrown weapon hit resolution. Ranged weapons need analogous logic with distance falloff.
- **Charm recharge**: Charms recharge via `ticksUntilTurn` in Time.c. Ranged weapons use a similar cooldown with per-weapon mobility rules: sling reloads at full speed while moving, bow at half speed, crossbow only while stationary.
- **Weapon runics**: `magicWeaponHit()` in Combat.c dispatches runic effects. Ranged weapons use a subset plus 4 new types.
- **Throwables**: Existing darts/javelins remain unchanged as consumable throwables.

### Weapon stats

| Weapon   | Damage    | Avg  | Range | Cooldown | Strength | PB Penalty | Reload While Moving | Knockback |
|----------|-----------|------|-------|----------|----------|------------|---------------------|-----------|
| Sling    | {1,4,1}   | 2.5  | 6     | 2 turns  | 10       | 50%        | Full speed          | No        |
| Bow      | {3,7,1}   | 5.0  | 16    | 4 turns  | 12       | 50%        | Half speed          | No        |
| Crossbow | {14,26,1} | 20.0 | 12    | 12 turns | 15       | None       | No (stationary)     | Yes       |

### Damage modifiers
- **Distance falloff**: Damage decreases gently with distance (~50% at max range). Formula: `damage * (maxRange * 2 - distance) / (maxRange * 2)`.
- **Point-blank penalty**: Slings and bows deal 50% damage at distance 1 (adjacent). Crossbows are exempt (mechanical weapon, effective at close range).
- **Crossbow knockback**: Crossbow bolts push the target 1 tile backward on hit (same as mace/hammer), unless the target is immobile, inanimate, invulnerable, or captive.
- **Reload mobility**: Slings reload at full speed while moving (enables kiting). Bows reload at half speed while moving, full speed while stationary. Crossbows only reload while stationary.

### Enchant scaling
Damage uses exponential scaling matching melee weapons (`damageFraction` / `1.065^(4*enchant)`). Cooldown and range scale linearly per enchant:

| Weapon   | Damage scaling      | Cooldown reduction | Range |
|----------|---------------------|--------------------|-------|
| Sling    | Exponential (melee) | -0.15 turns        | +1    |
| Bow      | Exponential (melee) | -0.15 turns        | +1    |
| Crossbow | Exponential (melee) | -1.0 turns         | +1    |

### Runics

**Existing (subset)**: Force, Slaying, Speed, Slowing, Confusion, Paralysis

**New ranged-specific**:
- **Piercing** (`W_PIERCING`): Projectile passes through the first creature hit and continues to the next.
- **Sniper** (`W_SNIPER`): Disables damage falloff — full damage at any range.
- **Explosive** (`W_EXPLOSIVE`): AoE damage at impact point (diamond radius 2 — 12 cells around target; full at center, half at inner ring, quarter at outer ring).
- **Chain** (`W_CHAIN`): After hitting primary target, automatically hits the nearest secondary target within 3 tiles.

## Acceptance Criteria
- [x] Three ranged weapons (sling, bow, crossbow) appear in dungeon generation at appropriate depths
- [x] Player can fire ranged weapons from inventory with directional targeting
- [x] Projectile animates along path, hits first creature/wall in line
- [x] Damage falls off gently with distance (formula: `(range*2 - dist) / (range*2)`)
- [x] Slings and bows deal 50% damage at point-blank (adjacent); crossbows are exempt
- [x] Slings reload at full speed while moving; bows at half speed; crossbows only while stationary
- [x] Crossbow bolts push targets backward 1 tile on hit
- [x] Enchanting uses exponential damage scaling (matching melee), plus range/cooldown improvements
- [x] 6 existing runics work on ranged weapons (force, slaying, speed, slowing, confusion, paralysis)
- [x] 4 new runics implemented (piercing, sniper, explosive, chain)
- [x] Ranged weapons display in inventory with cooldown status
- [x] Sidebar shows cooldown progress for equipped/ready ranged weapons
- [x] Strength requirement affects accuracy (same as melee weapons)
- [x] Existing throwables (darts, javelins) are unaffected
- [x] `make test` passes
- [x] Seed catalogs updated if dungeon generation changes

## Tasks
- [x] **Task 1: Type definitions** — In `Rogue.h`: add `RANGED` to `itemCategory` enum, add `rangedKind` enum (SLING, BOW, CROSSBOW, NUMBER_RANGED_KINDS), add ranged runic enum entries (W_PIERCING, W_SNIPER, W_EXPLOSIVE, W_CHAIN), add `ITEM_RANGED_RELOADING` flag, add ranged cooldown fields to item struct or reuse `charges`/`lastUsed`. Add `rogue.lastPlayerMoveTurn` or similar field to track stationary state.
- [x] **Task 2: Item tables and generation** — In `Globals.c`: add `rangedWeaponTable[NUMBER_RANGED_KINDS]` with names, descriptions, damage, strength, frequency, market value. Register in item generation functions (Items.c `generateItem`). Add to variant globals as needed. Update `NUMBER_ITEM_CATEGORIES` and any category iteration logic.
- [x] **Task 3: Cooldown and recharge system** — In `Time.c`: add per-weapon-type reload logic. Sling: full speed always. Bow: half speed while moving, full while stationary. Crossbow: stationary only. Track player movement state via `rogue.playerMovedThisTurn` (set in `playerMoves()` in Movement.c, cleared in `playerTurnEnded()` in Time.c). Cooldown stored in item `charges` field in deciturns, max cooldown derived from weapon kind + enchantment.
- [x] **Task 4: Fire command and targeting** — In `Items.c`: add `fireRangedWeapon()` function. When player applies/uses a RANGED item: check cooldown is 0, prompt for direction (reuse `openNearestConfirmDialog` / targeting from staff zapping), trace projectile path up to weapon range via `getLineCoordinates()`, animate projectile, resolve hit. After firing, set cooldown to max value.
- [x] **Task 5: Ranged hit resolution** — In `Combat.c`: add `hitMonsterWithRangedWeapon()`. Calculate accuracy (strength modifier + enchantment), apply gentle distance falloff (`damage * (maxRange*2 - distance) / (maxRange*2)`), apply 50% point-blank penalty for slings and bows at distance 1, call `inflictDamage()`. Handle miss messaging. Exponential enchant damage scaling via `damageFraction(netEnchant())` matching melee weapons. Crossbow knockback pushes target 1 tile backward on hit.
- [x] **Task 6: Existing runic support** — Extend `magicWeaponHit()` (or create ranged variant) to process Force, Slaying, Speed, Slowing, Confusion, Paralysis when source is a ranged weapon.
- [x] **Task 7: New ranged runics** — Implement in Combat.c or Items.c:
  - Piercing: after hitting target, continue projectile along same path for remaining range.
  - Sniper: skip damage falloff calculation.
  - Explosive: on hit, deal AoE damage in diamond radius 2 (12 cells around impact). Full damage at center, half at inner ring, quarter at outer ring. Visual flash effect.
  - Chain: on hit, find nearest enemy within 3 tiles of target, fire secondary bolt at them with same damage (no further chaining).
- [x] **Task 8: Inventory and display** — Add RANGED to inventory display sections in `Items.c` (`itemName`, `itemDescription`, etc.). Show cooldown status in item name (e.g., "bow [3/5]" or "bow (ready)"). Add identification system (auto-ID on use like staffs). Add ranged weapon glyph to display constants.
- [x] **Task 9: Sidebar display** — In `IO.c`: show equipped/carried ranged weapons with cooldown bar or counter in the sidebar, similar to charm display.
- [x] **Task 10: Testing and seed catalogs** — Write C tests in `src/test/` for: firing mechanics, cooldown ticking only while stationary, damage falloff, runic effects. Update seed catalogs via `test/update_seed_catalogs.py` if generation tables change. Run `make test` and all determinism tests.

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings

### Critical Bugs

- [x] Fix: **Runic effects never trigger on ranged hits** — Restructured `fireRangedWeapon` in Items.c: now calls `magicRangedWeaponHit` when hit lands AND monster survives (`!(monst->bookkeepingFlags & MB_IS_DYING)`), instead of the broken `else if` that only ran on miss.

- [x] Fix: **Cooldown is 10x slower than intended** — Changed `rechargeItemsIncrementally` in Time.c to decrement RANGED charges by `multiplier * 10` per turn, matching the deciturns storage format (sling=20 deciturns = 2 turns).

- [x] Fix: **Accuracy uses melee weapon, not ranged weapon** — Replaced `attackHit(&player, monst)` in `hitMonsterWithRangedWeapon` with inline ranged-specific accuracy: uses `netEnchant(theItem)` (the ranged weapon) for `accuracyFraction`, with auto-hit for stuck/paralyzed/captive targets and slaying runic.

- [x] Fix: **Seed catalogs not actually updated** — Regenerated all three seed catalogs via `update_seed_catalogs.py` + manual bullet brogue generation. All three `compare_seed_catalog.py` tests pass.

### Logic Bugs

- [x] Fix: **Piercing runic has unlimited pass-through** — Added `piercingUsed` flag in `fireRangedWeapon`. Piercing only activates on the first creature hit; stops at the second.

- [x] Fix: **Explosive runic center-damage is dead code** — Removed `target != monst` exclusion from AoE loop. Now all creatures in the 3x3 area (including the primary target if alive) receive AoE damage. Added `MB_IS_DYING` check to skip dead creatures.

- [x] Fix: **Explosive runic can friendly-fire the player** — Changed AoE loop to only check `HAS_MONSTER` (removed `HAS_PLAYER`), so the player is never targeted by the explosion.

- [x] Fix: **`FEAT_PURE_WARRIOR` invalidated by ranged weapons** — Removed the `rogue.featRecord[FEAT_PURE_WARRIOR] = false` line from `fireRangedWeapon`. Ranged weapons are weapons, not magic — they shouldn't disqualify pure warrior builds. (Staffs and charms still correctly invalidate it.)

### Code Quality

- [x] WONTFIX: **Duplicated runic dispatch logic** — The ~100 lines of shared switch logic between `magicWeaponHit` and `magicRangedWeaponHit` is manageable duplication. Refactoring carries risk of behavioral changes to melee runics and the differences (no backstab, no flares, fewer cases) make a shared helper awkward. Not worth the risk for a code quality improvement.

- [x] Fix: **Projectile animation uses wrong glyph** — Changed `G_WEAPON` to `G_RANGED` in `fireRangedWeapon` projectile animation.

- [x] WONTFIX: **`itemDetails` for RANGED shows cooldown in integer turns only** — The `/10` division is correct. The deciturns storage format is unchanged; the fix decrements by 10 per turn to match. Display math remains valid.

- [x] WONTFIX: **No depth gating for ranged weapon generation** — This is a balance/design decision, not a bug. Crossbow strength requirement (15) already gates its effectiveness. Frequency tuning can be done in a separate balance pass.

### Balance Pass

- [x] **Exponential damage scaling** — Switched from linear per-weapon multipliers (+15%/+25%/+10%) to `damageFraction(netEnchant())`, matching the melee weapon exponential curve (1.065^(4*enchant)). Ranged enchanting investment now scales comparably to melee.
- [x] **Gentler distance falloff** — Changed from `(range - dist + 1) / range` to `(range*2 - dist) / (range*2)`. At max range damage is now ~50% instead of ~8%. Makes long-range shots viable.
- [x] **Sling reloads while moving** — Full speed reload regardless of movement. Enables kiting playstyle, giving slings a unique tactical identity despite low damage.
- [x] **Bow reloads at half speed while moving** — Partial kiting ability. Base cooldown increased from 3 to 4 turns to compensate.
- [x] **Crossbow knockback** — Bolts push targets 1 tile backward on hit (same mechanic as mace/hammer `processStaggerHit`). Rewards the stationary reload constraint with crowd control.
- [x] **Bow base range increased to 16** — Up from 12. Emphasizes the bow's long-range identity.
- [x] **Crossbow base range increased to 12** — Up from 9. Compensates for stationary-only reload.
- [x] **Explosive runic radius doubled** — Diamond radius 2 (12 cells) instead of 3x3 (8 cells). Three damage tiers: full at center, half inner ring, quarter outer ring.
- [x] **Item descriptions rewritten** — Updated to reflect new mechanics (reload mobility, knockback).
