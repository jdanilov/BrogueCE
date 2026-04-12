# BrogueCE Dungeon Variety Overview

## Terrain Layer System

Every cell has **4 independent layers**, allowing rich environmental stacking:

| Layer       | Examples                                       |
|-------------|------------------------------------------------|
| **DUNGEON** | Walls, floors, doors, levers, statues, traps   |
| **LIQUID**  | Water, lava, ice, mud, chasms, brimstone       |
| **GAS**     | Poison gas, confusion gas, steam, methane      |
| **SURFACE** | Grass, foliage, fungus, blood, webs, ash, fire |

## Dungeon Features (DF_*) — 177 Types

These are procedural terrain effects that spawn, spread, and chain. Major categories:

**Hazards:** Fire (plain, gas, explosion, flamethrower, brimstone), poison/confusion/paralysis gas clouds, flooding water, collapsing floors, lava, nets, ice

**Vegetation:** Grass, foliage, fungus forest, dead grass/foliage, lichens, bloodflower pods, spider webs, ancient spirit vines — all with spread/regrow mechanics

**Water/Ice:** Deep/shallow water, swamp water, algae (3 growth stages), freeze/thaw cycles, mud

**Atmospheric:** Blood (red, green, purple, worm, acid, ash, ember, ectoplasm), bones, rubble, corpses, vomit, urine, unicorn poop

**Interactive:** Doors open/close, cages open/close, altars (commutation, resurrection, sacrifice), levers, portcullises, bridges activate/collapse, portals, glyphs, pressure plates

**Machine Components:** Poison/methane gas vents, turret emergence, wall shattering, statue cracking, coffin bursts, worm tunnels, electric crystals

Each feature has probability-based spawning, spread rates, light emission, and can chain into subsequent features.

## Machines (Blueprints) — 70 Templates

Machines are themed room templates placed during generation. Each has a **depth range** and **frequency weight**.

### Reward Rooms (14 types)
- **Item libraries** (depths 1-12): Caged items you can check
- **Treasure rooms** (depths 8-26): Potions and scrolls
- **Pedestal rooms** (depths 5-16+): Runic weapons, armor, staffs
- **Commutation altar** (depths 13-26): Swap item enchantments
- **Resurrection altar** (depths 13-26): Revive fallen allies
- **Ally locations**: Chained allies, kennels, vampire lairs
- **Legendary ally portal** (depths 8-26): Summons a champion
- **Goblin warren** (depths 5-15): Redesigned interior full of goblins
- **Sentinel sanctuary** (depths 10-23)

### Vestibules — Door Guardians (10 types)
- Locked doors (depths 1-26, freq 100 — most common)
- Secret doors, lever/portcullis mechanisms, flammable barricades
- Statue blockades, throwing tutorials, pit traps, glyph puzzles

### Key Holders — Puzzle Chambers (32 types)

| Depth Tier | Examples                                                                |
|------------|-------------------------------------------------------------------------|
| **1-8**    | Rat trap (paralysis + swarms), throwing tutorials, burning grass        |
| **3-13**   | Flood room (eel-infested), fire trap, thief chase, lava moat            |
| **4-26**   | Guardian puzzles (water/corridor/gauntlet), poison gas, sacrifice altar |
| **7-26**   | Explosive methane vents, web climbing over pits, boss rooms             |
| **12-26**  | Worms in walls, mud pit bogs, electric crystal puzzles, zombie crypts   |
| **16-26**  | Haunted house (darkness + phantoms)                                     |

### Flavor Machines — Decorative Themes (13 types)
- **Idyll** (depths 1-5): Peaceful ponds, grass, forest
- **Swamp** (mid depths): Mud, shallow water, gray fungus
- **Camp**: Hay, scavenged junk
- **Remnant** (depths 10+): Carpet, ash, crumbling statues
- **Dismal** (depths 7+): Blood, bones, charcoal
- **Lake walk**: Shallow water bridge with turrets
- **Shrine** (depths 5-26): Safe haven with supplies

## Depth-Based Progression

### Architecture
Room shapes shift with depth via `descentPercent = 100 * (depth-1) / (amuletLevel-1)`:
- **Shallow**: Cross rooms, small rooms, corridors common
- **Deep**: Caves and caverns dominate (cavern weight increases by `+50 * descentPercent / 100`)
- **Corridor chance** decreases with depth
- **Secret doors** scale from 0% (depth 1) to 67% (depth 26)

### Terrain
| Depth     | Terrain Changes                 |
|-----------|---------------------------------|
| **1-3**   | Water only, grass, foliage      |
| **4-9**   | Lava begins, dead grass appears |
| **7+**    | Luminescent fungus              |
| **10-13** | Bones, fungus forests increase  |
| **14+**   | Crystal walls                   |
| **16+**   | Steam vents                     |
| **17+**   | Brimstone replaces some lava    |

### Monsters
- **1-5**: Rats, kobolds, jackals, bloats
- **10-17**: Wraiths, zombies, trolls, will-o-wisps
- **18-26**: Revenants, golems, tentacle horrors, dragons
- **10% out-of-depth chance** to spawn monsters from 5 levels deeper
- **Mutations begin at depth 10**, scaling exponentially post-amulet

### Traps
- **Depths 2-6**: Poison gas, nets, paralysis, alarms (pre-revealed)
- **Depths 4-14**: Flamethrowers, flood traps
- **Depths 5+**: Hidden versions of all traps, increasing frequency

### Post-Amulet (Depths 27-40)
- Extreme monster mutations via exponential scaling
- 3 lumenstones per level initially, decreasing to 1
- Maximum difficulty encounters

## The Big Picture

BrogueCE doesn't use named biomes, but achieves environmental variety through **layered procedural systems**: terrain features spread and chain organically, machine blueprints create themed encounters gated by depth, and architectural parameters shift room shapes from structured corridors to open caverns. The result is a smooth difficulty curve where shallow levels feel civilized (grass, water, simple rooms) and deep levels feel alien (fungus forests, brimstone, vast caverns full of horrors).
