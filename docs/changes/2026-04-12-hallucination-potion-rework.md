# Hallucination Potion Rework

**Date:** 2026-04-12

## Summary

Reworked the Hallucination Potion from a pure-negative trap item into a tactically
interesting tool with offensive use, a thematic cure, perception benefits, and
an interaction with fear.

## Changes

### 1. Thrown Hallucination Potion Creates Gas (Offensive Use)

Previously the hallucination potion was the only malevolent potion that "splashed
harmlessly" when thrown. Now it creates a cloud of psychedelic gas (similar to
confusion/paralysis gas) that applies `STATUS_HALLUCINATING` to any creature
standing in it.

**Hallucinating monsters:**
- Move randomly 50% of the time (less severe than confusion's 100%)
- Retain their normal AI otherwise — they can still fight, just stumble around

**New game objects:**
- `HALLUCINATION_GAS` tile type with `T_CAUSES_HALLUCINATION` terrain flag
- `DF_HALLUCINATION_GAS_CLOUD_POTION` dungeon feature (radius 4, dissipates quickly)
- `HALLUCINATION_GAS_LIGHT` light source
- `hallucinationGasColor` — green-tinted shifting color (distinct from confusion's rainbow)

**Files:** Rogue.h, Globals.c, Globals.h, Items.c, Time.c, Monsters.c

### 2. Water Cures Hallucination

Stepping into water (any tile with `TM_EXTINGUISHES_FIRE`, which includes shallow
and deep water) now clears hallucination status. Uses the message "the cold water
clears your head." Applies to both player and monsters. Levitating creatures are
not affected (they don't touch the water).

**Files:** Time.c (`applyInstantTileEffectsToCreature`)

### 3. Hallucination Reveals Secrets

While hallucinating, there is a 3% chance per turn per nearby secret tile (within
radius 2, line-of-sight) to discover it. The player sees the message "your warped
vision catches a strange shimmer." This gives hallucination a silver lining —
your altered perception occasionally catches things normal senses miss.

**Files:** Time.c (player status processing)

### 4. Hallucination Grants Fear Immunity

Hallucinating creatures cannot be feared (`STATUS_MAGICAL_FEAR`). If a creature
becomes hallucinating while already feared, the fear is immediately cleared.
Applies to both the player and monsters.

**Files:** Time.c (player status processing), Monsters.c (`decrementMonsterStatus`)

## Determinism Impact

- Dungeon generation is **not** affected (no seed catalog updates needed)
- Game recordings involving thrown hallucination potions or hallucination near
  secrets may diverge due to new gas spawning and additional `rand_percent` calls
- The hallucination-reveals-secrets feature adds RNG calls during hallucination,
  which will affect replay determinism for turns where the player is hallucinating
  near secrets
