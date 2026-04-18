# Plan: Mimic Monster

## Goal
Add a rare Mimic monster disguised as a gold pile. When a player steps adjacent (1 tile), the gold transforms into a dangerous creature that seizes the player and drains gold per hit. Detectable via search, which spawns a sleeping mimic that can be avoided or attacked.

## Context
- Gold items are picked up in `pickUpItemAt()` (Items.c:1097) — instant add to `rogue.gold`, no inventory
- Gold is generated in `populateItems()` (Items.c:917) with heat-map placement
- Item flags use `Fl()` macros up to `Fl(23)` — `Fl(24)` is available
- Search function (Movement.c:2163) currently only reveals `TM_IS_SECRET` terrain
- `MA_SEIZES` already exists — first hit grabs (no damage), subsequent hits deal damage while player is held in place but can still attack back
- Monsters used by: bog monster, kraken, grappling turret

## Design

### Trigger: Proximity (1 tile)
When the player moves within 1 tile of mimic-gold:
1. Delete the gold item
2. Spawn a Mimic monster at the gold's location with stats scaled to `rogue.gold`
3. Log "the gold pile springs to life!"
4. Mimic gets a free attack (seize on first hit)

### Search Detection
When `search()` detects mimic-gold:
1. Log "you sense something wrong with that gold pile"
2. Remove the gold item
3. Spawn a **sleeping** Mimic at that location
4. Player can avoid it or attack it for bonus gold

### Stats (scaled by gold)
- HP: `40 + rogue.gold / 40` (e.g., 200g → 45 HP, 1000g → 65 HP, 4000g → 140 HP)
- Damage: `{4, 8, 2}` base + `rogue.gold / 100` bonus to min and max
- Gold drain per hit: `rogue.gold / 100` (1% of current gold). Drained gold is lost permanently.
- Ability: `MA_SEIZES` — latches onto player, preventing movement
- Display: `G_GOLD` glyph (`*`), gold/brown color
- On death: drops a fixed amount of bonus gold (not proportional to drained gold)

### Spawning
- D4-D15 only, during level creation (never mid-game spawning)
- ~15% chance per level to flag one gold pile as `ITEM_IS_MIMIC`
- Results in roughly 0-2 mimics per full run, median ~1

## Acceptance Criteria
- [ ] Mimic-gold looks identical to normal gold on the map
- [ ] Moving within 1 tile of mimic-gold triggers transformation + free attack
- [ ] Mimic HP and damage scale with `rogue.gold`
- [ ] Mimic seizes player on first hit (MA_SEIZES)
- [ ] Mimic attacks drain 1% of player's gold per hit (permanently lost)
- [ ] Search/awareness reveals mimic-gold → spawns sleeping mimic + message
- [ ] Mimics only placed during level creation, D4-D15
- [ ] Killing a mimic drops a fixed amount of bonus gold
- [ ] Mimic hidden from monster sidebar until spawned/revealed
- [ ] All existing tests pass (`make test`)

## Tasks
- [ ] 1. Add `ITEM_IS_MIMIC = Fl(24)` to `enum itemFlags` in Rogue.h
- [ ] 2. Add `MK_MIMIC` to monster kinds enum in Rogue.h. Add catalog entry in Globals.c with `G_GOLD` glyph, gold color, `MA_SEIZES` ability, base stats as placeholders (overwritten at spawn). Add to variant Globals files. Add horde entry (no members, D4-D15, low frequency).
- [ ] 3. In `populateItems()` (Items.c), after gold generation: if depth is D4-D15, ~15% chance to flag one random gold pile as `ITEM_IS_MIMIC`
- [ ] 4. Add proximity trigger: in the player movement code path, before processing the move destination, check all items within 1 tile of the player's new position. If any have `ITEM_IS_MIMIC`: delete the item, spawn Mimic creature at that location with stats scaled by `rogue.gold`, log message, give mimic a free attack turn.
- [ ] 5. Add gold drain on mimic attack: in combat resolution (Combat.c), if attacker is `MK_MIMIC`, drain `rogue.gold / 100` from player, log "the mimic devours X gold!"
- [ ] 6. Extend `search()` in Movement.c: iterate items at each searched cell. If item has `ITEM_IS_MIMIC` and search roll succeeds, remove the gold item, spawn a sleeping Mimic at that position, log "you sense something wrong with that gold pile"
- [ ] 7. On mimic death: drop a fixed gold pile (use DF or direct item spawn)
- [ ] 8. Add tests using arena-based test harness. Verify `make test` passes.
- [ ] 9. Create change doc in `docs/changes/`

## Ask User
(empty)

## Critic Findings
(empty)
