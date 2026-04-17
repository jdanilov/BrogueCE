// test_trap_hunting.c — Tests for Wand of Trapping and trap items

#include "test_harness.h"

// Helper: place a revealed trap tile on a cell
static void place_revealed_trap(short x, short y, enum tileType trapTile) {
    pmap[x][y].layers[DUNGEON] = trapTile;
    pmap[x][y].flags &= ~(PRESSURE_PLATE_DEPRESSED | KNOWN_TO_BE_TRAP_FREE);
}

// Helper: place a hidden (secret) trap tile on a cell
static void place_hidden_trap(short x, short y, enum tileType hiddenTrapTile) {
    pmap[x][y].layers[DUNGEON] = hiddenTrapTile;
    pmap[x][y].flags &= ~(PRESSURE_PLATE_DEPRESSED | KNOWN_TO_BE_TRAP_FREE);
}

// Helper: count floor items of a given category/kind at position
static int count_floor_items_at(short x, short y, unsigned short category, short kind) {
    int count = 0;
    for (item *it = floorItems->nextItem; it != NULL; it = it->nextItem) {
        if (it->loc.x == x && it->loc.y == y
            && it->category == category && it->kind == kind) {
            count++;
        }
    }
    return count;
}

// --- Wand extraction tests ---

TEST(test_wand_trapping_extracts_poison_gas_trap) {
    test_init_arena(42);

    // Give player a wand of trapping
    item *wand = test_give_item(WAND, WAND_TRAPPING, 0);
    ASSERT(wand != NULL);
    ASSERT_EQ(wand->category, WAND);
    ASSERT_EQ(wand->kind, WAND_TRAPPING);
    ASSERT_GT(wand->charges, 0);
    short initialCharges = wand->charges;

    // Place a revealed poison gas trap adjacent to player
    short trapX = player.loc.x + 1;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, GAS_TRAP_POISON);

    // Verify trap is there
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], GAS_TRAP_POISON);
    ASSERT(tileCatalog[GAS_TRAP_POISON].flags & T_IS_DF_TRAP);
    ASSERT(!(tileCatalog[GAS_TRAP_POISON].mechFlags & TM_IS_SECRET));

    // Zap the wand at the trap using the bolt system directly
    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    // Trap should be extracted
    ASSERT(autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], FLOOR);

    // Trap item should be on the floor
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_POISON_GAS), 1);

    test_teardown_game();
}

TEST(test_wand_trapping_extracts_confusion_gas_trap) {
    test_init_arena(43);

    short trapX = player.loc.x + 1;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, GAS_TRAP_CONFUSION);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    ASSERT(autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], FLOOR);
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_CONFUSION_GAS), 1);

    test_teardown_game();
}

TEST(test_wand_trapping_extracts_net_trap) {
    test_init_arena(44);

    short trapX = player.loc.x + 1;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, NET_TRAP);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    ASSERT(autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], FLOOR);
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_NET), 1);

    test_teardown_game();
}

TEST(test_wand_trapping_extracts_alarm_trap) {
    test_init_arena(45);

    short trapX = player.loc.x + 1;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, ALARM_TRAP);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    ASSERT(autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], FLOOR);
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_ALARM), 1);

    test_teardown_game();
}

TEST(test_wand_trapping_extracts_flamethrower_trap) {
    test_init_arena(46);

    short trapX = player.loc.x + 1;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, FLAMETHROWER);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    ASSERT(autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], FLOOR);
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_FLAMETHROWER), 1);

    test_teardown_game();
}

TEST(test_wand_trapping_extracts_paralysis_trap) {
    test_init_arena(47);

    short trapX = player.loc.x + 1;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, GAS_TRAP_PARALYSIS);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    ASSERT(autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], FLOOR);
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_PARALYSIS_GAS), 1);

    test_teardown_game();
}

// --- Fizzle tests ---

TEST(test_wand_trapping_fizzles_on_floor) {
    test_init_arena(50);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Ensure it's just floor
    ASSERT_EQ(pmap[targetX][targetY].layers[DUNGEON], FLOOR);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ targetX, targetY }, &theBolt, false, false);

    // Should fizzle
    ASSERT(!autoID);
    ASSERT_EQ(pmap[targetX][targetY].layers[DUNGEON], FLOOR);

    // No trap item on floor
    ASSERT_EQ(count_floor_items_at(targetX, targetY, TRAP_ITEM, TRAP_POISON_GAS), 0);

    test_teardown_game();
}

