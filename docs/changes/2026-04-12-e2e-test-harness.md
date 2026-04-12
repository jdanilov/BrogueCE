# E2E Test Harness

## Overview

Added a C-based end-to-end test harness that exercises core game systems by initializing real game instances, injecting player actions, and asserting on game state. Tests run headlessly via the null platform backend (no SDL or ncurses required). The harness includes 130 tests across 21 suites.

Build and run with:

```bash
make test
```

This builds `bin/brogue-tests` (linking all game sources except `src/platform/main.c` against the test runner) and executes it.

## How Tests Work

Each test follows the same lifecycle:

1. **Initialize** -- `test_init_game(seed)` sets the null console, initializes the standard Brogue variant, calls `initializeRogue()` and `startLevel()`, and enables `autoPlayingLevel` to suppress confirmation dialogs.
2. **Act** -- Helper functions inject player actions (`test_move()`, `test_rest()`) or manipulate game state directly (`test_place_monster()`, `test_teleport_player()`, `test_give_item()`, `test_set_terrain()`).
3. **Assert** -- Macros check conditions: `ASSERT(cond)`, `ASSERT_EQ(a, b)`, `ASSERT_NE`, `ASSERT_GT`, `ASSERT_GE`, `ASSERT_LT`, `ASSERT_LE`. On failure, the macro prints file/line/values and marks the test as failed.
4. **Teardown** -- `test_teardown_game()` sets `gameHasEnded` and calls `freeEverything()`.

### Test Framework Macros

| Macro | Purpose |
|-------|---------|
| `TEST(name)` | Declare a test function |
| `SUITE(name)` | Declare a test suite function |
| `RUN_TEST(name)` | Run a test and record pass/fail |
| `RUN_SUITE(name)` | Run all tests in a suite |

### Game Helper Functions

| Helper | Purpose |
|--------|---------|
| `test_init_game(seed)` | Start a new game with a given RNG seed |
| `test_teardown_game()` | Free all game resources |
| `test_move(direction)` | Move the player in a cardinal/diagonal direction |
| `test_rest()` / `test_rest_turns(n)` | Rest for one or N turns |
| `test_send_key(key, ctrl, shift)` | Inject a raw keystroke event |
| `test_place_monster(id, x, y)` | Spawn a monster at a position |
| `test_teleport_player(x, y)` | Move the player to a position |
| `test_give_item(category, kind, enchant)` | Add an identified item to the player's pack |
| `test_set_player_hp(hp, maxHp)` | Set player HP directly |
| `test_set_player_status(type, value)` | Apply a status effect to the player |
| `test_monster_at(x, y)` | Get the creature at a cell, or NULL |
| `test_count_monsters()` | Count living monsters on the level |
| `test_cell_has_terrain_flag(x, y, flag)` | Check terrain flags at a cell |
| `test_clear_area(cx, cy, radius)` | Clear a rectangle to open floor, removing monsters and items |
| `test_remove_all_monsters()` | Kill all monsters (active and dormant) |
| `test_set_terrain(x, y, layer, terrain)` | Set a terrain tile on a specific layer |
| `test_advance_environment(ticks)` | Run N environment update ticks (fire, gas, promotions) |
| `test_find_open_floor(x, y, out)` | Find a passable floor cell near a position |

## How to Add New Tests

1. Create `src/test/test_foo.c`.
2. Include `test_harness.h`.
3. Define tests with `TEST(test_name) { ... }` using `test_init_game` / assert macros / `test_teardown_game`.
4. Define a suite at the bottom: `SUITE(foo) { RUN_TEST(test_name); ... }`.
5. In `src/test/test_main.c`, add `extern void suite_foo(void);` and `RUN_SUITE(foo);`.

No other build changes are needed -- `make/test.mk` uses a wildcard to compile all `src/test/*.c` files.

## Test Suites

