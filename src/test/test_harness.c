// test_harness.c — Game helper implementations for BrogueCE test harness

#include "test_harness.h"
#include "platform.h"

test_state _test_state = {0, 0, 0};

// Globals normally defined in src/platform/main.c (which we exclude since it has its own main())
struct brogueConsole currentConsole;
char dataDirectory[BROGUE_FILENAME_MAX] = ".";
boolean serverMode = false;
boolean nonInteractivePlayback = false;
boolean hasGraphics = false;
enum graphicsModes graphicsMode = TEXT_GRAPHICS;
boolean isCsvFormat = false;

boolean tryParseUint64(char *str, uint64_t *num) {
    unsigned long long n;
    char buf[100];
    if (strlen(str) && sscanf(str, "%llu", &n) && sprintf(buf, "%llu", n) && !strcmp(buf, str)) {
        *num = (uint64_t)n;
        return true;
    }
    return false;
}

extern struct brogueConsole nullConsole;

void test_init_game(uint64_t seed) {
    // Clear recording path so initRecording() is a no-op
    currentFilePath[0] = '\0';

    // Use null platform (no rendering, no input)
    currentConsole = nullConsole;

    // Initialize game variant (sets gameConst)
    gameVariant = VARIANT_BROGUE;
    initializeGameVariant();

    // Initialize the game
    initializeRogue(seed);

    // Skip all confirmation dialogs
    rogue.autoPlayingLevel = true;

    // Generate and enter the first dungeon level
    startLevel(rogue.depthLevel, 1);
}

void test_teardown_game(void) {
    rogue.gameHasEnded = true;
    freeEverything();
}

void test_send_key(signed long key, boolean ctrl, boolean shift) {
    rogueEvent event;
    memset(&event, 0, sizeof(event));
    event.eventType = KEYSTROKE;
    event.param1 = key;
    event.controlKey = ctrl;
    event.shiftKey = shift;
    executeEvent(&event);
}

static const signed long directionKeys[8] = {
    UP_KEY, DOWN_KEY, LEFT_KEY, RIGHT_KEY,
    UPLEFT_KEY, DOWNLEFT_KEY, UPRIGHT_KEY, DOWNRIGHT_KEY
};

void test_move(short direction) {
    if (direction < 0 || direction >= DIRECTION_COUNT) return;
    test_send_key(directionKeys[direction], false, false);
}

void test_rest(void) {
    test_send_key(REST_KEY, false, false);
}

void test_rest_turns(int n) {
    for (int i = 0; i < n && !rogue.gameHasEnded; i++) {
        test_rest();
    }
}

creature *test_monster_at(short x, short y) {
    return monsterAtLoc((pos){ x, y });
}

int test_count_monsters(void) {
    int count = 0;
    creatureIterator iter = iterateCreatures(monsters);
    while (hasNextCreature(iter)) {
        nextCreature(&iter);
        count++;
    }
    return count;
}

boolean test_cell_has_terrain_flag(short x, short y, unsigned long flag) {
    return cellHasTerrainFlag((pos){ x, y }, flag);
}

creature *test_place_monster(short monsterID, short x, short y) {
    // Generate the monster (adds it to the monsters list)
    creature *monst = generateMonster(monsterID, false, false);
    if (!monst) return NULL;

    // Place it at the specified location
    monst->loc.x = x;
    monst->loc.y = y;
    pmap[x][y].flags |= HAS_MONSTER;

    return monst;
}

void test_reseed(uint64_t seed) {
    seedRandomGenerator(seed);
}

void test_init_arena(uint64_t seed) {
    // Clear recording path so initRecording() is a no-op
    currentFilePath[0] = '\0';

    // Use null platform (no rendering, no input)
    currentConsole = nullConsole;

    // Initialize game variant (sets gameConst)
    gameVariant = VARIANT_BROGUE;
    initializeGameVariant();

    // Initialize the game (seeds RNG, creates player, allocates grids)
    initializeRogue(seed);

    // Skip all confirmation dialogs
    rogue.autoPlayingLevel = true;

    // Allocate scent map (required by monster AI / pathfinding)
    levels[0].scentMap = allocGrid();
    scentMap = levels[0].scentMap;
    fillGrid(scentMap, 0);
    levels[0].visited = true;

    // Set up miner's light radius
    rogue.minersLightRadius = (DCOLS - 1) * FP_FACTOR;
    rogue.minersLightRadius += FP_FACTOR * 225 / 100;

    // Instead of startLevel/dungeon generation, build a blank floor map.
    // This avoids consuming RNG during dungeon generation, making the test
    // fully independent of item/monster table changes.
    for (short x = 0; x < DCOLS; x++) {
        for (short y = 0; y < DROWS; y++) {
            pmap[x][y].layers[DUNGEON] = (x == 0 || x == DCOLS-1 || y == 0 || y == DROWS-1) ? GRANITE : FLOOR;
            pmap[x][y].layers[LIQUID] = NOTHING;
            pmap[x][y].layers[GAS] = NOTHING;
            pmap[x][y].layers[SURFACE] = NOTHING;
            pmap[x][y].flags = 0;
            pmap[x][y].volume = 0;
            pmap[x][y].machineNumber = 0;
        }
    }

    // Place the player at the center
    player.loc.x = DCOLS / 2;
    player.loc.y = DROWS / 2;
    pmap[player.loc.x][player.loc.y].flags |= HAS_PLAYER;

    // Seed the RNG to the requested seed so gameplay is deterministic
    // and independent of whatever initializeRogue consumed
    seedRandomGenerator(seed);

    updateVision(true);

    // Mark all non-wall cells as discovered so tests don't depend on lighting
    for (short x = 0; x < DCOLS; x++) {
        for (short y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[DUNGEON] != GRANITE) {
                pmap[x][y].flags |= DISCOVERED | VISIBLE | IN_FIELD_OF_VIEW;
            }
        }
    }
}

