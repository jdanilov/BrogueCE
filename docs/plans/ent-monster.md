# Plan: Ent Monster

## Goal
Add an Ent (E) — a slow, tanky tree creature appearing on D1-D4 that leaves a trail of vision-blocking foliage as it moves. The foliage decays to fungus, then to bare ground. The Ent prefers summoning small creatures (rats, monkeys, toads) over melee combat. Its foliage trail is flammable, making fire the natural counterplay. When the Ent panics (enters fleeing state at ≤75% HP), it erupts foliage in a radius-4 burst around itself (50% chance per tile), creating a sudden vision-blocking smoke screen.

## Context
- Monsters spawn DFs via `DFChance`/`DFType` fields on the `creatureType` struct
- Existing FOLIAGE/FUNGUS_FOREST terrains promote on step, but we need **time-based decay**, so we need new terrain types (ENT_FOLIAGE, ENT_FUNGUS) with `promoteChance > 0`
- Summoning uses `MA_CAST_SUMMON` + matching `HORDE_IS_SUMMONED` entries in horde catalog
- Summoning frequency self-limits: chance decreases as `minionCount²` increases
- Fleeing state transition happens in `updateMonsterState()` (Monsters.c ~line 1807) via `MONST_FLEES_NEAR_DEATH` when HP ≤ 75% max
- No existing monsters trigger special effects on entering fleeing state — this will be the first
- The DF propagation system uses wave-based cardinal spreading; a direct radius loop with `rand_percent(50)` is simpler for the burst effect

## Key Design

| Property | Value |
|----------|-------|
| Glyph | `E` (new G_ENT glyph) |
| Depth | D1-D4 |
| Speed | Slow (~200 movementSpeed) |
| HP | High (~80-100) |
| Damage | Low-moderate (3-7) |
| DFChance | 100% (foliage every turn) |
| Foliage decay | ~50 turns → fungus |
| Fungus decay | ~50 turns → ground |
| Summoning | Rats, monkeys, toads (capped by existing minion-count scaling) |
| Fire | Not immune; foliage is T_IS_FLAMMABLE |
| Behavior | MONST_MAINTAINS_DISTANCE + MA_CAST_SUMMON + MA_AVOID_CORRIDORS |
| Panic burst | On entering MONSTER_FLEEING: spawn DF_ENT_FOLIAGE at 50% chance per tile in radius 4 |

### Panic foliage burst
When the Ent's HP drops to ≤75% max, `MONST_FLEES_NEAR_DEATH` triggers the transition to `MONSTER_FLEEING` in `updateMonsterState()`. At that moment (once only), the Ent erupts foliage:

```
for dy in [-4..4], dx in [-4..4]:
  distSq = dx*dx + dy*dy
  if distSq > 16: skip              // circular cutoff at radius 4
  pct = 100 - distSq * 5            // 100% at center, 20% at edge
  if coordinatesAreInMap(x+dx, y+dy)
     && !T_OBSTRUCTS_PASSABILITY
     && rand_percent(pct):
    spawnDungeonFeature(x+dx, y+dy, DF_ENT_FOLIAGE)
```

Probability scales with squared distance: 100% at center, ~80% at distance 2, ~20% at radius 4, and tiles beyond the circle are skipped entirely. This produces a dense, naturally circular foliage cloud. The foliage is flammable, so a player with fire can burn through the screen — or ignite the Ent as it flees through its own foliage.

### Decay chain
```
ENT_FOLIAGE (vision-blocking, flammable)
  → promoteChance ~200 (~50 turns avg)
  → ENT_FUNGUS (luminescent, flammable, vision-blocking)
    → promoteChance ~200 (~50 turns avg)
    → bare ground (NOTHING on SURFACE layer)
```

## Acceptance Criteria
- [x] Ent appears on D1-D4, moves slowly, has high HP
- [x] Leaves dense foliage trail (blocks vision, flammable)
- [x] Foliage auto-decays to fungus (~50 turns), fungus decays to ground (~50 turns)
- [x] Ent summons rats/monkeys/toads/jackals periodically, prefers summoning over melee
- [x] Fire burns the foliage trail effectively (natural counterplay)
- [x] When Ent enters fleeing state, foliage bursts in radius 4 (50% per tile)
- [x] Panic burst only fires once (on state transition, not every fleeing turn)
- [x] `make test` passes
- [x] Seed catalogs updated

## Tasks
- [x] Add `ENT_FOLIAGE` and `ENT_FUNGUS` terrain types to `tileType` enum in Rogue.h
- [x] Add terrain catalog entries for both (with time-based promoteChance, T_IS_FLAMMABLE, T_OBSTRUCTS_VISION)
- [x] Add `DF_ENT_FOLIAGE`, `DF_ENT_FOLIAGE_DECAY`, `DF_ENT_FUNGUS_DECAY` to DF enum and catalog
- [x] Add `MK_ENT` to monster kind enum and monster catalog (slow, tanky, DFChance=100, DFType=DF_ENT_FOLIAGE)
- [x] Add `G_ENT` glyph constant
- [x] Add 3 summoning horde entries (MK_ENT → MK_RAT, MK_MONKEY, MK_TOAD) with HORDE_IS_SUMMONED
- [x] Add natural Ent horde entry for spawning on D1-D4
- [x] Add Ent color definition
- [x] Add panic foliage burst in `updateMonsterState()` — when Ent transitions to `MONSTER_FLEEING`, loop radius-4 square calling `spawnDungeonFeature(DF_ENT_FOLIAGE)` at `rand_percent(50)` per passable tile
- [x] Update seed catalogs
- [x] Verify with `make test`

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings
(empty — populated during execution by the Critic)