| Suite | Tests | Coverage |
|-------|-------|----------|
| `infrastructure` | 9 | Test helpers themselves: `clear_area`, `remove_all_monsters`, `set_terrain`, `advance_environment`, init/teardown stress (50 cycles), helper composition |
| `movement` | 6 | Player position after init, cardinal movement, wall blocking, rest advancing turns, multi-move sequences |
| `combat` | 3 | Melee attack reduces HP, player takes damage from adjacent monster, killing a monster removes it from the map |
| `status` | 7 | Depth level, game seed, starting equipment, poison damage, haste decay, nutrition decrease, HP regeneration |
| `items` | 4 | Giving weapons/armor to player, pack linked list integrity, floor items on generated levels |
| `environment` | 6 | Dungeon has walls/stairs, player on valid floor, monsters exist, seed determinism, different seeds produce different dungeons |
| `diagonal` | 3 | Diagonal movement, all four diagonal directions, diagonal blocked by two adjacent walls |
| `terrain` | 8 | Lava terrain flags, lava kills monsters, player refuses known lava, lava safe with levitation/fire immunity, chasm causes descent, deep water with levitation, spiderweb entanglement |
| `levels` | 5 | Descending/ascending stairs, player position after descent, deepest level tracking, depth increment |
| `fire_gas` | 5 | Fire spreads on grass, gas dissipates, fire does not spread to stone, fire ignites dead grass, environment update stability |
| `weapons` | 5 | Whip range-2 attack, axe hits all adjacent, spear penetration, weapon equip state, enchanted weapon kills weak monster |
| `ranged` | 5 | Dart quantity, staff charges, wand creation, incendiary darts, ranged monster attacks |
| `monsters` | 7 | Sleeping monster wakes when adjacent, monster moves toward player, ally follows player, monster attacks when adjacent, immobile monster stays put, monster count after kill, placed monster type |
| `item_usage` | 10 | Potions, scrolls, food, weapon/armor equipping, gold pickup, pack count, item identification flag, rings, charms |
| `combat_math` | 6 | Damage range validation, armor damage reduction, enchanted weapon effectiveness, haste/slow application and decay, strength vs heavy weapons |
| `status_effects` | 7 | Paralysis, confusion, levitation, invisibility, web entanglement, multiple coexisting effects, shielding absorbs damage |
| `vision` | 5 | Cells near player visible, distant cells not visible, discovered flag persistence, telepathy reveals monsters, adjacent cell visibility |
| `lifecycle` | 6 | Game starts not ended, deepest level tracked, gold accumulates, nutrition decreases, turn count increases, seed consistency |
| `edge_cases` | 7 | Map dimensions, player bounds, 1-HP monster kill, pack item limit, multiple init/teardown cycles, clear area passability, teleport validation |
| `keys` | 11 | Key/cage/portal item creation, key in pack, key identification, locked door blocks movement/vision, wooden door opens on step, open iron door passage, multiple keys, secret door blocking |
| `allies` | 5 | Ally creation and state, ally survives multiple turns, ally attacks enemy, multiple allies coexist, ally stays near player |

## Files Added

| File | Purpose |
|------|---------|
| `make/test.mk` | Build rules for the test binary and `make test` target |
| `src/test/test_harness.h` | Test framework macros, assertion macros, game helper declarations |
| `src/test/test_harness.c` | Game helper implementations, platform globals for test binary |
| `src/test/test_main.c` | Test runner `main()` -- registers and runs all suites |
| `src/test/test_movement.c` | Movement suite |
| `src/test/test_combat.c` | Combat suite |
| `src/test/test_combat_math.c` | Combat math suite |
| `src/test/test_status.c` | Status suite |
| `src/test/test_status_effects.c` | Status effects suite |
| `src/test/test_items.c` | Items suite |
| `src/test/test_item_usage.c` | Item usage suite |
| `src/test/test_environment.c` | Environment suite |
| `src/test/test_infrastructure.c` | Infrastructure suite |
| `src/test/test_diagonal.c` | Diagonal movement suite |
| `src/test/test_terrain.c` | Terrain suite |
| `src/test/test_levels.c` | Levels suite |
| `src/test/test_fire_gas.c` | Fire and gas suite |
| `src/test/test_weapons.c` | Weapons suite |
| `src/test/test_ranged.c` | Ranged items suite |
| `src/test/test_monsters.c` | Monsters suite |
| `src/test/test_vision.c` | Vision suite |
| `src/test/test_lifecycle.c` | Lifecycle suite |
| `src/test/test_edge_cases.c` | Edge cases suite |
| `src/test/test_keys.c` | Keys and doors suite |
| `src/test/test_allies.c` | Allies suite |

## Build Integration

The test target is defined in `make/test.mk`. It compiles all `src/test/*.c` files and links them against the full game object files (excluding `src/platform/main.c`, which has its own `main()`). The test binary uses the null platform (`nullConsole` from `src/platform/null-platform.c`) so no display libraries are needed.

The `test_harness.c` file re-declares the platform globals (`currentConsole`, `dataDirectory`, `serverMode`, etc.) that would normally come from `src/platform/main.c`, allowing the test binary to link without the real platform entry point.
