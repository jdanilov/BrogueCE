# Ent Monster

Added the **Ent** (E) — a slow, tanky tree creature that appears on D1-D4 in foliage terrain. It leaves a trail of dense, vision-blocking foliage wherever it moves, summons small forest creatures to fight for it, and erupts a massive foliage cloud when it panics. Fire is the natural counterplay — both the Ent and its foliage are flammable.

## Design

- **Stats**: 80 HP, 40 defense, 200 move/attack speed (very slow), 3-7 damage. Large creature.
- **Foliage trail**: 100% chance to spawn ENT_FOLIAGE on every move. Blocks vision, flammable.
- **Decay chain**: ENT_FOLIAGE decays to ENT_FUNGUS (~50 turns avg), then to bare ground (~50 turns avg). Unlike regular foliage, ent foliage is not trampled by stepping on it — it only decays by timer.
- **Summoning**: Calls rats, jackals, monkeys, and toads via MA_CAST_SUMMON. Prefers summoning over melee (MONST_MAINTAINS_DISTANCE, MONST_CAST_SPELLS_SLOWLY). Summon count self-limits via existing minion-count scaling.
- **Panic burst**: When HP drops to ≤75% max (MONST_FLEES_NEAR_DEATH), the Ent erupts foliage in a radius-4 circle. Probability scales from 100% at center to 20% at the edge. Fires once on state transition only.
- **Behavior**: Avoids corridors (MA_AVOID_CORRIDORS), flees near death, always carries an item (MONST_CARRY_ITEM_100). Loot pool: potions, scrolls, staffs.
- **Spawning**: Appears in foliage terrain on D1-D4 (all three variants). Spawn weight 10 (Brogue), 10 (Rapid/Bullet).

## Files Changed

- `src/brogue/Rogue.h` — Added G_ENT glyph, MK_ENT monster kind, ENT_FOLIAGE/ENT_FUNGUS tile types, DF_ENT_FOLIAGE/DF_ENT_FOLIAGE_DECAY/DF_ENT_FUNGUS_DECAY dungeon features
- `src/brogue/Globals.c` — Ent color, two tile catalog entries with time-based decay, three DF catalog entries, monster catalog entry with flavor text
- `src/brogue/Monsters.c` — Panic foliage burst in `updateMonsterState()` on WANDERING→FLEEING transition
- `src/brogue/Items.c` — Fixed guardian loot to skip monsters without carry flags
- `src/platform/platformdependent.c` — G_ENT → 'E' glyph mapping
- `src/variants/GlobalsBrogue.c` — Ent horde (D1-D4, FOLIAGE) and 4 summoned minion hordes
- `src/variants/GlobalsRapidBrogue.c` — Same horde entries
- `src/variants/GlobalsBulletBrogue.c` — Same horde entries
- `src/test/test_ent.c` — 10 tests covering catalog properties, foliage spawning, vision/flammability, decay chain, step-through, placement, panic burst, and burst-once guard
- `src/test/test_main.c` — Registered ent suite
- `test/seed_catalogs/` — Both brogue and rapid_brogue catalogs updated
