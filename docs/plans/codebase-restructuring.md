# Plan: Codebase Restructuring for SoC and AI Navigability

## Goal
Split the 3 largest files (Items.c, IO.c, Rogue.h) along natural domain boundaries and reduce variant duplication. Each task extracts one file, runs tests, and commits — so breakage is caught immediately and rollback is trivial.

## Context
- Build system uses `$(wildcard src/brogue/*.c)` — new .c files are auto-discovered, no Makefile changes needed
- `Rogue.h` is included by every .c file as an umbrella header — splitting it into sub-headers that Rogue.h re-includes is a zero-risk operation
- All function declarations already live in Rogue.h organized by source file — they just need to be re-pointed when functions move
- `make test` runs a C test harness (headless, no SDL) that covers all game systems
- No function signatures change. No runtime behavior changes. Pure file reorganization.

## Acceptance Criteria
- [ ] `make test` passes after every task
- [ ] `make bin/brogue` builds successfully after every task
- [ ] No function signatures changed
- [ ] No runtime behavior changes
- [ ] Each original file still exists (just smaller)
- [ ] Total line count across the project is unchanged (± comment/include boilerplate)

## Tasks

### Phase 1: Split Items.c (8,394 lines → 5 files)

- [ ] **Task 1: Extract `src/brogue/Bolts.c`** (~2,200 lines)
  Move the bolt/projectile system out of Items.c. Functions to extract:
  `getLineCoordinates`, `getImpactLoc`, `impermissibleKinkBetween`, `tunnelize`, `negationWillAffectMonster`, `negate`, `weaken`, `polymorph`, `slow`, `haste`, `heal`, `makePlayerTelepathic`, `rechargeItems`, `negationBlast`, `discordBlast`, `crystalize`, `imbueInvisibility`, `projectileReflects`, `reflectBolt`, `checkForMissingKeys`, `beckonMonster`, `boltEffectForItem`, `boltForItem`, `updateBolt`, `detonateBolt`, `zap`, `itemMagicPolarityIsKnown`, `canAutoTargetMonster`, `nextTargetAfter`, `hiliteTrajectory`, `moveCursor`, `pullMouseClickDuringPlayback`, `chooseTarget`.
  New file includes `Rogue.h`, `GlobalsBase.h`, `Globals.h`. No changes to Rogue.h function declarations needed — they stay in Rogue.h.
  Verify: `make -B && make test`

- [ ] **Task 2: Extract `src/brogue/ItemDisplay.c`** (~1,900 lines)
  Move item naming, descriptions, and inventory UI. Functions to extract:
  `inscribeItem`, `itemCanBeCalled`, `call`, `itemName`, `itemKindName`, `itemRunicName`, `enchantMagnitude`, `tableForItemCategory`, `isVowelish`, `enchantIncrement`, `itemIsCarried`, `effectiveRingEnchant`, `apparentRingBonus`, `monsterClassHasAcidicMonster`, `itemDetails`, `displayMagicCharForItem`, `displayInventory`, `numberOfMatchingPackItems`, `describedItemName`, `describeHallucinatedItem`, `magicCharDiscoverySuffix`.
  Verify: `make -B && make test`

- [ ] **Task 3: Extract `src/brogue/ItemUsage.c`** (~1,500 lines)
  Move item usage commands (throw, eat, drink, read, apply, fire). Functions to extract:
  `hitMonsterWithProjectileWeapon`, `throwItem`, `throwCommand`, `relabel`, `swapLastEquipment`, `playerCancelsBlinking`, `recordApplyItemCommand`, `useStaffOrWand`, `summonGuardian`, `consumePackItem`, `eat`, `useCharm`, `rangedWeaponBaseCooldown`, `rangedWeaponCooldownMax`, `rangedWeaponRange`, `fireRangedWeapon`, `apply`, `identify`, `lotteryDraw`, `chooseVorpalEnemy`, `describeMonsterClass`, `updateIdentifiableItem`, `updateIdentifiableItems`, `magicMapCell`, `uncurse`, `readScroll`, `detectMagicOnItem`, `drinkPotion`.
  Verify: `make -B && make test`

