# Plan: Trap Hunting — Wand of Trapping

## Goal
Add a Wand of Trapping that lets the player extract discovered traps into portable inventory items and place them tactically on adjacent cells, enabling a "trap hunter" playstyle where the dungeon's own hazards become weapons.

## Context

### Current Trap System
Brogue has 8 trap types, all hostile to the player:
- **Poison Gas** — pressure plate spawns poison gas cloud (1000 volume)
- **Confusion Gas** — pressure plate spawns confusion cloud (300 volume)
- **Paralysis Gas** — wired machine vent, spawns paralysis gas
- **Net** — pressure plate drops netting (300 volume, 90 decrement)
- **Alarm** — pressure plate triggers `aggravateMonsters()` (DCOLS/2 radius)
- **Flamethrower** — directional vent shoots PLAIN_FIRE (100 start, 37 decrement)
- **Flood** — pressure plate spawns spreading water (NOT included — too environmental)
- **Trap Door** — floor gives way to chasm (NOT included — can't relocate a hole)

Traps exist as hidden/visible tile pairs in `tileCatalog[]`. They trigger via `T_IS_DF_TRAP` flag in `applyInstantTileEffectsToCreature()` (Time.c). Discovery uses `TM_IS_SECRET` + `discoverType` dungeon features. Monsters avoid traps via `monsterAvoids()` (Monsters.c).

### Design Decisions
- **6 eligible traps**: poison gas, confusion gas, paralysis gas, net, alarm, flamethrower
- **Flamethrower simplified**: when player-placed, creates a fire burst at the cell (not directional)
- **Paralysis simplified**: when player-placed, spawns a paralysis gas cloud (no machine wiring)
- **Flood and trap door excluded**: too environmental/structural to relocate
- **Pickup requires discovery**: player must search/reveal the trap first, then zap it
- **Placed traps**: hidden from monsters (TM_IS_SECRET behavior) but with a player-visible marker so the player remembers placement

## Acceptance Criteria
- [x] Wand of Trapping appears in dungeon loot with standard wand generation frequency
- [x] Zapping revealed trap: removes trap tile (becomes floor), adds corresponding trap item to inventory, costs 1 charge. Limit zap radius to 1.
- [x] Zapping non-trap tile or hidden trap: wand fizzles with message, no charge consumed
- [x] Trap is converted to a trap item of the respected type, right on the floor, which can be picked up
- [x] Applying trap item: prompts for direction (1 radius zap), places trap on adjacent floor cell, consumes item
- [x] Applying fails gracefully if target cell is not suitable (wall, chasm, water, existing trap)
- [x] Placed traps trigger normally when any creature steps on them (including player)
- [x] Placed traps are invisible to monsters (they don't path around them)
- [x] Placed traps have a player-visible marker so player can track placements
- [x] Monster AI does NOT avoid player-placed traps (treats them as hidden/secret)
- [x] All 6 trap types work correctly when picked up and placed
- [x] Wand identification follows standard wand ID rules
- [x] Tests cover pickup, placement, and triggering of each trap type
- [x] Seed catalogs updated if item generation tables change

## Tasks

### 1. Define new enums and constants in Rogue.h
- [x] Add `BOLT_TRAPPING` to bolt enum
- [x] Add `WAND_TRAPPING` to wand kind enum (or extend existing)
- [x] Add `TRAP` to item category enum (or define trap item sub-kinds)
- [x] Add 6 trap item kind constants: `TRAP_POISON_GAS`, `TRAP_CONFUSION_GAS`, `TRAP_PARALYSIS_GAS`, `TRAP_NET`, `TRAP_ALARM`, `TRAP_FLAMETHROWER`
- [x] Add `PLAYER_PLACED_TRAP` cell flag or `TM_PLAYER_TRAP` mech flag for player-visible-but-monster-hidden behavior

### 2. Add wand definition to Globals.c
- [x] Add Wand of Trapping entry to wand catalog (bolt type, charges 2-4, frequency)
- [x] Add bolt definition for `BOLT_TRAPPING` (short range or self-targeted, no damage, custom effect)
- [x] Add wand name, description, and identification text

### 3. Add trap item definitions to Globals.c
- [x] Add trap item catalog entries for all 6 types
- [x] Define names: "poison gas trap", "confusion gas trap", "paralysis gas trap", "net trap", "alarm trap", "flamethrower trap"
- [x] Define inventory descriptions and flavor text
- [x] Trap items should NOT appear in normal dungeon generation — only obtainable via wand

### 4. Implement wand pickup logic
- [x] In bolt processing (likely Items.c or Combat.c where bolts resolve):
  - Check if target cell has a revealed (non-secret) trap tile with `T_IS_DF_TRAP`
  - Identify which trap type is on the cell
  - Remove the trap tile (set to FLOOR/NOTHING)
  - Create the corresponding trap item and add to player inventory
  - Display message: "You extract the {trap name}."
- [x] If target has no revealed trap: display "The wand fizzles." and don't consume charge
- [x] Handle full inventory edge case

### 5. Implement trap placement action
- [x] When player "applies" a trap item:
  - Prompt for direction (adjacent cell selection)
  - Validate target cell: must be passable floor, no existing trap, no chasm/water/lava
  - Place the appropriate hidden trap tile on the cell
  - Set `TM_PLAYER_TRAP` flag (or equivalent) for player visibility
  - Consume the trap item from inventory
  - Display message: "You carefully set the {trap name}."
- [x] Placement takes a turn (like applying other items)

### 6. Add player-visible trap marker rendering
- [x] In rendering code (IO.c), check for `TM_PLAYER_TRAP` flag
- [x] Show a distinct glyph/color for player-placed traps (e.g., magenta pressure plate)
- [x] Monsters' `monsterAvoids()` should NOT detect player-placed traps (they have `TM_IS_SECRET`)

### 7. Handle trap-specific placement details
- [x] **Poison gas**: place `GAS_TRAP_POISON_HIDDEN` tile (uses standard trigger → `DF_POISON_GAS_CLOUD`)
- [x] **Confusion gas**: place `GAS_TRAP_CONFUSION_HIDDEN` tile (→ `DF_CONFUSION_GAS_TRAP_CLOUD`)
- [x] **Paralysis gas**: need new DF or reuse existing — place tile that triggers `DF_PARALYSIS_GAS_CLOUD` (may need new DF entry if one doesn't exist for standalone paralysis trap)
- [x] **Net**: place `NET_TRAP_HIDDEN` tile (→ `DF_NET`)
- [x] **Alarm**: place `ALARM_TRAP_HIDDEN` tile (→ `DF_AGGRAVATE_TRAP`)
- [x] **Flamethrower**: need modified behavior — place tile that triggers a fire burst DF (non-directional). May need new tile + DF entry, or reuse flamethrower with adjusted parameters.

### 8. Add to item generation tables
- [x] Add Wand of Trapping to wand generation table in appropriate variant Globals files
- [x] Set generation frequency (should be uncommon/rare — powerful tactical tool)
- [x] Trap items should have `ITEM_PREPLACED` or equivalent flag to prevent natural generation

### 9. Write tests
- [x] Test wand pickup: zap revealed poison gas trap → trap removed, item in inventory
- [x] Test wand fizzle: zap floor / hidden trap → no effect, no charge used
- [x] Test placement: apply trap item → trap appears on adjacent cell with correct tile
- [x] Test trigger: monster steps on player-placed trap → trap fires correctly
- [x] Test monster AI: monsters do NOT path around player-placed traps
- [x] Test edge cases: full inventory, invalid placement target, placing on self

### 10. Update seed catalogs and documentation
- [x] Run `test/update_seed_catalogs.py` if item tables changed
- [x] Update regression tests if needed
- [x] Create change doc in `docs/changes/`

## Ask User
(empty — populated during execution if Developer needs input)

## Critic Findings
(empty — populated during execution by the Critic)
