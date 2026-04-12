// test_fire_gas.c — Tests for fire spread and gas dissipation mechanics

#include "test_harness.h"

TEST(test_fire_on_grass_spreads) {
    test_init_game(12345);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 6);
    test_remove_all_monsters();

    // Place grass in a 5x5 area offset from the player
    for (short x = cx + 2; x <= cx + 6; x++) {
        for (short y = cy - 2; y <= cy + 2; y++) {
            if (x >= 0 && x < DCOLS && y >= 0 && y < DROWS) {
                test_set_terrain(x, y, SURFACE, GRASS);
            }
        }
    }

    // Ignite center of the grass patch
    short fireX = cx + 4;
    short fireY = cy;
    if (fireX < DCOLS && fireY < DROWS) {
        exposeTileToFire(fireX, fireY, true);

        // Advance environment to let fire spread
        test_advance_environment(30);

        // Check that fire spread to at least two neighboring cells
        // (grass burned away, fire appeared, or gas layer changed)
        int affectedNeighbors = 0;
        for (short dx = -1; dx <= 1; dx++) {
            for (short dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                short nx = fireX + dx;
                short ny = fireY + dy;
                if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS) {
                    if (pmap[nx][ny].layers[SURFACE] != GRASS
                        || pmap[nx][ny].layers[GAS] != NOTHING
                        || cellHasTerrainFlag((pos){ nx, ny }, T_IS_FIRE)) {
                        affectedNeighbors++;
                    }
                }
            }
        }
        ASSERT_GE(affectedNeighbors, 2);
    }

    test_teardown_game();
}

TEST(test_gas_dissipates_over_time) {
    test_init_game(99999);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 5);
    test_remove_all_monsters();

    // Place poison gas on a cell away from the player
    short gx = cx + 3;
    short gy = cy;
    if (gx < DCOLS && gy < DROWS) {
        test_set_terrain(gx, gy, GAS, POISON_GAS);
        pmap[gx][gy].volume = 1000;

        ASSERT_EQ(pmap[gx][gy].layers[GAS], POISON_GAS);
        ASSERT_EQ(pmap[gx][gy].volume, 1000);

        // Advance environment many ticks to let gas dissipate
        test_advance_environment(100);

        // Gas volume should have decreased or gas should be gone entirely
        boolean dissipated = (pmap[gx][gy].volume < 1000
                              || pmap[gx][gy].layers[GAS] == NOTHING);
        ASSERT(dissipated);
    }

    test_teardown_game();
}

TEST(test_fire_does_not_spread_to_stone) {
    test_init_game(54321);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 5);
    test_remove_all_monsters();

    // Ensure all SURFACE layers in the area are NOTHING (no flammable material)
    for (short x = cx + 2; x <= cx + 4; x++) {
        for (short y = cy - 1; y <= cy + 1; y++) {
            if (x >= 0 && x < DCOLS && y >= 0 && y < DROWS) {
                test_set_terrain(x, y, SURFACE, NOTHING);
            }
        }
    }

    // Ignite the center cell (bare floor, no fuel)
    short fireX = cx + 3;
    short fireY = cy;
    if (fireX < DCOLS && fireY < DROWS) {
        exposeTileToFire(fireX, fireY, true);

        test_advance_environment(20);

        // No adjacent cell should have fire — there's nothing to burn
        for (short dx = -1; dx <= 1; dx++) {
            for (short dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                short nx = fireX + dx;
                short ny = fireY + dy;
                if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS) {
                    ASSERT(!cellHasTerrainFlag((pos){ nx, ny }, T_IS_FIRE));
                }
            }
        }
    }

    test_teardown_game();
}

TEST(test_fire_ignites_dead_grass) {
    test_init_game(77777);

    short cx = player.loc.x;
    short cy = player.loc.y;
    test_clear_area(cx, cy, 6);
    test_remove_all_monsters();

    // Place dead grass (also flammable) in a line
    for (short x = cx + 2; x <= cx + 5; x++) {
        if (x >= 0 && x < DCOLS) {
            test_set_terrain(x, cy, SURFACE, DEAD_GRASS);
        }
    }

    // Ignite one end of the line
    short fireX = cx + 2;
    if (fireX < DCOLS) {
        exposeTileToFire(fireX, cy, true);

        test_advance_environment(30);

        // At least one other dead grass cell should have been affected
        boolean affected = false;
        for (short x = cx + 3; x <= cx + 5; x++) {
            if (x >= 0 && x < DCOLS) {
                if (pmap[x][cy].layers[SURFACE] != DEAD_GRASS
                    || cellHasTerrainFlag((pos){ x, cy }, T_IS_FIRE)) {
                    affected = true;
                }
            }
        }
        ASSERT(affected);
    }

    test_teardown_game();
}

TEST(test_environment_update_does_not_crash) {
    test_init_game(31415);

    // Run many environment ticks on a default level — stress test
    test_advance_environment(100);
    ASSERT(!rogue.gameHasEnded);

    // Also try on a second seed for variety
    test_teardown_game();
    test_init_game(27182);
    test_advance_environment(100);
    ASSERT(!rogue.gameHasEnded);

    test_teardown_game();
}

SUITE(fire_gas) {
    RUN_TEST(test_fire_on_grass_spreads);
    RUN_TEST(test_gas_dissipates_over_time);
    RUN_TEST(test_fire_does_not_spread_to_stone);
    RUN_TEST(test_fire_ignites_dead_grass);
    RUN_TEST(test_environment_update_does_not_crash);
}
