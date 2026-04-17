# Plan: Ent Monster

## Goal
Add an Ent (E) — a slow, tanky tree creature appearing on D1-D4 that leaves a trail of vision-blocking foliage as it moves. The foliage decays to fungus, then to bare ground. The Ent prefers summoning small creatures (rats, monkeys, toads) over melee combat. Its foliage trail is flammable, making fire the natural counterplay.

## Context
- Monsters spawn DFs via `DFChance`/`DFType` fields on the `creatureType` struct
- Existing FOLIAGE/FUNGUS_FOREST terrains promote on step, but we need **time-based decay**, so we need new terrain types (ENT_FOLIAGE, ENT_FUNGUS) with `promoteChance > 0`
- Summoning uses `MA_CAST_SUMMON` + matching `HORDE_IS_SUMMONED` entries in horde catalog
- Summoning frequency self-limits: chance decreases as `minionCount²` increases

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

### Decay chain
```
ENT_FOLIAGE (vision-blocking, flammable)
  → promoteChance ~200 (~50 turns avg)
  → ENT_FUNGUS (luminescent, flammable, vision-blocking)
    → promoteChance ~200 (~50 turns avg)
    → bare ground (NOTHING on SURFACE layer)
```

## Acceptance Criteria
- [ ] Ent appears on D1-D4, moves slowly, has high HP
- [ ] Leaves dense foliage trail (blocks vision, flammable)
- [ ] Foliage auto-decays to fungus (~50 turns), fungus decays to ground (~50 turns)
- [ ] Ent summons rats/monkeys/toads periodically, prefers summoning over melee
- [ ] Fire burns the foliage trail effectively (natural counterplay)
- [ ] `make test` passes
- [ ] Seed catalogs updated

## Tasks
- [ ] Add `ENT_FOLIAGE` and `ENT_FUNGUS` terrain types to `tileType` enum in Rogue.h
- [ ] Add terrain catalog entries for both (with time-based promoteChance, T_IS_FLAMMABLE, T_OBSTRUCTS_VISION)
- [ ] Add `DF_ENT_FOLIAGE`, `DF_ENT_FOLIAGE_DECAY`, `DF_ENT_FUNGUS_DECAY` to DF enum and catalog
- [ ] Add `MK_ENT` to monster kind enum and monster catalog (slow, tanky, DFChance=100, DFType=DF_ENT_FOLIAGE)
- [ ] Add `G_ENT` glyph constant
- [ ] Add 3 summoning horde entries (MK_ENT → MK_RAT, MK_MONKEY, MK_TOAD) with HORDE_IS_SUMMONED
- [ ] Add natural Ent horde entry for spawning on D1-D4
- [ ] Add Ent color definition
- [ ] Update seed catalogs
- [ ] Verify with `make test`

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings
(empty — populated during execution by the Critic)
