# BrogueCE - AI Agent Guide

## Build & Run

```bash
brew install sdl2 sdl2_image          # macOS deps
make bin/brogue                        # build (GNU Make)
./brogue                               # run
make TERMINAL=YES GRAPHICS=NO bin/brogue  # terminal-only build
make MAC_APP=YES GRAPHICS=YES Brogue.app  # macOS app bundle
make -B                                # force full rebuild (no `clean` target)
```

## Test

### C Test Harness (E2E behavioral tests)

```bash
make test                    # build and run all tests
```

Tests in `src/test/`. Runs headlessly via null platform, no SDL required. Each test calls `test_init_game(seed)`, injects actions via helpers like `test_move()` / `test_rest()` / `test_place_monster()`, asserts on game state, then calls `test_teardown_game()`. See `test_harness.h` for all helpers and assert macros. To add a suite: create `src/test/test_foo.c` with `TEST()` + `SUITE()` macros, register in `test_main.c`.

### Determinism tests (Python)

```bash
python3 test/compare_seed_catalog.py test/seed_catalogs/seed_catalog_brogue.txt 40
python3 test/compare_seed_catalog.py --extra_args "--variant rapid_brogue" test/seed_catalogs/seed_catalog_rapid_brogue.txt 10
python3 test/compare_seed_catalog.py --extra_args "--variant bullet_brogue" test/seed_catalogs/seed_catalog_bullet_brogue.txt 5
python3 test/run_regression_tests.py --num_processes=3 test/regression_test_ce_v1_14/
python3 test/run_regression_tests.py --num_processes=3 --extra_args "--variant rapid_brogue" test/regression_test_rb_v1_6/
python3 test/run_regression_tests.py --num_processes=3 --extra_args "--variant bullet_brogue" test/regression_test_bb_v1_1/
```

Seed catalog tests verify deterministic dungeon generation hasn't changed. Regression tests replay `.broguerec` recordings headlessly via null-platform. These will break if dungeon generation or turn-order logic changes.

## Architecture

```
src/brogue/     Core game logic (platform-independent)
src/platform/   Rendering backends (SDL2, ncurses, web, null/headless)
src/variants/   Game variant constants (Brogue, RapidBrogue, BulletBrogue)
```

- **Platform abstraction**: `struct brogueConsole` in `platform.h` — function pointers for plotChar, input, timing. Game code never calls platform directly.
- **Entry flow**: `main.c:main()` → `currentConsole.gameLoop()` → `rogueMain()` → `mainBrogueJunction()` → `initializeRogue()` → `mainInputLoop()`
- **Turn system**: Speed-based (`ticksUntilTurn`). After player input: monster turns → environmental updates → vision/scent recalc.
- **Dungeon**: Fixed 78x29 grid (`DCOLS`x`DROWS`). 4 terrain layers per cell. Levels persist in `levelData` array.
- **Global state**: `playerCharacter rogue` holds all game state. `gameConstants *gameConst` holds variant-specific balance.

## Key Files

| File | Purpose |
|------|---------|
| `src/brogue/Rogue.h` | Central header — ALL types, enums, constants (~46K lines) |
| `src/brogue/RogueMain.c` | Game init, `rogueMain()`, `startLevel()` |
| `src/brogue/IO.c` | Main game loop (`mainInputLoop`), display, input |
| `src/brogue/Time.c` | Turn processing, environment updates |
| `src/brogue/Monsters.c` | Monster AI, behavior, combat |
| `src/brogue/Architect.c` | Dungeon generation |
| `src/brogue/Items.c` | Item generation, identification, usage |
| `src/brogue/Combat.c` | Attack resolution, damage calc |
| `src/brogue/Movement.c` | Pathfinding, player/monster movement |
| `src/brogue/Globals.c` | Monster catalog, item tables, game constants |
| `src/brogue/Recordings.c` | Game recording/playback |
| `src/platform/platform.h` | Platform abstraction interface |
| `src/platform/main.c` | CLI arg parsing, platform selection |
| `src/platform/null-platform.c` | Headless backend (for testing/replay) |
| `src/variants/Globals*.c` | Per-variant game constants |
| `config.mk` | Build configuration options |

## Domain Terminology

- **Creature**: Any living entity (player + monsters + allies). Defined in `monsterCatalog[]`.
- **Item categories**: FOOD, WEAPON, ARMOR, POTION, SCROLL, STAFF, WAND, RING, CHARM, AMULET, GEM, KEY, GOLD.
- **Enchantment**: `enchant1`/`enchant2` on items. Scrolls of Enchanting increase these.
- **Bolt**: Unified magic projectile system (staffs, wands, monster spells all use `boltType`).
- **Machine**: Themed dungeon room/puzzle (vault, altar, trap sequence) defined by blueprints.
- **Dungeon Feature (DF_*)**: Procedural terrain effect (fire spread, flood, blood, growth).
- **Status effects**: Temporary creature modifiers (poison, haste, levitation, etc.) in `status[]` array.
- **Lumenstones**: Post-amulet collectibles for score.
- **Ally**: Creature with `creatureState == MONSTER_ALLY` and `leader` pointing to player.
- **Variants**: Different game balance via swapped `gameConstants` (Standard/Rapid/Bullet Brogue).

## Conventions

- Pure C codebase (C11). No C++.
- No dynamic plugin system — modify source directly or add a new variant.
- To add a variant: create `GlobalsYourVariant.c/h` in `src/variants/`, register in `MainMenu.c`.
- Seed catalogs MUST be updated (`test/update_seed_catalogs.py`) if dungeon generation changes.
- Game recordings break if turn-order or input-handling logic changes.
