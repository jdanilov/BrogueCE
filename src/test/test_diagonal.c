// test_diagonal.c — Tests for diagonal movement

#include "test_harness.h"

#define CENTER_X 40
#define CENTER_Y 15
#define ARENA_RADIUS 5
#define SEED 54321

TEST(test_diagonal_movement_works) {
    test_init_game(SEED);
    test_remove_all_monsters();
    test_clear_area(CENTER_X, CENTER_Y, ARENA_RADIUS);
    test_teleport_player(CENTER_X, CENTER_Y);

    pos start = player.loc;
    unsigned long turnBefore = rogue.playerTurnNumber;

    test_move(UPLEFT);

    ASSERT_EQ(player.loc.x, start.x - 1);
    ASSERT_EQ(player.loc.y, start.y - 1);
    ASSERT_GT(rogue.playerTurnNumber, turnBefore);

    test_teardown_game();
}

TEST(test_all_diagonal_directions) {
    short diags[] = { UPLEFT, UPRIGHT, DOWNLEFT, DOWNRIGHT };

    for (int i = 0; i < 4; i++) {
        test_init_game(SEED);
        test_remove_all_monsters();
        test_clear_area(CENTER_X, CENTER_Y, ARENA_RADIUS);
        test_teleport_player(CENTER_X, CENTER_Y);

        short dir = diags[i];
        short expectedX = CENTER_X + nbDirs[dir][0];
        short expectedY = CENTER_Y + nbDirs[dir][1];

        // Sanity: target must be in bounds
        ASSERT_GE(expectedX, 0);
        ASSERT_LT(expectedX, DCOLS);
        ASSERT_GE(expectedY, 0);
        ASSERT_LT(expectedY, DROWS);

        unsigned long turnBefore = rogue.playerTurnNumber;
        test_move(dir);

        ASSERT_EQ(player.loc.x, expectedX);
        ASSERT_EQ(player.loc.y, expectedY);
        ASSERT_GT(rogue.playerTurnNumber, turnBefore);

        test_teardown_game();
    }
}

TEST(test_diagonal_blocked_by_two_walls) {
    test_init_game(SEED);
    test_remove_all_monsters();
    test_clear_area(CENTER_X, CENTER_Y, ARENA_RADIUS);
    test_teleport_player(CENTER_X, CENTER_Y);

    // Place walls at (cx+1, cy) and (cx, cy-1) to block UPRIGHT movement.
    // UPRIGHT goes to (cx+1, cy-1). diagonalBlocked checks (cx, cy-1) and
    // (cx+1, cy) — both are walls with T_OBSTRUCTS_DIAGONAL_MOVEMENT,
    // so the move should be blocked.
    test_set_terrain(CENTER_X + 1, CENTER_Y, DUNGEON, WALL);
    test_set_terrain(CENTER_X, CENTER_Y - 1, DUNGEON, WALL);

    pos start = player.loc;
    unsigned long turnBefore = rogue.playerTurnNumber;

    test_move(UPRIGHT);

    // Player should not have moved
    ASSERT_EQ(player.loc.x, start.x);
    ASSERT_EQ(player.loc.y, start.y);
    // Turn should not have advanced
    ASSERT_EQ(rogue.playerTurnNumber, turnBefore);

    test_teardown_game();
}

SUITE(diagonal) {
    RUN_TEST(test_diagonal_movement_works);
    RUN_TEST(test_all_diagonal_directions);
    RUN_TEST(test_diagonal_blocked_by_two_walls);
}