TEST(test_wand_trapping_fizzles_on_hidden_trap) {
    test_init_arena(51);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Place a hidden (secret) trap — should NOT be extractable
    place_hidden_trap(targetX, targetY, GAS_TRAP_POISON_HIDDEN);
    ASSERT(tileCatalog[GAS_TRAP_POISON_HIDDEN].mechFlags & TM_IS_SECRET);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ targetX, targetY }, &theBolt, false, false);

    // Should fizzle since trap is hidden
    ASSERT(!autoID);
    ASSERT_EQ(pmap[targetX][targetY].layers[DUNGEON], GAS_TRAP_POISON_HIDDEN);

    test_teardown_game();
}

TEST(test_wand_trapping_fizzles_beyond_range_1) {
    test_init_arena(52);

    // Place a revealed trap 3 cells away — beyond extraction range
    short trapX = player.loc.x + 3;
    short trapY = player.loc.y;
    place_revealed_trap(trapX, trapY, GAS_TRAP_POISON);

    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], GAS_TRAP_POISON);

    bolt theBolt = boltCatalog[BOLT_TRAPPING];
    boolean autoID = zap(player.loc, (pos){ trapX, trapY }, &theBolt, false, false);

    // Should fizzle — trap is out of range even though it's revealed
    ASSERT(!autoID);
    ASSERT_EQ(pmap[trapX][trapY].layers[DUNGEON], GAS_TRAP_POISON);
    ASSERT_EQ(count_floor_items_at(trapX, trapY, TRAP_ITEM, TRAP_POISON_GAS), 0);

    test_teardown_game();
}

// --- Trap placement tests ---

TEST(test_trap_item_placement_sets_hidden_tile) {
    test_init_arena(60);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Place a poison gas trap tile directly (simulating useTrapItem)
    pmap[targetX][targetY].layers[DUNGEON] = GAS_TRAP_POISON_HIDDEN;
    pmap[targetX][targetY].flags |= HAS_PLAYER_PLACED_TRAP;
    pmap[targetX][targetY].flags &= ~(PRESSURE_PLATE_DEPRESSED | KNOWN_TO_BE_TRAP_FREE);

    // Verify the trap is there and hidden
    ASSERT_EQ(pmap[targetX][targetY].layers[DUNGEON], GAS_TRAP_POISON_HIDDEN);
    ASSERT(tileCatalog[GAS_TRAP_POISON_HIDDEN].flags & T_IS_DF_TRAP);
    ASSERT(tileCatalog[GAS_TRAP_POISON_HIDDEN].mechFlags & TM_IS_SECRET);
    ASSERT(pmap[targetX][targetY].flags & HAS_PLAYER_PLACED_TRAP);

    test_teardown_game();
}

TEST(test_trap_item_placement_paralysis_uses_standalone_tile) {
    test_init_arena(61);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Simulate placing paralysis trap
    pmap[targetX][targetY].layers[DUNGEON] = PLAYER_PARALYSIS_TRAP_HIDDEN;
    pmap[targetX][targetY].flags |= HAS_PLAYER_PLACED_TRAP;

    // Verify it's a standalone trap (not wired)
    ASSERT(tileCatalog[PLAYER_PARALYSIS_TRAP_HIDDEN].flags & T_IS_DF_TRAP);
    ASSERT(tileCatalog[PLAYER_PARALYSIS_TRAP_HIDDEN].mechFlags & TM_IS_SECRET);
    ASSERT(!(tileCatalog[PLAYER_PARALYSIS_TRAP_HIDDEN].mechFlags & TM_IS_WIRED));

    // The fireType should spawn paralysis gas
    ASSERT_EQ(tileCatalog[PLAYER_PARALYSIS_TRAP_HIDDEN].fireType, DF_PARALYSIS_GAS_TRAP_CLOUD);

    test_teardown_game();
}

