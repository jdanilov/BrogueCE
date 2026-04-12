// test_levels.c — Tests for level transitions (descend/ascend stairs)

#include "test_harness.h"

// Helper: descend one level by calling useStairs directly.
// Teleports player to downLoc first (required for useStairs to work).
static void descend(void) {
    test_teleport_player(rogue.downLoc.x, rogue.downLoc.y);
    useStairs(1);
}

// Helper: ascend one level.
static void ascend(void) {
    test_teleport_player(rogue.upLoc.x, rogue.upLoc.y);
    useStairs(-1);
}

TEST(test_descend_stairs) {
    test_init_game(12345);
    test_remove_all_monsters();
    test_set_player_hp(200, 200);

    ASSERT_EQ(rogue.depthLevel, 1);

    descend();

    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 2);
    ASSERT_GE(rogue.deepestLevel, 2);

    test_teardown_game();
}

TEST(test_ascend_stairs) {
    test_init_game(12345);
    test_remove_all_monsters();
    test_set_player_hp(200, 200);

    descend();

    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 2);

    // Now ascend back to level 1
    test_remove_all_monsters();
    ascend();

    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 1);

    test_teardown_game();
}

TEST(test_player_position_at_stairs_after_descent) {
    test_init_game(12345);
    test_remove_all_monsters();
    test_set_player_hp(200, 200);

    descend();

    ASSERT(!rogue.gameHasEnded);

    // Player should be at a valid position on the new level
    ASSERT_GE(player.loc.x, 0);
    ASSERT_LT(player.loc.x, DCOLS);
    ASSERT_GE(player.loc.y, 0);
    ASSERT_LT(player.loc.y, DROWS);

    // Player cell should be passable terrain
    ASSERT(!cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY));

    test_teardown_game();
}

TEST(test_deepest_level_tracks_maximum) {
    test_init_game(12345);
    test_remove_all_monsters();
    test_set_player_hp(200, 200);

    // Descend to level 2
    descend();
    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 2);

    // Descend to level 3
    test_remove_all_monsters();
    descend();
    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 3);
    ASSERT_EQ(rogue.deepestLevel, 3);

    // Ascend to level 2
    test_remove_all_monsters();
    ascend();
    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 2);

    // deepestLevel should still be 3
    ASSERT_EQ(rogue.deepestLevel, 3);

    test_teardown_game();
}

TEST(test_depth_level_increments_correctly) {
    test_init_game(12345);
    test_remove_all_monsters();
    test_set_player_hp(200, 200);

    ASSERT_EQ(rogue.depthLevel, 1);

    descend();
    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 2);

    test_remove_all_monsters();
    descend();
    ASSERT(!rogue.gameHasEnded);
    ASSERT_EQ(rogue.depthLevel, 3);

    test_teardown_game();
}

SUITE(levels) {
    RUN_TEST(test_descend_stairs);
    RUN_TEST(test_ascend_stairs);
    RUN_TEST(test_player_position_at_stairs_after_descent);
    RUN_TEST(test_deepest_level_tracks_maximum);
    RUN_TEST(test_depth_level_increments_correctly);
}
