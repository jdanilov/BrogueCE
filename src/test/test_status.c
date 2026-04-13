// test_status.c — Tests for status effects and basic game state

#include "test_harness.h"

TEST(test_poison_reduces_hp) {
    test_init_game(12345);

    short startHP = player.currentHP;
    ASSERT_GT(startHP, 1);

    // Apply poison
    player.status[STATUS_POISONED] = 5;
    player.maxStatus[STATUS_POISONED] = 5;
    player.poisonAmount = 1;

    // Rest to let poison tick
    test_rest_turns(3);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.currentHP, startHP);
    }

    test_teardown_game();
}

TEST(test_haste_status_decays) {
    test_init_game(12345);

    test_set_player_status(STATUS_HASTED, 10);
    ASSERT_EQ(player.status[STATUS_HASTED], 10);

    test_rest_turns(3);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.status[STATUS_HASTED], 10);
    }

    test_teardown_game();
}

TEST(test_nutrition_decreases) {
    test_init_game(12345);

    long startNutrition = player.status[STATUS_NUTRITION];
    ASSERT_GT(startNutrition, 0);

    test_rest_turns(10);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.status[STATUS_NUTRITION], startNutrition);
    }

    test_teardown_game();
}

TEST(test_regeneration) {
    test_init_arena(12345);

    // Damage the player to half HP in a safe, monster-free arena
    short maxHP = player.info.maxHP;
    test_set_player_hp(maxHP / 2, maxHP);
    short damagedHP = player.currentHP;

    // Rest many turns for regen to kick in
    test_rest_turns(50);

    if (!rogue.gameHasEnded) {
        ASSERT_GT(player.currentHP, damagedHP);
    }

    test_teardown_game();
}

TEST(test_depth_level_starts_at_one) {
    test_init_game(12345);

    ASSERT_EQ(rogue.depthLevel, 1);
    ASSERT_EQ(rogue.deepestLevel, 1);

    test_teardown_game();
}

TEST(test_game_seed_is_set) {
    test_init_game(12345);

    ASSERT_EQ(rogue.seed, 12345);

    test_teardown_game();
}

TEST(test_player_starts_with_equipment) {
    test_init_game(12345);

    // Player should have a weapon equipped
    ASSERT(rogue.weapon != NULL);
    // Player should have armor equipped
    ASSERT(rogue.armor != NULL);

    test_teardown_game();
}

SUITE(status) {
    RUN_TEST(test_depth_level_starts_at_one);
    RUN_TEST(test_game_seed_is_set);
    RUN_TEST(test_player_starts_with_equipment);
    RUN_TEST(test_poison_reduces_hp);
    RUN_TEST(test_haste_status_decays);
    RUN_TEST(test_nutrition_decreases);
    RUN_TEST(test_regeneration);
}