- [ ] **Task 4: Extract `src/brogue/ItemEquip.c`** (~600 lines)
  Move equipment and identification logic. Functions to extract:
  `armorValueIfUnenchanted`, `displayedArmorValue`, `strengthCheck`, `equip`, `keyMatchesLocation`, `keyInPackFor`, `keyOnTileAt`, `aggravateMonsters`, `itemMagicPolarity`, `unequip`, `canDrop`, `drop`, `promptForItemOfType`, `itemOfPackLetter`, `itemAtLoc`, `dropItem`, `recalculateEquipmentBonuses`, `equipItem`, `unequipItem`, `updateRingBonuses`, `updatePlayerRegenerationDelay`, `itemKindCount`, `tryGetLastUnidentifiedItemKind`, `magicPolarityRevealedItemKindCount`, `tryIdentifyLastItemKind`, `tryIdentifyLastItemKinds`, `identifyItemKind`, `autoIdentify`, `checkForDisenchantment`, `itemIsSwappable`, `swapItemToEnchantLevel`, `enchantLevelKnown`, `effectiveEnchantLevel`, `swapItemEnchants`.
  Verify: `make -B && make test`

  After this phase, Items.c retains: `initializeItem`, `generateItem`, `pickItemCategory`, `makeItemInto`, `chooseKind`, `getItemCategoryGlyph`, item predicates, `placeItemAt`, `fillItemSpawnHeatMap`, `coolHeatMapAt`, `getItemSpawnLoc`, `populateItems`, `itemWillStackWithPack`, `removeItemAt`, `pickUpItemAt`, `conflateItemCharacteristics`, `stackItems`, `inventoryLetterAvailable`, `addItemToPack`, `numberOfItemsInPack`, `nextAvailableInventoryCharacter`, `updateFloorItems`, `removeItemFromChain`, `addItemToChain`, `deleteItem`, `resetItemTableEntry`, `shuffleFlavors`, `itemValue` (~1,200 lines).

### Phase 2: Split IO.c (5,175 lines → 3 files)

- [ ] **Task 5: Extract `src/brogue/Messages.c`** (~1,200 lines)
  Move the message system. Functions to extract:
  `clearMessageArchive`, `getArchivedMessage`, `dequeueEvent`, `formatCountedMessage`, `foldMessages`, `splitLines`, `capitalizeAndPunctuateSentences`, `formatRecentMessages`, `displayRecentMessages`, `drawMessageArchive`, `animateMessageArchive`, `scrollMessageArchive`, `displayMessageArchive`, `temporaryMessage`, `flavorMessage`, `messageWithColor`, `message`, `displayMoreSignWithoutWaitingForAcknowledgment`, `displayMoreSign`, `updateMessageDisplay`, `deleteMessages`, `confirmMessages`, `encodeMessageColor`, `decodeMessageColor`, `messageColorFromVictim`, `flashMessage`, `combatMessage`, `displayCombatText`.
  Verify: `make -B && make test`

- [ ] **Task 6: Extract `src/brogue/Render.c`** (~2,200 lines)
  Move cell appearance, color math, sidebar, and text rendering. Functions to extract:
  `getCellAppearance`, `refreshDungeonCell`, `glyphIsWallish`, `randomAnimateMonster`, color algebra (`bakeColor`, `bakeTerrainColors`, `storeColorComponents`, `shuffleTerrainColors`, `normColor`, `separateColors`, `applyColorMultiplier`, `applyColorAverage`, `applyColorAugment`, `applyColorScalar`, `applyColorBounds`, `desaturate`, `randomizeColor`, `swapColors`, `colorMultiplierFromDungeonLight`, `colorBlendCell`, `hiliteCell`, `colorFromComponents`), `plotCharWithColor`, `plotCharToBuffer`, `plotForegroundChar`, `dumpLevelToScreen`, `copyDisplayBuffer`, `clearDisplayBuffer`, `saveDisplayBuffer`, `restoreDisplayBuffer`, `overlayDisplayBuffer`, `flashForeground`, `flashCell`, `colorFlash`, text rendering (`printString`, `wrapText`, `printStringWithWrapping`, `breakUpLongWordsIn`, `upperCase`), sidebar (`refreshSideBar`, `printMonsterInfo`, `printItemInfo`, `printTerrainInfo`, `printMonsterDetails`, `printFloorItemDetails`, `printCarriedItemDetails`), screens (`printHelpScreen`, `printDiscoveriesScreen`, `displayFeatsScreen`, `printHighScores`, `displayGrid`, `printSeed`, `printProgressBar`, `highlightScreenCell`), display utilities (`rectangularShading`, `printTextBox`, `funkyFade`, `displayCenteredAlert`, `flashTemporaryAlert`, `waitForAcknowledgment`, `waitForKeystrokeOrMouseClick`, `confirm`, `hiliteCharGrid`, `blackOutScreen`, `colorOverDungeon`, `irisFadeBetweenBuffers`, `flashMonster`).
  Verify: `make -B && make test`

  After this phase, IO.c retains: `mainInputLoop`, `executeKeystroke`, `executeMouseClick`, `nextBrogueEvent`, `commitDraws`, `refreshScreen`, `displayLevel`, cursor management (`getPlayerPathOnMap`, `reversePath`, `hilitePath`, `clearCursorPath`, `hideCursor`, `showCursor`, `processSnapMap`, `getClosestValidLocationOnMap`), `actionMenu`, `initializeMenuButtons`, `considerCautiousMode`, `exploreKey`, wizard displays, `getInputTextString`, `pauseBrogue`, `pauseAnimation`, `nextKeyPress`, `displayMonsterFlashes`, `translateModernKeys`, `stripShiftFromMovementKeystroke`, `setButtonText` (~1,800 lines).

### Phase 3: Split Rogue.h (3,637 lines → umbrella + 4 sub-headers)

