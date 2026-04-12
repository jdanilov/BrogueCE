# BrogueCE Variant System

## gameConstants Structure (`Rogue.h:2389-2452`)

30 fields controlling variant-specific behavior:

### Version
- `majorVersion`, `minorVersion`, `patchVersion` — semantic versioning
- `variantName` — internal ID ("brogue", "rapidBrogue", "bulletBrogue")
- `versionString`, `dungeonVersionString`, `recordingVersionString`

### Dungeon Parameters
| Field | Brogue | Rapid | Bullet | Purpose |
|-------|--------|-------|--------|---------|
| `deepestLevel` | 40 | 10 | 5 | Max dungeon depth |
| `amuletLevel` | 26 | 7 | 3 | Level where amulet spawns |
| `depthAccelerator` | 1 | 4 | 8 | Multiplier for depth-dependent scaling |
| `minimumAltarLevel` | 13 | 4 | 2 | First altar depth |
| `minimumLavaLevel` | 4 | 2 | 2 | First lava depth |
| `minimumBrimstoneLevel` | 17 | 5 | 3 | First brimstone depth |
| `mutationsOccurAboveLevel` | 10 | — | — | Mutation start depth |
| `monsterOutOfDepthChance` | 10 | 10 | 2 | % chance for deeper monsters |

### Machine Generation
| Field | Brogue | Rapid | Bullet |
|-------|--------|-------|--------|
| `machinesPerLevelSuppressionMultiplier` | 4 | 2 | 1 |
| `machinesPerLevelIncreaseFactor` | 1 | 3 | — |
| `maxLevelForBonusMachines` | — | — | — |
| `deepestLevelForMachines` | — | — | — |

### Item Parameters
| Field | Brogue | Rapid | Bullet |
|-------|--------|-------|--------|
| `extraItemsPerLevel` | 0 | 4 | 8 |
| `weaponKillsToAutoID` | 20 | 5 | 2 |
| `armorDelayToAutoID` | 1000 | 250 | 120 |
| `ringDelayToAutoID` | 1500 | 250 | 120 |
| `playerTransferenceRatio` | 20 | 10 | — |

### Combat Effects
| Field | Brogue | Rapid | Bullet |
|-------|--------|-------|--------|
| `onHitHallucinateDuration` | 20 | 20 | 10 |
| `onHitWeakenDuration` | 300 | 100 | 50 |
| `onHitMercyHealPercent` | 15 | 15 | 15 |
| `fallDamageMin/Max` | 8-10 | 8-10 | 8-10 |

## Variant-Specific Catalogs

Each variant defines its own copies of these tables (not in gameConstants, swapped via pointers):

| Catalog | Purpose |
|---------|---------|
| `autoGeneratorCatalog` | Dungeon auto-generation rules |
| `boltCatalog` | Magic bolt/spell definitions |
| `blueprintCatalog` | Vault/machine blueprints |
| `hordeCatalog` | Monster group definitions |
| `potionTable`, `scrollTable`, `wandTable`, `charmTable` | Item definitions |
| `featTable` | Achievement definitions |
| `charmEffectTable` | Charm effect parameters |
| `levelFeelings` | Level atmosphere/flavor text |
| `lumenstoneDistribution` | Post-amulet collectible distribution |
| `itemGenerationProbabilities` | Item category spawn weights |
| `meteredItemsGenerationTable` | Per-level item generation |
| `mainMenuTitle` | ASCII art title screen |

**Shared across all variants** (NOT swappable):
- `monsterCatalog` — all monster definitions
- Dungeon grid dimensions (78×29)
- Core gameplay mechanics (turn system, pathfinding)
- UI framework

## Variant Registration

### Enum (`Rogue.h:199-204`)
```c
enum gameVariant {
    VARIANT_BROGUE,
    VARIANT_RAPID_BROGUE,
    VARIANT_BULLET_BROGUE,
    NUMBER_VARIANTS
};
```

### Global Pointer (`GlobalsBase.c:44-45`)
```c
const gameConstants *gameConst;   // Set during initialization
int gameVariant = VARIANT_BROGUE; // Default
```