TEST(test_trap_item_placement_fire_uses_burst_tile) {
    test_init_arena(62);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Simulate placing fire trap
    pmap[targetX][targetY].layers[DUNGEON] = PLAYER_FIRE_TRAP_HIDDEN;
    pmap[targetX][targetY].flags |= HAS_PLAYER_PLACED_TRAP;

    // Verify it's a burst trap (not directional)
    ASSERT(tileCatalog[PLAYER_FIRE_TRAP_HIDDEN].flags & T_IS_DF_TRAP);
    ASSERT(tileCatalog[PLAYER_FIRE_TRAP_HIDDEN].mechFlags & TM_IS_SECRET);
    ASSERT_EQ(tileCatalog[PLAYER_FIRE_TRAP_HIDDEN].fireType, DF_FIRE_TRAP_BURST);

    test_teardown_game();
}

// --- Trap trigger tests ---

TEST(test_placed_trap_triggers_on_monster) {
    test_init_arena(70);

    short trapX = player.loc.x + 3;
    short trapY = player.loc.y;

    // Place a hidden net trap (player-placed)
    pmap[trapX][trapY].layers[DUNGEON] = NET_TRAP_HIDDEN;
    pmap[trapX][trapY].flags |= HAS_PLAYER_PLACED_TRAP;
    pmap[trapX][trapY].flags &= ~(PRESSURE_PLATE_DEPRESSED | KNOWN_TO_BE_TRAP_FREE);

    // Place a monster directly ON the trap
    creature *rat = test_place_monster(MK_RAT, trapX, trapY);
    ASSERT(rat != NULL);

    // Trigger trap effects
    applyInstantTileEffectsToCreature(rat);

    // Pressure plate should have fired
    ASSERT(pmap[trapX][trapY].flags & PRESSURE_PLATE_DEPRESSED);

    // Net trap spawns NETTING on the surface layer
    ASSERT_EQ(pmap[trapX][trapY].layers[SURFACE], NETTING);

    // Player-placed flag should be cleared after trigger
    ASSERT(!(pmap[trapX][trapY].flags & HAS_PLAYER_PLACED_TRAP));

    test_teardown_game();
}

// --- Monster AI tests ---

TEST(test_monsters_dont_avoid_player_placed_traps) {
    test_init_arena(80);

    short trapX = player.loc.x + 3;
    short trapY = player.loc.y;

    // Place a player trap
    pmap[trapX][trapY].layers[DUNGEON] = GAS_TRAP_POISON_HIDDEN;
    pmap[trapX][trapY].flags |= HAS_PLAYER_PLACED_TRAP;
    pmap[trapX][trapY].flags &= ~PRESSURE_PLATE_DEPRESSED;

    // Place a hostile monster that is actively tracking the player
    creature *rat = test_place_monster(MK_RAT, trapX - 1, trapY);
    ASSERT(rat != NULL);
    rat->creatureState = MONSTER_TRACKING_SCENT;

    // Tracking monster should NOT avoid a hidden (TM_IS_SECRET) trap cell
    ASSERT(!monsterAvoids(rat, (pos){ trapX, trapY }));

    // For comparison, a revealed (non-secret) trap IS avoided even by wandering monsters
    pmap[trapX][trapY].layers[DUNGEON] = GAS_TRAP_POISON;
    rat->creatureState = MONSTER_WANDERING;
    ASSERT(monsterAvoids(rat, (pos){ trapX, trapY }));

    test_teardown_game();
}

// --- Trap apply tests ---

TEST(test_trap_item_apply_places_and_consumes) {
    test_init_arena(81);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Give player a trap item
    item *trapItem = test_give_item(TRAP_ITEM, TRAP_NET, 0);
    ASSERT(trapItem != NULL);
    ASSERT_EQ(trapItem->category, TRAP_ITEM);
    ASSERT_EQ(trapItem->kind, TRAP_NET);

    // Verify target cell is clear floor
    ASSERT_EQ(pmap[targetX][targetY].layers[DUNGEON], FLOOR);

    // Directly set up and place the trap (simulating useTrapItem core logic)
    enum tileType trapTile = NET_TRAP_HIDDEN;
    pmap[targetX][targetY].layers[DUNGEON] = trapTile;
    pmap[targetX][targetY].flags |= HAS_PLAYER_PLACED_TRAP;
    pmap[targetX][targetY].flags &= ~(PRESSURE_PLATE_DEPRESSED | KNOWN_TO_BE_TRAP_FREE);

    // Remove the item from pack (simulating consumption)
    removeItemFromChain(trapItem, packItems);
    deleteItem(trapItem);

    // Verify trap was placed
    ASSERT_EQ(pmap[targetX][targetY].layers[DUNGEON], NET_TRAP_HIDDEN);
    ASSERT(pmap[targetX][targetY].flags & HAS_PLAYER_PLACED_TRAP);

    // Verify item was consumed — no trap items in pack
    int trapCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        if (it->category == TRAP_ITEM) trapCount++;
    }
    ASSERT_EQ(trapCount, 0);

    test_teardown_game();
}

