// test_vision.c — Tests for vision, discovery, and telepathy

#include "test_harness.h"

TEST(test_cells_near_player_are_visible) {
    test_init_game(12345);

    // After game init, cells adjacent to the player should mostly be visible.
    // Check all 8 neighbors; at least some passable ones should have VISIBLE.
    int visibleCount = 0;
    int passableCount = 0;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS) {
            if (!cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)) {
                passableCount++;
                if (pmap[nx][ny].flags & VISIBLE) {
                    visibleCount++;
                }
            }
        }
    }

    // There should be at least one passable neighbor
    ASSERT_GT(passableCount, 0);
    // At least most passable adjacent cells should be visible
    ASSERT_GT(visibleCount, 0);
    ASSERT_GE(visibleCount, passableCount / 2);

    test_teardown_game();
}

TEST(test_cells_far_away_not_visible) {
    test_init_game(12345);

    // Check corners of the map. At least one corner should not be visible,
    // since dungeons are rarely so open that all corners are in view.
    short corners[][2] = {
        {1, 1},
        {DCOLS - 2, 1},
        {1, DROWS - 2},
        {DCOLS - 2, DROWS - 2}
    };

    int notVisibleCount = 0;
    for (int i = 0; i < 4; i++) {
        short cx = corners[i][0];
        short cy = corners[i][1];
        if (!(pmap[cx][cy].flags & VISIBLE)) {
            notVisibleCount++;
        }
    }

    // At least one far corner should not be visible
    ASSERT_GT(notVisibleCount, 0);

    test_teardown_game();
}

TEST(test_discovered_flag_persists) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();

    // After clearing and updating vision, adjacent cells should be DISCOVERED
    short checkX = player.loc.x + 1;
    short checkY = player.loc.y;
    ASSERT(pmap[checkX][checkY].flags & DISCOVERED);
    ASSERT(pmap[checkX][checkY].flags & VISIBLE);

    // Teleport the player far away
    short farX = (player.loc.x < DCOLS / 2) ? DCOLS - 5 : 5;
    short farY = (player.loc.y < DROWS / 2) ? DROWS - 5 : 5;
    test_clear_area(farX, farY, 3);
    test_teleport_player(farX, farY);

    // The original cell should still be DISCOVERED (it persists)
    ASSERT(pmap[checkX][checkY].flags & DISCOVERED);

    // But it should no longer be VISIBLE (player is far away)
    // This might not always hold in very open maps, but with walls it should
    ASSERT(!(pmap[checkX][checkY].flags & VISIBLE));

    test_teardown_game();
}

TEST(test_telepathy_reveals_monsters) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();

    // Place a wall between the player and a spot 3 tiles to the right
    short wallX = player.loc.x + 1;
    short wallY = player.loc.y;
    test_set_terrain(wallX, wallY, DUNGEON, WALL);

    // Place a monster behind the wall
    short monX = player.loc.x + 2;
    short monY = player.loc.y;
    creature *mon = test_place_monster(MK_RAT, monX, monY);
    ASSERT(mon != NULL);

    // Without telepathy, the monster should not be visible through the wall
    updateVision(true);
    boolean visibleBefore = playerCanSeeOrSense(monX, monY);

    // Give the player telepathy
    test_set_player_status(STATUS_TELEPATHIC, 100);
    updateVision(true);

    // With telepathy, the player should be able to sense the monster
    boolean visibleAfter = playerCanSeeOrSense(monX, monY);

    // The monster should not have been visible before (wall blocks LOS)
    ASSERT(!visibleBefore);
    // But should be sensed after telepathy is granted
    ASSERT(visibleAfter);

    test_teardown_game();
}

TEST(test_player_can_see_adjacent_cells) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 3);
    test_remove_all_monsters();

    // playerCanSee should return true for adjacent passable cells
    int seeCount = 0;
    int checked = 0;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)) {
            checked++;
            if (playerCanSee(nx, ny)) {
                seeCount++;
            }
        }
    }

    ASSERT_GT(checked, 0);
    // In a cleared area, all passable adjacent cells should be visible
    ASSERT_EQ(seeCount, checked);

    test_teardown_game();
}

SUITE(vision) {
    RUN_TEST(test_cells_near_player_are_visible);
    RUN_TEST(test_cells_far_away_not_visible);
    RUN_TEST(test_discovered_flag_persists);
    RUN_TEST(test_telepathy_reveals_monsters);
    RUN_TEST(test_player_can_see_adjacent_cells);
}
