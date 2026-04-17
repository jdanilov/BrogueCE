# Dungeon Fixtures

Added **31 decorative fixtures** — small landmark features placed in rooms and caverns that make each dungeon level feel more memorable and varied. Fixtures use only existing tile types arranged in themed clusters, ranging from tiny (1-3 cells) to small (20-50 cells). Some have minor gameplay effects (steam, light) or thematic loot drops.

## Design

- **Machine/blueprint system**: Each fixture is a blueprint with an auto-generator entry controlling spawn frequency and depth range. All use `BP_PURGE_INTERIOR` and `BP_NO_INTERIOR_FLAG`.
- **Two placement approaches**: Simple fixtures use standard blueprint `machineFeature` entries. Complex fixtures use custom `applyXxxLayout()` functions in Architect.c for precise geometric patterns (rings, grids, trails).
- **Rotatable layouts**: A reusable rotation system lets directional fixtures (Altar Nook, Forge, etc.) adapt to room orientation automatically.
- **Connectivity safety**: All multi-cell fixtures use `MF_TREAT_AS_BLOCKING` on obstructing tiles, plus a universal connectivity check that rejects placements breaking level pathability.
- **Machine placement tracking**: `rogue.placedMachines[]` records every built machine's blueprint index, machine number, and origin for testing and tooling.
- **Thematic loot**: 10 fixtures can spawn contextual items (weapons from Forge/Weapon Rack, scrolls from Toppled Bookcase, etc.) at 30-50% chance via `MF_ALTERNATIVE` flags.
- **Spawn frequency**: Tuned so ~75% of levels get at least one fixture, ~25% get two or more, and some get none.

## Fixture catalog

### Universal (any depth)

| Fixture | Description |
|---------|-------------|
| **Rubble Heap** | Broken column with scattered debris |
| **Lone Statue** | Imposing figure on marble pedestal |
| **Collapsed Pillar** | Broken column with asymmetric rubble scatter |
| **Drainage Channel** | Water seeping through cracked floor in a line |
| **Mossy Alcove** | Overgrown nook with seeping water |
| **Cobweb Corner** | Old webs with ancient remains in a wall corner |
| **Crumbled Wall** | Partially collapsed wall section |
| **Dust Motes** | Scattered ash and dried grass, undisturbed for ages |

### Early levels (D1-D8)

| Fixture | Description |
|---------|-------------|
| **Garden Patch** | Overgrown garden with alternating foliage and water rows |
| **Fountain** | Classic dungeon fountain on marble |
| **Mushroom Circle** | Glowing fairy ring in a damp hollow |
| **Sunlit Patch** | Shaft of light from a crack above |
| **Bird Nest** | Organic nest tucked into a crevice |
| **Vine Trellis** | Creeping vines hugging a wall |
| **Puddle** | Stagnant water in a muddy depression |

### Mid levels (D5-D18)

| Fixture | Description |
|---------|-------------|
| **Forge** | Ancient smithy with lava trough and obsidian platform |
| **Altar Nook** | Processional hall with carpet runner to altar on marble dais |
| **Crystal Outcrop** | Glowing mineral vein. Loot: GEM |
| **Steam Vent** | Geothermal fissure venting periodic scalding steam |
| **Abandoned Camp** | Fire ring camp with bedroll and marker post. Loot: FOOD/POTION |
| **Weapon Rack** | Broken equipment on wall. Loot: WEAPON |
| **Scorched Earth** | Aftermath of an old fire — ash and embers |
| **Lichen Garden** | Bioluminescent fungal archipelago with glowing pools |
| **Toppled Bookcase** | Bookcase in wall nook with scattered books. Loot: SCROLL |

### Deep levels (D10+)

| Fixture | Description |
|---------|-------------|
| **Bone Throne** | Grand throne on marble dais with bones and blood. Loot: RING/GOLD |
| **Blood Pool** | Altar rising from a lake of ancient blood |
| **Obsidian Formation** | Cooled lava remnants with concentric thermal rings |
| **Ember Pit** | Smoldering pyre with burnt stake |
| **Claw Marks** | Drag trail leading to predator's lair with depth-appropriate monster |
| **Sacrificial Slab** | Ritual stone with marble cross and eerie fungal glow. Loot: POTION |
| **Sulfur Crust** | Yellow deposits around a geothermal fumarole |

## Files changed

- `src/brogue/Rogue.h` — 31 `MT_FIXTURE_*` machine type constants, `placedMachineInfo` struct, `MAX_PLACED_MACHINES`
- `src/brogue/Architect.c` — 20+ custom `applyXxxLayout()` functions, rotatable layout system, connectivity check in `buildAMachine()`
- `src/brogue/Globals.c` — Fixture-related dungeon feature entries
- `src/variants/GlobalsBrogue.c` — 31 auto-generator entries, 31 blueprint definitions
- `src/variants/GlobalsRapidBrogue.c` — Same fixture entries for Rapid Brogue variant
- `src/variants/GlobalsBulletBrogue.c` — Same fixture entries for Bullet Brogue variant
- `src/test/test_fixtures.c` — Blueprint verification and placement tests
- `src/test/test_main.c` — Registered `suite_fixtures`
- `tools/scan_fixture_seeds.c` — Dev tool to find fixtures by seed/depth
- `test/seed_catalogs/` — All three seed catalogs regenerated