void test_teleport_player(short x, short y) {
    // Clear old position
    pmap[player.loc.x][player.loc.y].flags &= ~HAS_PLAYER;

    // Set new position
    player.loc.x = x;
    player.loc.y = y;
    pmap[x][y].flags |= HAS_PLAYER;

    // Update vision from new position
    updateVision(true);
}

item *test_give_item(unsigned short category, short kind, short enchant) {
    item *theItem = generateItem(category, kind);
    if (!theItem) return NULL;

    theItem->enchant1 = enchant;
    theItem->flags |= ITEM_IDENTIFIED;

    // Recompute derived fields for ranged weapons after enchant override
    if (category == RANGED) {
        theItem->cooldownMax = rangedWeaponCooldownMax(theItem);
        theItem->maxRange = rangedWeaponRange(theItem);
    }

    // Add to player's pack
    addItemToChain(theItem, packItems);

    return theItem;
}

void test_set_player_hp(short hp, short maxHp) {
    if (maxHp > 0) {
        player.info.maxHP = maxHp;
    }
    player.currentHP = hp;
}

void test_set_player_status(short statusType, short value) {
    player.status[statusType] = value;
    player.maxStatus[statusType] = value;
}

boolean test_find_open_floor(short nearX, short nearY, pos *out) {
    // Search in expanding rings around the target position
    for (short radius = 0; radius < 20; radius++) {
        for (short dx = -radius; dx <= radius; dx++) {
            for (short dy = -radius; dy <= radius; dy++) {
                if (abs(dx) != radius && abs(dy) != radius) continue; // only ring edges
                short x = nearX + dx;
                short y = nearY + dy;
                if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;
                if (!(pmap[x][y].flags & (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS))
                    && !cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)
                    && !cellHasTerrainFlag((pos){ x, y }, T_PATHING_BLOCKER)) {
                    out->x = x;
                    out->y = y;
                    return true;
                }
            }
        }
    }
    return false;
}

void test_clear_area(short centerX, short centerY, short radius) {
    for (short x = centerX - radius; x <= centerX + radius; x++) {
        for (short y = centerY - radius; y <= centerY + radius; y++) {
            if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;

            // Kill any monster here
            creature *monst = monsterAtLoc((pos){ x, y });
            if (monst) {
                killCreature(monst, true);
            }

            // Remove any floor items here
            for (item *theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
                if (theItem->loc.x == x && theItem->loc.y == y) {
                    removeItemFromChain(theItem, floorItems);
                    deleteItem(theItem);
                    break; // restart would be safer but one item per cell is typical
                }
            }

            // Set terrain to open floor
            pmap[x][y].layers[DUNGEON] = FLOOR;
            pmap[x][y].layers[LIQUID] = NOTHING;
            pmap[x][y].layers[GAS] = NOTHING;
            pmap[x][y].layers[SURFACE] = NOTHING;
            pmap[x][y].volume = 0;
            pmap[x][y].flags &= (HAS_PLAYER | HAS_STAIRS); // preserve only player/stairs
            pmap[x][y].machineNumber = 0;
        }
    }
    updateVision(true);
}

void test_remove_all_monsters(void) {
    creatureIterator iter = iterateCreatures(monsters);
    while (hasNextCreature(iter)) {
        creature *monst = nextCreature(&iter);
        killCreature(monst, true);
    }
    // Also clear dormant monsters
    creatureIterator dIter = iterateCreatures(dormantMonsters);
    while (hasNextCreature(dIter)) {
        creature *monst = nextCreature(&dIter);
        killCreature(monst, true);
    }
}

void test_set_terrain(short x, short y, enum dungeonLayers layer, enum tileType terrain) {
    if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) return;
    if (layer < 0 || layer >= NUMBER_TERRAIN_LAYERS) return;
    pmap[x][y].layers[layer] = terrain;
}

void test_advance_environment(int ticks) {
    for (int i = 0; i < ticks; i++) {
        updateEnvironment();
    }
}
