# Wand of Trapping

Added the **Wand of Trapping** — a new wand that lets the player extract discovered traps into portable inventory items and place them tactically, enabling a "trap hunter" playstyle.

## Design

- **Wand**: Wand of Trapping (2–4 charges), uses new BOLT_TRAPPING bolt type
- **Extraction**: Zap a revealed (non-secret) trap to remove it from the dungeon and drop a corresponding trap item on the floor. Fizzles on non-trap tiles or hidden traps (no charge consumed).
- **6 trap types**: poison gas, confusion gas, paralysis gas, net, alarm, flamethrower
- **Placement**: Apply a trap item to place it on an adjacent floor cell as a hidden trap. Player-placed traps are invisible to monsters (TM_IS_SECRET) but shown to the player with a purple trap marker.
- **Trigger**: Placed traps trigger normally when any creature (including the player) steps on them. The HAS_PLAYER_PLACED_TRAP flag is cleared on trigger.
- **Special cases**: Paralysis uses a standalone tile (not machine-wired). Flamethrower uses a non-directional fire burst instead of a directional vent.

## Files Changed

- `src/brogue/Rogue.h` — Added WAND_TRAPPING, BOLT_TRAPPING, BE_TRAPPING, TRAP_ITEM category, trapItemKind enum, 4 new tile types (player paralysis/fire traps), 4 new DFs, HAS_PLAYER_PLACED_TRAP cell flag
- `src/brogue/Globals.h` — Declared `trapItemTable[]`
- `src/brogue/Globals.c` — Trap item table (6 entries), 4 tile catalog entries, 4 dungeon feature entries
- `src/brogue/Items.c` — Trap extraction in `updateBolt()`, fizzle in `detonateBolt()`, `useTrapItem()` placement, trap item generation/naming, charge skip on fizzle
- `src/brogue/IO.c` — Player-placed trap rendering (purple G_TRAP glyph)
- `src/brogue/Time.c` — Clear HAS_PLAYER_PLACED_TRAP on trap trigger
- `src/variants/GlobalsBrogue.c` — Bolt, wand, and generation table entries
- `src/variants/GlobalsRapidBrogue.c` — Same variant entries
- `src/variants/GlobalsBulletBrogue.c` — Same variant entries
- `src/test/test_trap_hunting.c` — 15 tests (extraction, fizzle, placement, trigger, monster AI, item generation, naming)
- `src/test/test_main.c` — Registered trap_hunting suite
- `test/seed_catalogs/` — All three seed catalogs updated