TEST(test_trap_item_placement_rejects_existing_trap) {
    test_init_arena(82);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Place an existing trap on the target cell
    pmap[targetX][targetY].layers[DUNGEON] = GAS_TRAP_POISON;

    // The cell has T_IS_DF_TRAP, so placement should be rejected
    enum tileType dungeonTile = pmap[targetX][targetY].layers[DUNGEON];
    ASSERT(dungeonTile != FLOOR && dungeonTile != FLOOR_FLOODABLE);
    ASSERT(tileCatalog[dungeonTile].flags & T_IS_DF_TRAP);

    test_teardown_game();
}

TEST(test_trap_item_placement_rejects_wall) {
    test_init_arena(83);

    short targetX = player.loc.x + 1;
    short targetY = player.loc.y;

    // Place a wall on the target cell
    pmap[targetX][targetY].layers[DUNGEON] = WALL;

    // The cell is not FLOOR/FLOOR_FLOODABLE, so placement should be rejected
    enum tileType dungeonTile = pmap[targetX][targetY].layers[DUNGEON];
    ASSERT(dungeonTile != FLOOR && dungeonTile != FLOOR_FLOODABLE);
    ASSERT(tileCatalog[dungeonTile].flags & T_OBSTRUCTS_PASSABILITY);

    test_teardown_game();
}

// --- Trap item generation tests ---

TEST(test_trap_item_generation) {
    test_init_arena(90);

    // Generate each trap item kind
    for (short kind = TRAP_POISON_GAS; kind < NUMBER_TRAP_ITEM_KINDS; kind++) {
        item *trapItem = generateItem(TRAP_ITEM, kind);
        ASSERT(trapItem != NULL);
        ASSERT_EQ(trapItem->category, TRAP_ITEM);
        ASSERT_EQ(trapItem->kind, kind);
        ASSERT(trapItem->flags & ITEM_IDENTIFIED);
        ASSERT_EQ(trapItem->displayChar, G_TRAP);
        deleteItem(trapItem);
    }

    test_teardown_game();
}

TEST(test_trap_item_naming) {
    test_init_arena(91);

    item *trapItem = generateItem(TRAP_ITEM, TRAP_POISON_GAS);
    ASSERT(trapItem != NULL);

    char name[DCOLS];
    itemName(trapItem, name, false, false, NULL);
    ASSERT(strstr(name, "poison gas trap") != NULL);

    deleteItem(trapItem);
    test_teardown_game();
}

// --- Suite registration ---

SUITE(trap_hunting) {
    RUN_TEST(test_wand_trapping_extracts_poison_gas_trap);
    RUN_TEST(test_wand_trapping_extracts_confusion_gas_trap);
    RUN_TEST(test_wand_trapping_extracts_net_trap);
    RUN_TEST(test_wand_trapping_extracts_alarm_trap);
    RUN_TEST(test_wand_trapping_extracts_flamethrower_trap);
    RUN_TEST(test_wand_trapping_extracts_paralysis_trap);
    RUN_TEST(test_wand_trapping_fizzles_on_floor);
    RUN_TEST(test_wand_trapping_fizzles_on_hidden_trap);
    RUN_TEST(test_wand_trapping_fizzles_beyond_range_1);
    RUN_TEST(test_trap_item_placement_sets_hidden_tile);
    RUN_TEST(test_trap_item_placement_paralysis_uses_standalone_tile);
    RUN_TEST(test_trap_item_placement_fire_uses_burst_tile);
    RUN_TEST(test_placed_trap_triggers_on_monster);
    RUN_TEST(test_monsters_dont_avoid_player_placed_traps);
    RUN_TEST(test_trap_item_apply_places_and_consumes);
    RUN_TEST(test_trap_item_placement_rejects_existing_trap);
    RUN_TEST(test_trap_item_placement_rejects_wall);
    RUN_TEST(test_trap_item_generation);
    RUN_TEST(test_trap_item_naming);
}
