// test_edge_cases.c — Tests for edge cases and boundary conditions

#include "test_harness.h"

TEST(test_map_dimensions_correct) {
    // Verify the fundamental map dimension constants
    // DCOLS = COLS - STAT_BAR_WIDTH - 1 = 100 - 20 - 1 = 79
    ASSERT_EQ(DCOLS, 79);
    ASSERT_EQ(DROWS, 29);
}

TEST(test_player_position_within_bounds) {
    test_init_game(42);

    // Player should be within the playable area (not on the outer walls)
    ASSERT_GE(player.loc.x, 1);
    ASSERT_LT(player.loc.x, DCOLS - 1);
    ASSERT_GE(player.loc.y, 1);
    ASSERT_LT(player.loc.y, DROWS - 1);

    test_teardown_game();
}

TEST(test_monster_at_1hp_dies_from_attack) {
    test_init_game(12345);

    // Set player HP high to survive any counterattack
    test_set_player_hp(500, 500);

    // Give the player a strong weapon
    test_give_item(WEAPON, SWORD, 5);

    // Find an open adjacent cell for the rat
    pos monsterPos;
    boolean placed = false;
    for (short dir = 0; dir < DIRECTION_COUNT && !placed; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 1 && nx < DCOLS - 1 && ny >= 1 && ny < DROWS - 1
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)
            && !(pmap[nx][ny].flags & (HAS_MONSTER | HAS_STAIRS))) {
            monsterPos.x = nx;
            monsterPos.y = ny;
            placed = true;
        }
    }
    ASSERT(placed);

    // Place a rat and set it to 1 HP
    creature *rat = test_place_monster(MK_RAT, monsterPos.x, monsterPos.y);
    ASSERT(rat != NULL);
    rat->currentHP = 1;

    // Determine direction to the rat
    short dir = -1;
    for (short d = 0; d < DIRECTION_COUNT; d++) {
        if (player.loc.x + nbDirs[d][0] == monsterPos.x
            && player.loc.y + nbDirs[d][1] == monsterPos.y) {
            dir = d;
            break;
        }
    }
    ASSERT_GE(dir, 0);

    // Attack the rat by moving into it
    test_move(dir);

    // The rat should be dead and gone
    creature *ratAfter = test_monster_at(monsterPos.x, monsterPos.y);
    ASSERT(ratAfter == NULL);

    test_teardown_game();
}

TEST(test_pack_item_limit) {
    test_init_game(12345);

    // Count initial pack items
    int initialCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        initialCount++;
    }

    // Try to add items up to and beyond the pack limit
    int added = 0;
    for (int i = 0; i < MAX_PACK_ITEMS + 5; i++) {
        item *theItem = test_give_item(POTION, 0, 0);
        if (theItem != NULL) {
            added++;
        }
    }

    // Count final pack items
    int finalCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        finalCount++;
    }

    // We should have been able to add at least some items without crashing
    ASSERT_GT(added, 0);
    // The final count should reflect the items we added (plus initial items)
    ASSERT_EQ(finalCount, initialCount + added);

    test_teardown_game();
}

TEST(test_multiple_init_teardown_cycles) {
    // Cycle 1
    test_init_game(111);
    ASSERT_GT(player.currentHP, 0);
    short hp1 = player.currentHP;
    test_teardown_game();

    // Cycle 2
    test_init_game(222);
    ASSERT_GT(player.currentHP, 0);
    test_teardown_game();

    // Cycle 3 — use the same seed as cycle 1 to verify fresh state
    test_init_game(111);
    ASSERT_GT(player.currentHP, 0);
    // With the same seed, HP should match cycle 1
    ASSERT_EQ(player.currentHP, hp1);
    test_teardown_game();
}

TEST(test_clear_area_makes_passable) {
    test_init_game(12345);

    // Pick a center point away from the player
    short cx = DCOLS / 2;
    short cy = DROWS / 2;
    short radius = 3;

    // Clear the area
    test_clear_area(cx, cy, radius);

    // Verify all cells in the cleared area are passable
    for (short x = cx - radius; x <= cx + radius; x++) {
        for (short y = cy - radius; y <= cy + radius; y++) {
            if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;
            ASSERT(!test_cell_has_terrain_flag(x, y, T_OBSTRUCTS_PASSABILITY));
        }
    }

    test_teardown_game();
}

TEST(test_teleport_to_valid_position) {
    test_init_game(12345);

    // Find an open floor cell to teleport to
    pos target;
    boolean found = test_find_open_floor(DCOLS / 2, DROWS / 2, &target);
    ASSERT(found);

    // Teleport the player
    test_teleport_player(target.x, target.y);

    // Verify player position matches the target
    ASSERT_EQ(player.loc.x, target.x);
    ASSERT_EQ(player.loc.y, target.y);

    // Verify HAS_PLAYER flag is set at new position
    ASSERT(pmap[target.x][target.y].flags & HAS_PLAYER);

    test_teardown_game();
}

SUITE(edge_cases) {
    RUN_TEST(test_map_dimensions_correct);
    RUN_TEST(test_player_position_within_bounds);
    RUN_TEST(test_monster_at_1hp_dies_from_attack);
    RUN_TEST(test_pack_item_limit);
    RUN_TEST(test_multiple_init_teardown_cycles);
    RUN_TEST(test_clear_area_makes_passable);
    RUN_TEST(test_teleport_to_valid_position);
}
