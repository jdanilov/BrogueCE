// test_movement.c — Tests for player movement

#include "test_harness.h"

TEST(test_move_up) {
    test_init_game(12345);
    pos start = player.loc;

    // Find a direction we can actually move
    short newY = start.y - 1;
    if (newY >= 0
        && !cellHasTerrainFlag((pos){ start.x, newY }, T_OBSTRUCTS_PASSABILITY)
        && !(pmap[start.x][newY].flags & HAS_MONSTER)) {

        unsigned long turnBefore = rogue.playerTurnNumber;
        test_move(UP);

        ASSERT_EQ(player.loc.x, start.x);
        ASSERT_EQ(player.loc.y, start.y - 1);
        ASSERT_GT(rogue.playerTurnNumber, turnBefore);
    }
    test_teardown_game();
}

TEST(test_move_to_open_cell) {
    test_init_game(12345);

    // Find a direction with guaranteed open floor (no wall, no monster, no stairs)
    boolean moved = false;
    for (short dir = 0; dir < DIRECTION_COUNT && !moved; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)
            && !(pmap[nx][ny].flags & (HAS_MONSTER | HAS_STAIRS))) {

            pos before = player.loc;
            unsigned long turnBefore = rogue.playerTurnNumber;
            test_move(dir);

            ASSERT_EQ(player.loc.x, nx);
            ASSERT_EQ(player.loc.y, ny);
            ASSERT_GT(rogue.playerTurnNumber, turnBefore);
            ASSERT_NE(player.loc.x * 1000 + player.loc.y,
                       before.x * 1000 + before.y);
            moved = true;
        }
    }
    ASSERT(moved);
    test_teardown_game();
}

TEST(test_move_blocked_by_wall) {
    test_init_game(12345);

    // Find a wall adjacent to the player
    boolean foundWall = false;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short newX = player.loc.x + nbDirs[dir][0];
        short newY = player.loc.y + nbDirs[dir][1];
        if (newX >= 0 && newX < DCOLS && newY >= 0 && newY < DROWS
            && cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {

            pos start = player.loc;
            unsigned long turnBefore = rogue.playerTurnNumber;
            test_move(dir);

            // Player should not have moved
            ASSERT_EQ(player.loc.x, start.x);
            ASSERT_EQ(player.loc.y, start.y);
            // Turn should not have advanced
            ASSERT_EQ(rogue.playerTurnNumber, turnBefore);
            foundWall = true;
            break;
        }
    }
    ASSERT(foundWall);
    test_teardown_game();
}

TEST(test_rest_advances_turn) {
    test_init_game(12345);

    pos start = player.loc;
    unsigned long turnBefore = rogue.playerTurnNumber;
    test_rest();

    // Position unchanged
    ASSERT_EQ(player.loc.x, start.x);
    ASSERT_EQ(player.loc.y, start.y);
    // Turn advanced
    ASSERT_GT(rogue.playerTurnNumber, turnBefore);

    test_teardown_game();
}

TEST(test_multiple_moves) {
    test_init_game(12345);

    // Move and rest a few times, verify the game doesn't crash
    for (int i = 0; i < 10 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    ASSERT_GE(rogue.playerTurnNumber, 10);
    test_teardown_game();
}

TEST(test_player_position_valid_after_init) {
    test_init_game(99999);

    ASSERT_GE(player.loc.x, 0);
    ASSERT_LT(player.loc.x, DCOLS);
    ASSERT_GE(player.loc.y, 0);
    ASSERT_LT(player.loc.y, DROWS);
    ASSERT(pmap[player.loc.x][player.loc.y].flags & HAS_PLAYER);

    test_teardown_game();
}

SUITE(movement) {
    RUN_TEST(test_player_position_valid_after_init);
    RUN_TEST(test_move_up);
    RUN_TEST(test_move_to_open_cell);
    RUN_TEST(test_move_blocked_by_wall);
    RUN_TEST(test_rest_advances_turn);
    RUN_TEST(test_multiple_moves);
}
