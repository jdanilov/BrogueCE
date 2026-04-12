// test_hallucination.c — Tests for hallucination potion rework

#include "test_harness.h"

// --- Feature #1: Hallucination gas from thrown potion ---

TEST(test_hallucination_gas_tile_causes_status) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place hallucination gas at the player's position
    test_set_terrain(player.loc.x, player.loc.y, GAS, HALLUCINATION_GAS);
    pmap[player.loc.x][player.loc.y].volume = 1000;

    // Verify the gas has the right terrain flag
    ASSERT(test_cell_has_terrain_flag(player.loc.x, player.loc.y, T_CAUSES_HALLUCINATION));

    // Step off and back on (or rest) to trigger applyInstantTileEffectsToCreature
    test_rest();

    if (!rogue.gameHasEnded) {
        ASSERT_GT(player.status[STATUS_HALLUCINATING], 0);
    }

    test_teardown_game();
}

TEST(test_hallucination_gas_affects_monster) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a monster away from the player
    short mx = player.loc.x + 4;
    short my = player.loc.y;
    creature *monst = test_place_monster(MK_GOBLIN, mx, my);
    ASSERT(monst != NULL);

    // Place hallucination gas at the monster's position
    test_set_terrain(mx, my, GAS, HALLUCINATION_GAS);
    pmap[mx][my].volume = 1000;

    // Rest a few turns to let the gas affect the monster
    for (int i = 0; i < 3 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Find the monster (it may have moved due to hallucination)
        creature *found = NULL;
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *c = nextCreature(&it);
            if (c->info.monsterID == MK_GOBLIN) {
                found = c;
                break;
            }
        }
        if (found) {
            ASSERT_GT(found->status[STATUS_HALLUCINATING], 0);
        }
    }

    test_teardown_game();
}

TEST(test_hallucination_gas_dissipates) {
    test_init_game(99999);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 5);
    test_remove_all_monsters();

    // Place hallucination gas away from the player
    short gx = cx + 3;
    short gy = cy;
    if (gx < DCOLS && gy < DROWS) {
        test_set_terrain(gx, gy, GAS, HALLUCINATION_GAS);
        pmap[gx][gy].volume = 1000;

        ASSERT_EQ(pmap[gx][gy].layers[GAS], HALLUCINATION_GAS);

        // Advance environment to let gas dissipate (it's TM_GAS_DISSIPATES_QUICKLY)
        test_advance_environment(100);

        boolean dissipated = (pmap[gx][gy].volume < 1000
                              || pmap[gx][gy].layers[GAS] == NOTHING);
        ASSERT(dissipated);
    }

    test_teardown_game();
}

TEST(test_hallucinating_monster_moves_erratically) {
    // Test across multiple seeds to handle RNG variance
    boolean movedLaterally = false;

    for (int seed = 1; seed <= 5 && !movedLaterally; seed++) {
        test_init_game(seed * 7777);

        test_clear_area(player.loc.x, player.loc.y, 12);
        test_remove_all_monsters();
        test_set_player_hp(500, 500);

        // Place the monster directly above the player so lateral = x movement
        short mx = player.loc.x;
        short my = player.loc.y - 8;
        if (my < 1) my = 1;
        creature *monst = test_place_monster(MK_GOBLIN, mx, my);
        if (monst == NULL) { test_teardown_game(); continue; }

        // Apply hallucination to the monster
        monst->status[STATUS_HALLUCINATING] = 100;
        monst->maxStatus[STATUS_HALLUCINATING] = 100;

        // Track monster's x position over several turns
        for (int i = 0; i < 30 && !rogue.gameHasEnded; i++) {
            test_rest();
            // If monster moved off its starting x-column, it moved laterally
            if (monst->loc.x != mx) {
                movedLaterally = true;
                break;
            }
        }

        test_teardown_game();
    }

    // Over 5 seeds x 30 turns at 50% random movement chance, we should see
    // lateral movement at least once
    ASSERT(movedLaterally);
}

// --- Feature #3: Water cures hallucination ---

TEST(test_water_cures_hallucination) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply hallucination
    test_set_player_status(STATUS_HALLUCINATING, 50);
    ASSERT_GT(player.status[STATUS_HALLUCINATING], 0);

    // Place shallow water to the right
    short wx = player.loc.x + 1;
    short wy = player.loc.y;
    test_set_terrain(wx, wy, LIQUID, SHALLOW_WATER);

    // Step into the water
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        ASSERT_EQ(player.loc.x, wx);
        ASSERT_EQ(player.status[STATUS_HALLUCINATING], 0);
    }

    test_teardown_game();
}

TEST(test_water_does_not_cure_hallucination_when_levitating) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply hallucination and levitation
    test_set_player_status(STATUS_HALLUCINATING, 50);
    test_set_player_status(STATUS_LEVITATING, 100);

    // Place shallow water to the right
    short wx = player.loc.x + 1;
    short wy = player.loc.y;
    test_set_terrain(wx, wy, LIQUID, SHALLOW_WATER);

    // Step onto the water
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // Levitating — should still be hallucinating
        ASSERT_GT(player.status[STATUS_HALLUCINATING], 0);
    }

    test_teardown_game();
}

TEST(test_deep_water_also_cures_hallucination) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply hallucination
    test_set_player_status(STATUS_HALLUCINATING, 50);

    // Place deep water to the right
    short wx = player.loc.x + 1;
    short wy = player.loc.y;
    test_set_terrain(wx, wy, LIQUID, DEEP_WATER);

    // Step into the water
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // Deep water should also cure hallucination
        // (player may have been pulled around by currents, but hallucination should be gone)
        ASSERT_EQ(player.status[STATUS_HALLUCINATING], 0);
    }

    test_teardown_game();
}