### Selection Flow
1. **Menu**: `chooseGameVariant()` in `MainMenu.c:379-415` — 3 buttons
2. **Command line**: `--variant rapid_brogue` or `--variant bullet_brogue` in `main.c:174-185`
3. **Initialization**: `initializeGameVariant()` in `RogueMain.c:173` dispatches to variant init function
4. **Variant init** (e.g., `initializeGameVariantBrogue()` in `GlobalsBrogue.c:1052`):
   - Sets `gameConst = &brogueGameConst`
   - Assigns all catalog pointers

### Usage Throughout Codebase
Accessed via `gameConst->fieldName`:
```c
if (rogue.depthLevel == gameConst->amuletLevel) { ... }
for (i = 0; i < gameConst->deepestLevel; i++) { ... }
```

## How to Add a New Variant

### Step 1: Create Variant Files
- `src/variants/GlobalsMyVariant.h` — declare `void initializeGameVariantMyVariant(void);`
- `src/variants/GlobalsMyVariant.c` — define all catalogs and constants

### Step 2: Define Constants
```c
const gameConstants myVariantGameConst = {
    .majorVersion = 1, .minorVersion = 0, .patchVersion = 0,
    .variantName = "myVariant",
    .deepestLevel = 20,
    .amuletLevel = 15,
    .depthAccelerator = 2,
    // ... all 30 fields
};
```

### Step 3: Define Catalogs
Copy and modify from an existing variant:
- `autoGeneratorCatalog_MyVariant[]`
- `boltCatalog_MyVariant[]`
- `blueprintCatalog_MyVariant[]`
- `hordeCatalog_MyVariant[]`
- `potionTable_MyVariant[]`, `scrollTable_MyVariant[]`, etc.

### Step 4: Implement Init Function
```c
void initializeGameVariantMyVariant(void) {
    gameConst = &myVariantGameConst;
    autoGeneratorCatalog = autoGeneratorCatalog_MyVariant;
    boltCatalog = boltCatalog_MyVariant;
    blueprintCatalog = blueprintCatalog_MyVariant;
    hordeCatalog = hordeCatalog_MyVariant;
    // ... assign all catalog pointers
}
```

### Step 5: Register Variant
1. **Add to enum** (`Rogue.h`): `VARIANT_MY_VARIANT` before `NUMBER_VARIANTS`
2. **Add to switch** (`RogueMain.c:173`):
   ```c
   case VARIANT_MY_VARIANT: initializeGameVariantMyVariant(); break;
   ```
3. **Add menu button** (`MainMenu.c:379-415`)
4. **Add CLI arg** (`main.c:174-185`): parse `--variant my_variant`
5. **Include header** in MainMenu.c and RogueMain.c

### Step 6: Build
Add source file to Makefile and rebuild.

## Key Differences Summary

| Aspect | Brogue | Rapid | Bullet |
|--------|--------|-------|--------|
| Depth | 40 levels | 10 levels | 5 levels |
| Amulet | Level 26 | Level 7 | Level 3 |
| Pacing | 1× | 4× | 8× |
| Items/level | +0 bonus | +4 bonus | +8 bonus |
| Auto-ID speed | Slow (20 kills) | Fast (5 kills) | Fastest (2 kills) |
| Machines | Sparse | Dense | Maximum |

## Key Files

| File | Purpose |
|------|---------|
| `Rogue.h:2389-2452` | `gameConstants` struct definition |
| `Rogue.h:199-204` | `gameVariant` enum |
| `GlobalsBase.c:44-45` | `gameConst` and `gameVariant` globals |
| `RogueMain.c:173-185` | `initializeGameVariant()` dispatch |
| `GlobalsBrogue.c:990-1078` | Brogue constants + init |
| `GlobalsRapidBrogue.c:994-1073` | Rapid Brogue constants + init |
| `GlobalsBulletBrogue.c:1005-1084` | Bullet Brogue constants + init |
| `MainMenu.c:379-415` | Variant selection UI |
| `main.c:174-185` | CLI `--variant` parsing |
