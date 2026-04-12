// test_terrain.c — Tests for terrain interactions (lava, water, chasms, webs)

#include "test_harness.h"

TEST(test_stepping_on_lava_terrain_flag) {
    test_init_game(12345);

    // Set up a controlled area around the player
    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();

    short lavaX = player.loc.x + 1;
    short lavaY = player.loc.y;
    test_set_terrain(lavaX, lavaY, LIQUID, LAVA);

    // Verify the lava tile has T_LAVA_INSTA_DEATH flag
    ASSERT(test_cell_has_terrain_flag(lavaX, lavaY, T_LAVA_INSTA_DEATH));

    test_teardown_game();
}

TEST(test_lava_kills_monster) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();

    // Place a rat on floor, then put lava under it
    short lavaX = player.loc.x + 3;
    short lavaY = player.loc.y;
    creature *rat = test_place_monster(MK_RAT, lavaX, lavaY);
    ASSERT(rat != NULL);

    // Place lava on the monster's tile
    test_set_terrain(lavaX, lavaY, LIQUID, LAVA);

    // Advance a few turns — the monster should die from lava
    test_rest_turns(3);

    // The rat should be dead (removed from map)
    creature *remains = test_monster_at(lavaX, lavaY);
    ASSERT(remains == NULL);

    test_teardown_game();
}

TEST(test_player_refuses_to_walk_into_known_lava) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();

    // Place lava to the right (it will be DISCOVERED from clear_area + updateVision)
    short lavaX = player.loc.x + 1;
    short lavaY = player.loc.y;
    test_set_terrain(lavaX, lavaY, LIQUID, LAVA);

    pos start = player.loc;
    unsigned long turnBefore = rogue.playerTurnNumber;

    // Try to step onto known lava — game should refuse
    test_move(RIGHT);

    // Player should NOT have moved (game blocks: "that would be certain death!")
    ASSERT_EQ(player.loc.x, start.x);
    ASSERT_EQ(player.loc.y, start.y);
    ASSERT_EQ(rogue.playerTurnNumber, turnBefore);

    test_teardown_game();
}

TEST(test_lava_safe_with_levitation) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(100, 100);

    // Give levitation before stepping on lava
    test_set_player_status(STATUS_LEVITATING, 100);

    short lavaX = player.loc.x + 1;
    short lavaY = player.loc.y;
    test_set_terrain(lavaX, lavaY, LIQUID, LAVA);

    test_move(RIGHT);

    // Player should survive thanks to levitation
    if (!rogue.gameHasEnded) {
        ASSERT_GT(player.currentHP, 0);
    } else {
        // If game ended, that means levitation did not protect — fail
        ASSERT(!rogue.gameHasEnded);
    }

    test_teardown_game();
}

TEST(test_lava_safe_with_fire_immunity) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(100, 100);

    // Give fire immunity before stepping on lava
    test_set_player_status(STATUS_IMMUNE_TO_FIRE, 100);

    short lavaX = player.loc.x + 1;
    short lavaY = player.loc.y;
    test_set_terrain(lavaX, lavaY, LIQUID, LAVA);

    test_move(RIGHT);

    // Player should survive thanks to fire immunity
    if (!rogue.gameHasEnded) {
        ASSERT_GT(player.currentHP, 0);
    } else {
        ASSERT(!rogue.gameHasEnded);
    }

    test_teardown_game();
}

TEST(test_chasm_causes_descent) {
    test_init_game(12345);

    ASSERT_EQ(rogue.depthLevel, 1);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(100, 100);

    // Place a chasm to the right of the player
    short chasmX = player.loc.x + 1;
    short chasmY = player.loc.y;
    test_set_terrain(chasmX, chasmY, LIQUID, CHASM);

    // Step onto the chasm
    test_move(RIGHT);

    // Chasm should cause descent to the next level
    ASSERT_GT(rogue.depthLevel, 1);

    test_teardown_game();
}

TEST(test_deep_water_with_levitation) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(100, 100);

    // Give levitation so deep water is safe
    test_set_player_status(STATUS_LEVITATING, 100);

    short waterX = player.loc.x + 1;
    short waterY = player.loc.y;
    test_set_terrain(waterX, waterY, LIQUID, DEEP_WATER);

    test_move(RIGHT);

    // Player should survive — levitation keeps them above water
    if (!rogue.gameHasEnded) {
        ASSERT_GT(player.currentHP, 0);
        ASSERT_EQ(player.loc.x, waterX);
        ASSERT_EQ(player.loc.y, waterY);
    }

    test_teardown_game();
}

TEST(test_spiderweb_entangles) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();
    test_set_player_hp(100, 100);

    // Place a spiderweb on the surface layer to the right
    short webX = player.loc.x + 1;
    short webY = player.loc.y;
    test_set_terrain(webX, webY, SURFACE, SPIDERWEB);

    // Step onto the web
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // Player should be stuck from the web
        ASSERT_GT(player.status[STATUS_STUCK], 0);
    }

    test_teardown_game();
}

SUITE(terrain) {
    RUN_TEST(test_stepping_on_lava_terrain_flag);
    RUN_TEST(test_lava_kills_monster);
    RUN_TEST(test_player_refuses_to_walk_into_known_lava);
    RUN_TEST(test_lava_safe_with_levitation);
    RUN_TEST(test_lava_safe_with_fire_immunity);
    RUN_TEST(test_chasm_causes_descent);
    RUN_TEST(test_deep_water_with_levitation);
    RUN_TEST(test_spiderweb_entangles);
}