// --- Feature #6: Hallucination reveals secrets ---

TEST(test_hallucination_can_reveal_secrets) {
    // Use many attempts with different seeds to test the 3% chance
    boolean revealedAny = false;

    for (int seed = 1; seed <= 20 && !revealedAny; seed++) {
        test_init_game(seed * 1000);

        test_clear_area(player.loc.x, player.loc.y, 5);
        test_remove_all_monsters();
        test_set_player_hp(500, 500);

        // Place a secret door adjacent to the player
        short sx = player.loc.x + 1;
        short sy = player.loc.y;
        test_set_terrain(sx, sy, DUNGEON, SECRET_DOOR);
        pmap[sx][sy].flags |= DISCOVERED; // must be in view

        // Verify the secret is there
        ASSERT(cellHasTMFlag((pos){ sx, sy }, TM_IS_SECRET));

        // Apply long hallucination
        test_set_player_status(STATUS_HALLUCINATING, 200);

        // Rest many turns to give the 3% chance time to trigger
        for (int i = 0; i < 100 && !rogue.gameHasEnded; i++) {
            test_rest();
            if (!cellHasTMFlag((pos){ sx, sy }, TM_IS_SECRET)) {
                revealedAny = true;
                break;
            }
        }

        test_teardown_game();
    }

    // Over 20 seeds x 100 turns, the cumulative probability of never revealing
    // a secret at 3% per turn is vanishingly small: (0.97)^2000 ~ 0
    ASSERT(revealedAny);
}

TEST(test_no_secret_reveal_without_hallucination) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a secret door adjacent to the player
    short sx = player.loc.x + 1;
    short sy = player.loc.y;
    test_set_terrain(sx, sy, DUNGEON, SECRET_DOOR);
    pmap[sx][sy].flags |= DISCOVERED;

    ASSERT(cellHasTMFlag((pos){ sx, sy }, TM_IS_SECRET));

    // No hallucination — just rest
    for (int i = 0; i < 50 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Secret should remain hidden (no hallucination to reveal it)
        ASSERT(cellHasTMFlag((pos){ sx, sy }, TM_IS_SECRET));
    }

    test_teardown_game();
}

// --- Feature #7: Hallucination grants fear immunity ---

TEST(test_hallucination_clears_existing_fear) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Apply fear first
    test_set_player_status(STATUS_MAGICAL_FEAR, 50);
    ASSERT_GT(player.status[STATUS_MAGICAL_FEAR], 0);

    // Now apply hallucination
    test_set_player_status(STATUS_HALLUCINATING, 30);

    // Rest a turn to let the status processing clear fear
    test_rest();

    if (!rogue.gameHasEnded) {
        ASSERT_EQ(player.status[STATUS_MAGICAL_FEAR], 0);
        ASSERT_GT(player.status[STATUS_HALLUCINATING], 0);
    }

    test_teardown_game();
}

TEST(test_hallucination_clears_monster_fear) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a monster
    short mx = player.loc.x + 4;
    short my = player.loc.y;
    creature *monst = test_place_monster(MK_GOBLIN, mx, my);
    ASSERT(monst != NULL);

    // Apply fear to the monster
    monst->status[STATUS_MAGICAL_FEAR] = 100;
    monst->maxStatus[STATUS_MAGICAL_FEAR] = 100;
    monst->creatureState = MONSTER_FLEEING;

    // Apply hallucination to the same monster
    monst->status[STATUS_HALLUCINATING] = 50;
    monst->maxStatus[STATUS_HALLUCINATING] = 50;

    // Rest to let decrementMonsterStatus process
    for (int i = 0; i < 3 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Find the goblin
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *c = nextCreature(&it);
            if (c->info.monsterID == MK_GOBLIN) {
                ASSERT_EQ(c->status[STATUS_MAGICAL_FEAR], 0);
                ASSERT_NE(c->creatureState, MONSTER_FLEEING);
                break;
            }
        }
    }

    test_teardown_game();
}

// --- Hallucination status basic behavior ---

TEST(test_hallucination_status_decays) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    test_set_player_status(STATUS_HALLUCINATING, 20);
    ASSERT_EQ(player.status[STATUS_HALLUCINATING], 20);

    test_rest_turns(5);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.status[STATUS_HALLUCINATING], 20);
    }

    test_teardown_game();
}

SUITE(hallucination) {
    // Feature #1: Gas from thrown potion
    RUN_TEST(test_hallucination_gas_tile_causes_status);
    RUN_TEST(test_hallucination_gas_affects_monster);
    RUN_TEST(test_hallucination_gas_dissipates);
    RUN_TEST(test_hallucinating_monster_moves_erratically);

    // Feature #3: Water cures hallucination
    RUN_TEST(test_water_cures_hallucination);
    RUN_TEST(test_water_does_not_cure_hallucination_when_levitating);
    RUN_TEST(test_deep_water_also_cures_hallucination);

    // Feature #6: Reveals secrets
    RUN_TEST(test_hallucination_can_reveal_secrets);
    RUN_TEST(test_no_secret_reveal_without_hallucination);

    // Feature #7: Fear immunity
    RUN_TEST(test_hallucination_clears_existing_fear);
    RUN_TEST(test_hallucination_clears_monster_fear);

    // Basic behavior
    RUN_TEST(test_hallucination_status_decays);
}
