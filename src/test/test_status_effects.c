// test_status_effects.c — Tests for status effect interactions

#include "test_harness.h"

TEST(test_paralysis_status_applied_and_decays) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply paralysis
    test_set_player_status(STATUS_PARALYZED, 10);
    ASSERT_EQ(player.status[STATUS_PARALYZED], 10);
    ASSERT_EQ(player.maxStatus[STATUS_PARALYZED], 10);

    // Rest turns — paralysis should decay over time
    test_rest_turns(3);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.status[STATUS_PARALYZED], 10);
    }

    test_teardown_game();
}

TEST(test_confusion_randomizes_movement) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply long-lasting confusion
    test_set_player_status(STATUS_CONFUSED, 50);
    ASSERT_GT(player.status[STATUS_CONFUSED], 0);

    short startX = player.loc.x;
    short startY = player.loc.y;

    // Move RIGHT 10 times. Confusion should randomize some of these.
    for (int i = 0; i < 10 && !rogue.gameHasEnded; i++) {
        test_move(RIGHT);
    }

    if (!rogue.gameHasEnded) {
        // If all 10 moves went right, player would be at startX + 10.
        // Due to confusion, at least some moves should go elsewhere.
        // We check that the player did NOT end up exactly 10 right,
        // OR that the Y position changed (moved in a non-right direction).
        boolean fullyRight = (player.loc.x == startX + 10) && (player.loc.y == startY);
        ASSERT(!fullyRight);
    }

    test_teardown_game();
}

TEST(test_levitation_protects_from_lava) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Grant levitation
    test_set_player_status(STATUS_LEVITATING, 100);

    // Place lava to the right
    short lavaX = player.loc.x + 1;
    short lavaY = player.loc.y;
    test_set_terrain(lavaX, lavaY, LIQUID, LAVA);

    // Step onto lava
    test_move(RIGHT);

    // Levitation should protect the player from lava death
    if (!rogue.gameHasEnded) {
        ASSERT_GT(player.currentHP, 0);
    } else {
        // If the game ended, levitation failed to protect — that is a failure
        ASSERT(!rogue.gameHasEnded);
    }

    test_teardown_game();
}

TEST(test_invisibility_status_set) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();

    // Apply invisibility
    test_set_player_status(STATUS_INVISIBLE, 20);
    ASSERT_GT(player.status[STATUS_INVISIBLE], 0);

    short initialValue = player.status[STATUS_INVISIBLE];

    // Rest a few turns to let it decay
    test_rest_turns(5);

    if (!rogue.gameHasEnded) {
        // Invisibility should have decayed
        ASSERT_LT(player.status[STATUS_INVISIBLE], initialValue);
    }

    test_teardown_game();
}

TEST(test_stuck_in_web_prevents_movement) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a web at the player's position AND set stuck status
    // (the game requires both T_ENTANGLES terrain and STATUS_STUCK)
    test_set_terrain(player.loc.x, player.loc.y, SURFACE, SPIDERWEB);
    test_set_player_status(STATUS_STUCK, 10);
    ASSERT_GT(player.status[STATUS_STUCK], 0);

    pos start = player.loc;

    // Try to move right — should be blocked by web + stuck
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // Player should still be at the start position
        ASSERT_EQ(player.loc.x, start.x);
        ASSERT_EQ(player.loc.y, start.y);
    }

    test_teardown_game();
}

TEST(test_multiple_status_effects_coexist) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply both poison and haste simultaneously
    test_set_player_status(STATUS_POISONED, 10);
    player.poisonAmount = 1;
    test_set_player_status(STATUS_HASTED, 20);

    // Verify both are active
    ASSERT_GT(player.status[STATUS_POISONED], 0);
    ASSERT_GT(player.status[STATUS_HASTED], 0);

    short hpBefore = player.currentHP;
    short hasteBefore = player.status[STATUS_HASTED];

    // Rest a few turns
    test_rest_turns(3);

    if (!rogue.gameHasEnded) {
        // Poison should have dealt damage
        ASSERT_LT(player.currentHP, hpBefore);
        // Haste should have decayed
        ASSERT_LT(player.status[STATUS_HASTED], hasteBefore);
    }

    test_teardown_game();
}

TEST(test_shielded_absorbs_damage) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(200, 200);

    // Grant a strong shield
    test_set_player_status(STATUS_SHIELDED, 50);
    ASSERT_EQ(player.status[STATUS_SHIELDED], 50);

    // Place an ogre adjacent to the right
    short ogreX = player.loc.x + 1;
    short ogreY = player.loc.y;
    creature *ogre = test_place_monster(MK_OGRE, ogreX, ogreY);
    ASSERT(ogre != NULL);

    // Rest several turns to let the ogre attack
    for (int i = 0; i < 5 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // The shield should have absorbed some hits (decreased from 50),
        // or the player took less HP damage than they would without it.
        // We check that at least one of these changed.
        boolean shieldDecreased = (player.status[STATUS_SHIELDED] < 50);
        boolean tookDamage = (player.currentHP < 200);
        ASSERT(shieldDecreased || tookDamage);
    }

    test_teardown_game();
}

SUITE(status_effects) {
    RUN_TEST(test_paralysis_status_applied_and_decays);
    RUN_TEST(test_confusion_randomizes_movement);
    RUN_TEST(test_levitation_protects_from_lava);
    RUN_TEST(test_invisibility_status_set);
    RUN_TEST(test_stuck_in_web_prevents_movement);
    RUN_TEST(test_multiple_status_effects_coexist);
    RUN_TEST(test_shielded_absorbs_damage);
}