- [ ] **Task 7: Extract `src/brogue/RogueBase.h`** (~400 lines)
  Extract foundational types that everything needs: includes/guards, version macros, debug flags, utility macros (`boolean`, `fixpt`), dimension constants (`COLS`, `ROWS`, `DCOLS`, `DROWS`), `typedef struct pos`, `typedef struct windowpos`, `typedef struct color`, `typedef struct randomRange`, `enum RNGs`, `enum directions`, `enum displayGlyph`, `enum graphicsModes`, `enum eventTypes`, `enum textEntryTypes`, `enum displayDetailValues`, `struct rogueEvent`. Rogue.h gets `#include "RogueBase.h"` at the top. All .c files continue to include Rogue.h unchanged.
  Verify: `make -B && make test`

- [ ] **Task 8: Extract `src/brogue/ItemTypes.h` and `src/brogue/CreatureTypes.h`** (~1,050 lines total)
  `ItemTypes.h`: item category enum, all item kind enums (weapon/armor/potion/scroll/staff/wand/ring/charm/key/food/ranged), enchant enums, `enum boltType`, `enum boltEffects`, `enum boltFlags`, `struct bolt`, `struct itemTable`, `struct item`, `struct meteredItem`, `struct charmEffectTableEntry`, `struct meteredItemGenerationTable`, `struct levelFeeling`, `struct keyLocationProfile`.
  `CreatureTypes.h` (includes ItemTypes.h): `enum monsterTypes`, all monster flag enums (behavior/ability/bookkeeping), `enum hordeFlags`, `enum creatureStates`, `enum creatureModes`, `enum statusEffects`, `struct mutation`, `struct hordeType`, `struct monsterClass`, `struct creatureType`, `struct creature`, creature list types.
  Both included by Rogue.h. RogueBase.h included by both.
  Verify: `make -B && make test`

- [ ] **Task 9: Extract `src/brogue/TerrainTypes.h` and `src/brogue/DungeonTypes.h`** (~800 lines total)
  `TerrainTypes.h` (includes RogueBase.h): `enum tileType`, `enum lightType`, `enum dungeonFeatureTypes`, `enum dungeonLayers`, terrain flag enums, `struct pcell`, `struct tcell`, `struct floorTileType`, `struct dungeonFeature`, `struct lightSource`, `struct flare`, `struct dungeonProfile`, `struct cellDisplayBuffer`, `struct screenDisplayBuffer`.
  `DungeonTypes.h` (includes TerrainTypes.h, CreatureTypes.h): `enum machineFeatureFlags`, `enum blueprintFlags`, `enum machineTypes`, `struct machineFeature`, `struct blueprint`, `struct autoGenerator`, `struct levelData`, `struct gameConstants`, `struct playerCharacter`, `enum NGCommands`, `enum featTypes`, `enum gameMode`, `struct feat`, `struct brogueButton`, `struct buttonState`, `struct archivedMessage`, button/message enums.
  Both included by Rogue.h.
  Verify: `make -B && make test`

### Phase 4: Reduce Variant Duplication

- [ ] **Task 10: Consolidate shared variant table structures**
  The item tables (`potionTable`, `scrollTable`, `wandTable`, `charmTable`, `rangedWeaponTable`), `featTable`, and color definitions that are structurally identical across all 3 variant files should be consolidated. Approach: move shared-structure tables to `Globals.c` as defaults, have variant init functions only override the numeric values that differ. This removes ~2,000 lines of duplication across the 3 variant files. Each variant's `initializeGameVariant*()` function already assigns pointers — extend this to copy-then-patch for tables with minimal per-variant differences.
  Verify: `make -B && make test`, then run seed catalog determinism tests for all 3 variants:
  ```
  python3 test/compare_seed_catalog.py test/seed_catalogs/seed_catalog_brogue.txt 40
  python3 test/compare_seed_catalog.py --extra_args "--variant rapid_brogue" test/seed_catalogs/seed_catalog_rapid_brogue.txt 10
  python3 test/compare_seed_catalog.py --extra_args "--variant bullet_brogue" test/seed_catalogs/seed_catalog_bullet_brogue.txt 5
  ```

## Execution Notes

- Each task: create new file → move functions (cut from old, paste to new) → add `#include` boilerplate → `make -B && make test` → commit
- Function declarations stay in Rogue.h throughout — they don't need to move when .c files split
- Static/private helper functions that are only called by functions in the extracted group move with them. Static helpers shared across groups must be made non-static (add declaration to Rogue.h) or duplicated
- Order matters: Items.c tasks can be done in any order within Phase 1, but Phase 2 depends on Phase 1 being complete (some functions reference both)
- The header split (Phase 3) is purely additive — Rogue.h becomes a thin wrapper that includes sub-headers
- For header splits: use `#pragma once` or standard include guards (`#ifndef ROGUBASE_H / #define ROGUEBASE_H`)
- When moving static functions: grep for all callers first to determine if the function should become public or move with its callers

## Ask User
(empty)

## Critic Findings
(empty)
