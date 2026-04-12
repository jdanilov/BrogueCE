// test_lifecycle.c — Tests for game lifecycle scenarios

#include "test_harness.h"

TEST(test_game_starts_not_ended) {
    test_init_game(12345);

    ASSERT(!rogue.gameHasEnded);

    test_teardown_game();
}

TEST(test_deepest_level_tracked) {
    test_init_game(54321);

    ASSERT_EQ(rogue.depthLevel, 1);
    ASSERT_EQ(rogue.deepestLevel, 1);

    // Descend to level 2
    test_teleport_player(rogue.downLoc.x, rogue.downLoc.y);
    useStairs(1);

    ASSERT_EQ(rogue.depthLevel, 2);
    ASSERT_GE(rogue.deepestLevel, 2);

    // Descend to level 3
    test_teleport_player(rogue.downLoc.x, rogue.downLoc.y);
    useStairs(1);

    ASSERT_EQ(rogue.depthLevel, 3);
    ASSERT_GE(rogue.deepestLevel, 3);

    test_teardown_game();
}

TEST(test_gold_accumulates) {
    test_init_game(12345);

    unsigned long startGold = rogue.gold;

    rogue.gold += 100;
    ASSERT_EQ(rogue.gold, startGold + 100);

    rogue.gold += 250;
    ASSERT_EQ(rogue.gold, startGold + 350);

    test_teardown_game();
}

TEST(test_nutrition_decreases_over_turns) {
    test_init_game(12345);

    // Remove all monsters so resting is safe
    test_remove_all_monsters();

    short startNutrition = player.status[STATUS_NUTRITION];
    ASSERT_GT(startNutrition, 0);

    // Rest for a number of turns; nutrition should decrease
    test_rest_turns(20);

    short endNutrition = player.status[STATUS_NUTRITION];
    ASSERT_LT(endNutrition, startNutrition);

    test_teardown_game();
}

TEST(test_turn_count_increases) {
    test_init_game(12345);

    // Remove all monsters so resting is safe
    test_remove_all_monsters();

    unsigned long startTurn = rogue.absoluteTurnNumber;

    test_rest_turns(10);

    ASSERT_GT(rogue.absoluteTurnNumber, startTurn);

    test_teardown_game();
}

TEST(test_seed_produces_consistent_start) {
    uint64_t seed = 99999;

    // First game
    test_init_game(seed);
    short x1 = player.loc.x;
    short y1 = player.loc.y;
    short depth1 = rogue.depthLevel;
    test_teardown_game();

    // Second game with same seed
    test_init_game(seed);
    short x2 = player.loc.x;
    short y2 = player.loc.y;
    short depth2 = rogue.depthLevel;
    test_teardown_game();

    ASSERT_EQ(x1, x2);
    ASSERT_EQ(y1, y2);
    ASSERT_EQ(depth1, depth2);
}

SUITE(lifecycle) {
    RUN_TEST(test_game_starts_not_ended);
    RUN_TEST(test_deepest_level_tracked);
    RUN_TEST(test_gold_accumulates);
    RUN_TEST(test_nutrition_decreases_over_turns);
    RUN_TEST(test_turn_count_increases);
    RUN_TEST(test_seed_produces_consistent_start);
}
