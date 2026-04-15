# Plan: Loot & Spawn Enrichment

## Goal
Make item placement and monster loot more interesting through four independent, cherry-pickable improvements: room-aware item heat, guardian loot encounters, thematic monster loot filtering, and expanded monster carry rates.

## Context
Currently, items are placed via a heat map biased toward doors/secret doors (Items.c `populateItems`). Monsters and items are placed independently with no spatial relationship outside of machine blueprints. Monster carried items come from a pre-generated random hopper of 100 items. Only a handful of monster types carry items at all (via `MONST_CARRY_ITEM_25` / `MONST_CARRY_ITEM_100` flags).

Each improvement below is independent and can be implemented/tested/merged separately.

## Acceptance Criteria
- [ ] Item heat map incorporates room "interestingness" score (size, feature count, door count)
- [ ] Some sleeping/stationary monsters act as loot guardians (item on ground nearby or in inventory)
- [ ] Monster item assignment filters hopper items to match monster theme (e.g. ogres get weapons, spellcasters get staffs)
- [ ] More monster types carry items; carry rates are generally higher
- [ ] All changes pass `make test`
- [ ] Seed catalogs updated after each change

## Tasks

### 1. Room-Aware Item Heat Map
- [ ] In `populateItems()` (Items.c), after building the door-based heat map, add a second pass that scores each cell's "room interestingness"
- [ ] Scoring heuristic: flood-fill connected open floor from each cell to measure room size; count terrain features (altars, statues, glowing fungi, etc.) and doors within that room
- [ ] Add bonus heat proportional to score — large featured rooms get significant heat boost
- [ ] Ensure hallways and tiny closets get no bonus (they already have low heat)
- [ ] Consider capping bonus so it supplements rather than overwhelms door-based heat

### 2. Guardian Loot Encounters
- [ ] After `populateMonsters()` in `initializeLevel()`, run a new pass over placed monsters
- [ ] Identify eligible guardians: monsters that are `MONSTER_SLEEPING` and not `HORDE_MACHINE_ONLY`, not in a machine, and in a room (not hallway)
- [ ] For a fraction of eligible guardians (e.g. 30-40%), designate as loot guardians:
  - 50% chance: place item on ground within 1-2 tiles (visible enticement)
  - 50% chance: add item to monster's `carriedItem` (surprise reward)
- [ ] Guardian items are deducted from the level's normal item count to preserve game balance
- [ ] Items generated via `generateItem()` — should be decent quality (exclude food/gold)
- [ ] Ensure this doesn't double-place items on cells that already have one

### 3. Thematic Monster Loot (Filtered Hopper)
- [ ] Add a `preferredItemCategories` field (bitmask) to `creatureType` in Rogue.h indicating preferred loot categories
- [ ] When `initializeMonster()` assigns a hopper item, scan the hopper for one matching the monster's preferred categories
- [ ] If no match found, fall back to next available hopper item (current behavior)
- [ ] Define thematic affinities for key monster groups:
  - Ogres/trolls → WEAPON, ARMOR
  - Goblin mystics/conjurers → STAFF, WAND, RING
  - Dar priestess/battlemage → SCROLL, CHARM, STAFF
  - Naga → RING, CHARM
  - Dragons → GEM, GOLD (or any high-value)
  - Others → no preference (random as today)

### 4. Expanded Monster Carry Rates
- [ ] Audit `monsterCatalog[]` and add `MONST_CARRY_ITEM_25` to more mid/late-game monsters that currently carry nothing
- [ ] Consider a new flag `MONST_CARRY_ITEM_50` for a middle ground, or just use 25% more broadly
- [ ] Candidates: trolls, nagas, dar blademasters, liches, dragons, revenants
- [ ] Reduce hopper pre-generation from 100 to match expected consumption, or increase it if needed
- [ ] Ensure post-amulet levels still don't assign items (existing depth check)

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings
(empty — populated during execution by the Critic)
