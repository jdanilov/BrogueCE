// test_infrastructure.c — Tests for the test helpers themselves + stress tests

#include "test_harness.h"

TEST(test_clear_area_creates_open_floor) {
    test_init_game(12345);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 3);

    // All cells in the cleared area should be passable floor
    for (short x = cx - 3; x <= cx + 3; x++) {
        for (short y = cy - 3; y <= cy + 3; y++) {
            if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;
            ASSERT_EQ(pmap[x][y].layers[DUNGEON], FLOOR);
            ASSERT_EQ(pmap[x][y].layers[LIQUID], NOTHING);
            ASSERT_EQ(pmap[x][y].layers[GAS], NOTHING);
            ASSERT_EQ(pmap[x][y].layers[SURFACE], NOTHING);
            ASSERT(!cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY));
        }
    }

    test_teardown_game();
}

TEST(test_clear_area_removes_monsters) {
    test_init_game(12345);

    short cx = player.loc.x;
    short cy = player.loc.y;

    // Place some monsters in the area
    pos spot;
    if (test_find_open_floor(cx + 2, cy, &spot)) {
        test_place_monster(MK_RAT, spot.x, spot.y);
    }

    test_clear_area(cx, cy, 3);

    // No monsters should remain in the cleared area
    for (short x = cx - 3; x <= cx + 3; x++) {
        for (short y = cy - 3; y <= cy + 3; y++) {
            if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;
            if (x == player.loc.x && y == player.loc.y) continue;
            ASSERT(!(pmap[x][y].flags & HAS_MONSTER));
        }
    }

    test_teardown_game();
}

TEST(test_remove_all_monsters_empties_level) {
    test_init_game(12345);

    int before = test_count_monsters();
    ASSERT_GT(before, 0);

    test_remove_all_monsters();

    int after = test_count_monsters();
    ASSERT_EQ(after, 0);

    test_teardown_game();
}

TEST(test_set_terrain_places_tile) {
    test_init_game(12345);

    short cx = player.loc.x;
    short cy = player.loc.y;

    // Clear area first so we have open floor
    test_clear_area(cx, cy, 2);

    // Place lava on an adjacent cell
    short tx = cx + 1;
    short ty = cy;
    if (tx < DCOLS) {
        test_set_terrain(tx, ty, LIQUID, LAVA);
        ASSERT_EQ(pmap[tx][ty].layers[LIQUID], LAVA);
        ASSERT(cellHasTerrainFlag((pos){ tx, ty }, T_LAVA_INSTA_DEATH));
    }

    test_teardown_game();
}

TEST(test_set_terrain_gas_layer) {
    test_init_game(12345);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 2);

    short tx = cx + 1;
    short ty = cy;
    if (tx < DCOLS) {
        test_set_terrain(tx, ty, GAS, POISON_GAS);
        pmap[tx][ty].volume = 1000;
        ASSERT_EQ(pmap[tx][ty].layers[GAS], POISON_GAS);
    }

    test_teardown_game();
}

TEST(test_advance_environment_runs) {
    test_init_game(12345);

    // Just verify it doesn't crash
    test_advance_environment(5);
    ASSERT(!rogue.gameHasEnded);

    test_teardown_game();
}

TEST(test_advance_environment_spreads_fire) {
    test_init_game(12345);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 5);
    test_remove_all_monsters();

    // Place grass in a 3x3 area
    for (short x = cx + 2; x <= cx + 4; x++) {
        for (short y = cy; y <= cy + 2; y++) {
            if (x >= 0 && x < DCOLS && y >= 0 && y < DROWS) {
                test_set_terrain(x, y, SURFACE, GRASS);
            }
        }
    }

    // Ignite center grass using the proper fire exposure system
    short fireX = cx + 3;
    short fireY = cy + 1;
    if (fireX < DCOLS && fireY < DROWS) {
        exposeTileToFire(fireX, fireY, true); // alwaysIgnite = true

        // Advance environment to let fire spread to neighbors
        test_advance_environment(20);

        // At least one neighboring cell should have been affected
        // (grass burned, fire appeared, or surface changed)
        boolean fireSpread = false;
        for (short dx = -1; dx <= 1; dx++) {
            for (short dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                short nx = fireX + dx;
                short ny = fireY + dy;
                if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS) {
                    if (pmap[nx][ny].layers[SURFACE] != GRASS
                        || pmap[nx][ny].layers[GAS] != NOTHING) {
                        fireSpread = true;
                    }
                }
            }
        }
        ASSERT(fireSpread);
    }

    test_teardown_game();
}

TEST(test_init_teardown_stress) {
    // Run many init/teardown cycles to catch memory leaks or corruption
    for (int i = 0; i < 50; i++) {
        test_init_game(10000 + i);
        ASSERT(rogue.gameInProgress);
        ASSERT_GT(player.currentHP, 0);
        ASSERT_GT(test_count_monsters(), 0);
        test_rest(); // exercise the turn system briefly
        test_teardown_game();
    }
}

TEST(test_helpers_compose) {
    // Test that multiple helpers work together in a realistic scenario
    test_init_game(12345);

    // Set up a controlled arena
    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 4);
    test_remove_all_monsters();

    // Verify clean slate
    ASSERT_EQ(test_count_monsters(), 0);

    // Place a monster, give player a weapon, teleport near it
    pos spot;
    ASSERT(test_find_open_floor(cx + 2, cy, &spot));
    creature *rat = test_place_monster(MK_RAT, spot.x, spot.y);
    ASSERT(rat != NULL);
    ASSERT_EQ(test_count_monsters(), 1);

    item *sword = test_give_item(WEAPON, SWORD, 5);
    ASSERT(sword != NULL);

    test_set_player_hp(100, 100);
    ASSERT_EQ(player.currentHP, 100);

    // Rest a turn — game should still be running
    test_rest();
    ASSERT(!rogue.gameHasEnded);

    test_teardown_game();
}

SUITE(infrastructure) {
    RUN_TEST(test_clear_area_creates_open_floor);
    RUN_TEST(test_clear_area_removes_monsters);
    RUN_TEST(test_remove_all_monsters_empties_level);
    RUN_TEST(test_set_terrain_places_tile);
    RUN_TEST(test_set_terrain_gas_layer);
    RUN_TEST(test_advance_environment_runs);
    RUN_TEST(test_advance_environment_spreads_fire);
    RUN_TEST(test_init_teardown_stress);
    RUN_TEST(test_helpers_compose);
}
