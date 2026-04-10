# Plan: Blood and Corpses

## Goal
Add immersive cosmetic gore: persistent corpse glyphs on death, blood trails from wounded creatures, increased blood splatter on hits, and directional blood splatter away from the attacker. All changes are cosmetic — no gameplay impact.

## Context
Brogue already has a solid foundation:
- 10 blood types (red, green, purple, acid, ash, ember, ectoplasm, rubble, rot gas, worm) assigned per creature via `bloodType`
- `inflictDamage()` in `Combat.c` spawns blood DF at defender's tile, scaled by damage
- Blood is SURFACE-layer terrain (layer 3) with `G_FLOOR_ALT` glyph and drawPriority 80
- `spawnDungeonFeature()` in `Architect.c` handles DF propagation with start probability and decrement
- `killCreature()` in `Combat.c` removes the creature but leaves no corpse tile
- 4-layer terrain system (DUNGEON/LIQUID/GAS/SURFACE) — corpses and blood both fit in SURFACE

Key files to modify:
- `src/brogue/Rogue.h` — new enums for corpse tile types, DF types, and any new flags
- `src/brogue/Globals.c` — new tileCatalog entries, dungeonFeatureCatalog entries
- `src/brogue/Combat.c` — `killCreature()` for corpse spawning, `inflictDamage()` for directional blood and increased splatter
- `src/brogue/Movement.c` — blood trail logic for wounded moving creatures
- `src/brogue/Time.c` — (if needed) blood trail on monster movement ticks

## Acceptance Criteria
- [ ] Killing a creature leaves a corpse glyph (`%` or similar) at the death location, colored by creature type
- [ ] Corpses persist permanently (until level exit/reload wipes transient state)
- [ ] Wounded creatures (below 25% HP) leave blood tiles on previously occupied cells when moving
- [ ] Blood splatter on hit is noticeably more dramatic (higher start probability, wider spread)
- [ ] Blood from melee hits spawns directionally — on tiles behind the defender relative to the attacker
- [ ] All 10 blood types produce appropriate corpse/trail variants (red corpse, ash remains, ectoplasm puddle, rubble pile, etc.)
- [ ] No gameplay changes — corpses don't block movement, don't interact with items, don't affect pathfinding
- [ ] Game compiles cleanly and existing seed catalog tests still pass (corpses are cosmetic surface layer, shouldn't affect deterministic dungeon gen)
- [ ] No regressions in recording playback

## Tasks

### Task 1: Add corpse tile types and DF entries
**Files:** `Rogue.h`, `Globals.c`

- Add new `tileType` entries for corpses in `tileCatalog[]` (e.g., `CORPSE_RED`, `CORPSE_GREEN`, `CORPSE_ASH`, `CORPSE_ECTOPLASM`, `CORPSE_RUBBLE`). Use glyph `G_BONES` or add a new `G_CORPSE` glyph (`%`). SURFACE layer, drawPriority ~75 (above blood at 80, lower number = higher priority). Flavor text: "the corpse of a fallen creature" etc.
- Add corresponding `dungeonFeatureTypes` entries (e.g., `DF_CORPSE_RED`, `DF_CORPSE_ASH`, etc.) in `dungeonFeatureCatalog[]` with 100% start probability and 0% decrement (single tile, no spread).
- Add a mapping from `bloodType` → `corpseDF` (either a lookup array or switch in code).

### Task 2: Spawn corpse on creature death
**Files:** `Combat.c`

- In `killCreature()`, after existing death logic, call `spawnDungeonFeature()` at the creature's `loc` with the appropriate corpse DF based on `info.bloodType`.
- Skip corpse spawning for creatures that shouldn't leave corpses (e.g., `MONST_INANIMATE` totems that already leave rubble, or phantoms/wisps that are ethereal). Use creature flags to decide.
- Ensure the corpse DF overwrites any existing blood on the same tile (higher draw priority).

### Task 3: Directional blood splatter on melee hit
**Files:** `Combat.c`

- In `inflictDamage()` (or the caller `attack()` which knows attacker position), calculate the direction vector from attacker to defender.
- Spawn additional blood DF on 1-2 tiles *behind* the defender (extending the attack direction). Use reduced probability for the farther tile.
- Only apply directional splatter for melee attacks (attacker adjacent to defender). Ranged/bolt attacks keep current behavior (blood at defender's feet).
- Handle edge cases: if the tile behind is a wall, skip it (blood doesn't go through walls).

### Task 4: Increase blood splatter intensity
**Files:** `Combat.c`, `Globals.c`

- In `inflictDamage()`, increase the blood probability multiplier. Current formula: `startProbability * (15 + min(damage, currentHP) * 3/2) / 100`. Increase the constant and/or multiplier so blood is more visible.
- Optionally increase `probabilityDecrement` reduction in blood DF catalog entries so blood spreads to more adjacent tiles on big hits.
- Consider adding a "heavy blood" variant DF with higher spread for killing blows or critical damage.

### Task 5: Blood trails from wounded creatures
**Files:** `Movement.c` or `Time.c` or `Monsters.c`

- When a creature (player or monster) moves and `currentHP < maxHP / 4`, spawn a blood DF at the *previous* position (the tile they just left).
- Use a reduced probability (e.g., 30-50%) so trails are intermittent, not solid lines.
- Scale probability by how wounded: more injured = more blood. Formula suggestion: `probability = 100 * (1 - currentHP/maxHP)` capped at some max.
- Use the creature's `bloodType` for the correct blood variant.
- Player movement: hook into `playerMoves()` or `movePlayer()` in `Movement.c`.
- Monster movement: hook into `moveMonster()` or the movement section of `monstersTurn()` in `Time.c` / `Monsters.c`.

### Task 6: Test and validate
- Build with `make -B bin/brogue`
- Playtest: verify corpses appear, blood trails work, directional splatter looks good
- Run seed catalog tests to confirm dungeon generation is unaffected
- Verify recording playback still works (cosmetic changes shouldn't break recordings since they don't affect turn order or RNG... but blood spawning may use RNG — investigate whether `spawnDungeonFeature` calls `rand()` and if so, whether this breaks determinism)

**RNG WARNING:** `spawnDungeonFeature()` likely calls the game RNG for probabilistic tile placement. Adding new DF spawns (corpses, trails, directional blood) will change the RNG sequence, which WILL break seed catalogs and recordings. Options:
1. Use a separate cosmetic RNG that doesn't affect the game RNG stream [recommended]
2. Accept the breakage and regenerate seed catalogs (`python3 test/update_seed_catalogs.py`)
3. Gate new features behind a flag and disable during recording playback

Option 2 is simplest. Option 1 is cleanest. Decide before implementing.

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings
(empty — populated during execution by the Critic)
