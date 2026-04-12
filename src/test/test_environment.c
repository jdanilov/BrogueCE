// test_environment.c — Tests for dungeon environment and terrain

#include "test_harness.h"

TEST(test_dungeon_has_walls) {
    test_init_game(12345);

    int wallCount = 0;
    for (short x = 0; x < DCOLS; x++) {
        for (short y = 0; y < DROWS; y++) {
            if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) {
                wallCount++;
            }
        }
    }

    ASSERT_GT(wallCount, 0);

    test_teardown_game();
}

TEST(test_dungeon_has_stairs) {
    test_init_game(12345);

    // Level 1 should have down stairs
    ASSERT_GE(rogue.downLoc.x, 0);
    ASSERT_LT(rogue.downLoc.x, DCOLS);
    ASSERT_GE(rogue.downLoc.y, 0);
    ASSERT_LT(rogue.downLoc.y, DROWS);

    test_teardown_game();
}

TEST(test_different_seeds_different_dungeons) {
    // Generate two dungeons with different seeds and verify they're different
    test_init_game(11111);
    pos player1 = player.loc;
    pos stairs1 = rogue.downLoc;
    test_teardown_game();

    test_init_game(22222);
    pos player2 = player.loc;
    pos stairs2 = rogue.downLoc;
    test_teardown_game();

    // Very unlikely both player positions AND stairs are identical
    boolean different = (player1.x != player2.x || player1.y != player2.y
                        || stairs1.x != stairs2.x || stairs1.y != stairs2.y);
    ASSERT(different);
}

TEST(test_same_seed_same_dungeon) {
    test_init_game(42424);
    pos player1 = player.loc;
    pos stairs1 = rogue.downLoc;
    int monsters1 = test_count_monsters();
    test_teardown_game();

    test_init_game(42424);
    pos player2 = player.loc;
    pos stairs2 = rogue.downLoc;
    int monsters2 = test_count_monsters();
    test_teardown_game();

    ASSERT_EQ(player1.x, player2.x);
    ASSERT_EQ(player1.y, player2.y);
    ASSERT_EQ(stairs1.x, stairs2.x);
    ASSERT_EQ(stairs1.y, stairs2.y);
    ASSERT_EQ(monsters1, monsters2);
}

TEST(test_monsters_on_level) {
    test_init_game(12345);

    int count = test_count_monsters();
    ASSERT_GT(count, 0);

    // Verify each monster is at a valid position
    creatureIterator iter = iterateCreatures(monsters);
    while (hasNextCreature(iter)) {
        creature *monst = nextCreature(&iter);
        ASSERT_GE(monst->loc.x, 0);
        ASSERT_LT(monst->loc.x, DCOLS);
        ASSERT_GE(monst->loc.y, 0);
        ASSERT_LT(monst->loc.y, DROWS);
        ASSERT_GT(monst->currentHP, 0);
    }

    test_teardown_game();
}

TEST(test_player_on_valid_floor) {
    test_init_game(12345);

    // Player should be on passable terrain
    ASSERT(!cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY));
    ASSERT(pmap[player.loc.x][player.loc.y].flags & HAS_PLAYER);

    test_teardown_game();
}

SUITE(environment) {
    RUN_TEST(test_dungeon_has_walls);
    RUN_TEST(test_dungeon_has_stairs);
    RUN_TEST(test_player_on_valid_floor);
    RUN_TEST(test_monsters_on_level);
    RUN_TEST(test_same_seed_same_dungeon);
    RUN_TEST(test_different_seeds_different_dungeons);
}
