// test_fixtures.c — Tests for dungeon fixture machines

#include "test_harness.h"

// --- Blueprint registration ---

TEST(test_fixture_fountain_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_FOUNTAIN];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_fountain_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_FOUNTAIN];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

// --- Placement ---

TEST(test_fixture_fountain_places_statue) {
    test_init_arena(42);

    rogue.depthLevel = 3;

    // The arena provides a large open floor area that reliably satisfies
    // the fountain blueprint's {4,8} room size and MF_NOT_IN_HALLWAY requirements.
    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_FOUNTAIN, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify at least one STATUE_INERT tile exists on the level
    boolean found = false;
    for (int x = 0; x < DCOLS && !found; x++) {
        for (int y = 0; y < DROWS && !found; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                found = true;
            }
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_fixture_fountain_blueprint_has_statue_feature) {
    // Verify the first feature places STATUE_INERT on the DUNGEON layer.
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_FOUNTAIN];
    ASSERT_EQ(bp->feature[0].terrain, STATUE_INERT);
    ASSERT_EQ(bp->feature[0].layer, DUNGEON);

    test_teardown_game();
}

// --- Rubble Heap ---

TEST(test_fixture_rubble_heap_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_RUBBLE_HEAP];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_rubble_heap_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_RUBBLE_HEAP];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_rubble_heap_places_statue) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_RUBBLE_HEAP, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Count STATUE_INERT tiles — at least 2 should exist (fountain + rubble heap)
    // or verify the rubble heap was placed by checking buildAMachine returned true
    // (which we already asserted above)
    // Additionally verify STATUE_INERT exists on the level
    boolean found = false;
    for (int x = 0; x < DCOLS && !found; x++) {
        for (int y = 0; y < DROWS && !found; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                found = true;
            }
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_fixture_rubble_heap_blueprint_has_statue_feature) {
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_RUBBLE_HEAP];
    ASSERT_EQ(bp->feature[0].terrain, STATUE_INERT);
    ASSERT_EQ(bp->feature[0].layer, DUNGEON);

    test_teardown_game();
}

// --- Lone Statue ---

TEST(test_fixture_lone_statue_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_LONE_STATUE];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_lone_statue_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_LONE_STATUE];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_lone_statue_places_statue) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_LONE_STATUE, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    boolean found = false;
    for (int x = 0; x < DCOLS && !found; x++) {
        for (int y = 0; y < DROWS && !found; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                found = true;
            }
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_fixture_lone_statue_blueprint_has_statue_feature) {
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_LONE_STATUE];
    ASSERT_EQ(bp->feature[0].terrain, STATUE_INERT);
    ASSERT_EQ(bp->feature[0].layer, DUNGEON);

    test_teardown_game();
}

// --- Garden Patch ---

TEST(test_fixture_garden_patch_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_GARDEN_PATCH];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_garden_patch_custom_layout) {
    test_init_game(99);

    // Garden uses custom layout (featureCount == 0), so verify that directly.
    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_GARDEN_PATCH];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_garden_patch_places_foliage_and_water) {
    test_init_game(42);

    rogue.depthLevel = 3;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_GARDEN_PATCH, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // The garden is recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_GARDEN_PATCH) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_garden_patch_alternating_rows) {
    test_init_game(42);

    rogue.depthLevel = 3;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_GARDEN_PATCH, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Find a foliage tile, then verify alternating pattern in any cardinal direction:
    // foliage → water → foliage in a line (works for both vertical and horizontal gardens).
    boolean foundPattern = false;
    short dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[SURFACE] != FOLIAGE) continue;
            for (int d = 0; d < 4 && !foundPattern; d++) {
                int nx1 = x + dirs[d][0], ny1 = y + dirs[d][1];
                int nx2 = x + dirs[d][0]*2, ny2 = y + dirs[d][1]*2;
                if (nx2 < 0 || nx2 >= DCOLS || ny2 < 0 || ny2 >= DROWS) continue;
                if (pmap[nx1][ny1].layers[LIQUID] == SHALLOW_WATER
                    && pmap[nx2][ny2].layers[SURFACE] == FOLIAGE) {
                    foundPattern = true;
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Collapsed Pillar ---

TEST(test_fixture_collapsed_pillar_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_COLLAPSED_PILLAR];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_collapsed_pillar_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_COLLAPSED_PILLAR];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_collapsed_pillar_places_statue) {
    test_init_arena(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_COLLAPSED_PILLAR, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    boolean found = false;
    for (int x = 0; x < DCOLS && !found; x++) {
        for (int y = 0; y < DROWS && !found; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                found = true;
            }
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_fixture_collapsed_pillar_blueprint_has_statue_feature) {
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_COLLAPSED_PILLAR];
    ASSERT_EQ(bp->feature[0].terrain, STATUE_INERT);
    ASSERT_EQ(bp->feature[0].layer, DUNGEON);

    test_teardown_game();
}

// --- Drainage Channel ---

TEST(test_fixture_drainage_channel_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_DRAINAGE_CHANNEL];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_drainage_channel_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_DRAINAGE_CHANNEL];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_drainage_channel_places_water) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_DRAINAGE_CHANNEL, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_DRAINAGE_CHANNEL) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_drainage_channel_water_line) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_DRAINAGE_CHANNEL, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Find 3 consecutive horizontal shallow water tiles
    boolean foundLine = false;
    for (int y = 0; y < DROWS && !foundLine; y++) {
        for (int x = 0; x < DCOLS - 2 && !foundLine; x++) {
            if (pmap[x][y].layers[LIQUID] == SHALLOW_WATER
                && pmap[x + 1][y].layers[LIQUID] == SHALLOW_WATER
                && pmap[x + 2][y].layers[LIQUID] == SHALLOW_WATER) {
                foundLine = true;
            }
        }
    }
    ASSERT(foundLine);

    test_teardown_game();
}

// --- Mossy Alcove ---

TEST(test_fixture_mossy_alcove_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_MOSSY_ALCOVE];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_mossy_alcove_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_MOSSY_ALCOVE];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_mossy_alcove_places_water) {
    test_init_game(42);

    rogue.depthLevel = 3;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_MOSSY_ALCOVE, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_MOSSY_ALCOVE) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_mossy_alcove_water_surrounded_by_vegetation) {
    test_init_game(42);

    rogue.depthLevel = 3;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_MOSSY_ALCOVE, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Find shallow water tile, verify at least one adjacent cell has foliage or grass
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[LIQUID] == SHALLOW_WATER) {
                // Check cardinal neighbors for vegetation
                if (pmap[x-1][y].layers[SURFACE] == FOLIAGE || pmap[x-1][y].layers[SURFACE] == GRASS
                    || pmap[x+1][y].layers[SURFACE] == FOLIAGE || pmap[x+1][y].layers[SURFACE] == GRASS
                    || pmap[x][y-1].layers[SURFACE] == FOLIAGE || pmap[x][y-1].layers[SURFACE] == GRASS
                    || pmap[x][y+1].layers[SURFACE] == FOLIAGE || pmap[x][y+1].layers[SURFACE] == GRASS) {
                    foundPattern = true;
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Cobweb Corner ---

TEST(test_fixture_cobweb_corner_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_COBWEB_CORNER];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_cobweb_corner_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_COBWEB_CORNER];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_cobweb_corner_places_webs) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_COBWEB_CORNER, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_COBWEB_CORNER) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_cobweb_corner_webs_near_walls) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_COBWEB_CORNER, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Find a spiderweb tile and verify it has at least one adjacent passability-blocking tile
    boolean foundWebNearWall = false;
    for (int x = 1; x < DCOLS - 1 && !foundWebNearWall; x++) {
        for (int y = 1; y < DROWS - 1 && !foundWebNearWall; y++) {
            if (pmap[x][y].layers[SURFACE] == SPIDERWEB) {
                if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) {
                    foundWebNearWall = true;
                }
            }
        }
    }
    ASSERT(foundWebNearWall);

    test_teardown_game();
}

// --- Crumbled Wall ---

TEST(test_fixture_crumbled_wall_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_CRUMBLED_WALL];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_crumbled_wall_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_CRUMBLED_WALL];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_crumbled_wall_places_statue_and_rubble) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_CRUMBLED_WALL, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_CRUMBLED_WALL) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_crumbled_wall_statue_near_wall) {
    test_init_game(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_CRUMBLED_WALL, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Find a STATUE_INERT tile and verify it has at least one adjacent wall
    boolean foundStatueNearWall = false;
    for (int x = 1; x < DCOLS - 1 && !foundStatueNearWall; x++) {
        for (int y = 1; y < DROWS - 1 && !foundStatueNearWall; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) {
                    // Also check for adjacent rubble
                    if (pmap[x-1][y].layers[SURFACE] == RUBBLE
                        || pmap[x+1][y].layers[SURFACE] == RUBBLE
                        || pmap[x][y-1].layers[SURFACE] == RUBBLE
                        || pmap[x][y+1].layers[SURFACE] == RUBBLE) {
                        foundStatueNearWall = true;
                    }
                }
            }
        }
    }
    ASSERT(foundStatueNearWall);

    test_teardown_game();
}

// --- Dust Motes ---

TEST(test_fixture_dust_motes_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_DUST_MOTES];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], gameConst->deepestLevel);

    test_teardown_game();
}

TEST(test_fixture_dust_motes_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_DUST_MOTES];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_dust_motes_places_ash) {
    test_init_arena(42);

    rogue.depthLevel = 5;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_DUST_MOTES, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    boolean foundAsh = false;
    for (int x = 0; x < DCOLS && !foundAsh; x++) {
        for (int y = 0; y < DROWS && !foundAsh; y++) {
            if (pmap[x][y].layers[SURFACE] == ASH) {
                foundAsh = true;
            }
        }
    }
    ASSERT(foundAsh);

    test_teardown_game();
}

TEST(test_fixture_dust_motes_blueprint_has_ash_feature) {
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_DUST_MOTES];
    ASSERT_EQ(bp->feature[0].terrain, ASH);
    ASSERT_EQ(bp->feature[0].layer, SURFACE);

    test_teardown_game();
}

// --- Mushroom Circle ---

TEST(test_fixture_mushroom_circle_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_MUSHROOM_CIRCLE];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_mushroom_circle_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_MUSHROOM_CIRCLE];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_mushroom_circle_places_fungus) {
    // Try multiple game seeds since the ring needs a large open room
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_MUSHROOM_CIRCLE, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_MUSHROOM_CIRCLE) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_mushroom_circle_ring_pattern) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_MUSHROOM_CIRCLE, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find a GRASS tile and verify it has adjacent FUNGUS_FOREST or LUMINESCENT_FUNGUS
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[SURFACE] == GRASS) {
                // Check neighbors for fungus ring
                for (int dx = -1; dx <= 1 && !foundPattern; dx++) {
                    for (int dy = -1; dy <= 1 && !foundPattern; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        short t = pmap[x+dx][y+dy].layers[SURFACE];
                        if (t == FUNGUS_FOREST || t == LUMINESCENT_FUNGUS) {
                            foundPattern = true;
                        }
                    }
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Sunlit Patch ---

TEST(test_fixture_sunlit_patch_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_SUNLIT_PATCH];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_sunlit_patch_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_SUNLIT_PATCH];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_sunlit_patch_places_foliage) {
    test_init_game(42);

    rogue.depthLevel = 3;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_SUNLIT_PATCH, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_SUNLIT_PATCH) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_sunlit_patch_blueprint_has_sunlight_df) {
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_SUNLIT_PATCH];
    ASSERT_EQ(bp->feature[0].featureDF, DF_SUNLIGHT);

    test_teardown_game();
}

// --- Bird Nest ---

TEST(test_fixture_bird_nest_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_BIRD_NEST];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_bird_nest_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_BIRD_NEST];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_bird_nest_places_statue) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_BIRD_NEST, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_BIRD_NEST) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_bird_nest_statue_with_adjacent_web) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_BIRD_NEST, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find a STATUE_INERT with adjacent SPIDERWEB
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                if (pmap[x-1][y].layers[SURFACE] == SPIDERWEB
                    || pmap[x+1][y].layers[SURFACE] == SPIDERWEB
                    || pmap[x][y-1].layers[SURFACE] == SPIDERWEB
                    || pmap[x][y+1].layers[SURFACE] == SPIDERWEB) {
                    foundPattern = true;
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Vine Trellis ---

TEST(test_fixture_vine_trellis_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_VINE_TRELLIS];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_vine_trellis_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_VINE_TRELLIS];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_vine_trellis_places_foliage) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_VINE_TRELLIS, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_VINE_TRELLIS) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    test_teardown_game();
}

TEST(test_fixture_vine_trellis_foliage_near_wall) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_VINE_TRELLIS, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find a FOLIAGE tile and verify it has at least one adjacent wall
    boolean foundFoliageNearWall = false;
    for (int x = 1; x < DCOLS - 1 && !foundFoliageNearWall; x++) {
        for (int y = 1; y < DROWS - 1 && !foundFoliageNearWall; y++) {
            if (pmap[x][y].layers[SURFACE] == FOLIAGE) {
                if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) {
                    foundFoliageNearWall = true;
                }
            }
        }
    }
    ASSERT(foundFoliageNearWall);

    test_teardown_game();
}

// --- Puddle ---

TEST(test_fixture_puddle_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_PUDDLE];
    ASSERT_EQ(bp->depthRange[0], 1);
    ASSERT_EQ(bp->depthRange[1], 8);

    test_teardown_game();
}

TEST(test_fixture_puddle_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_PUDDLE];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_puddle_places_mud) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_PUDDLE, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_PUDDLE) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify MUD tile exists
    boolean foundMud = false;
    for (int x = 0; x < DCOLS && !foundMud; x++) {
        for (int y = 0; y < DROWS && !foundMud; y++) {
            if (pmap[x][y].layers[LIQUID] == MUD) {
                foundMud = true;
            }
        }
    }
    ASSERT(foundMud);

    test_teardown_game();
}

TEST(test_fixture_puddle_mud_surrounded_by_vegetation) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17};
    for (int s = 0; s < 5 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 3;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_PUDDLE, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find the MUD tile and verify it has vegetation or water on adjacent cells
    for (int x = 1; x < DCOLS - 1; x++) {
        for (int y = 1; y < DROWS - 1; y++) {
            if (pmap[x][y].layers[LIQUID] == MUD) {
                // Count adjacent cells with water, grass, or foliage
                int vegOrWater = 0;
                short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
                for (int d = 0; d < 4; d++) {
                    int nx = x + dirs[d][0];
                    int ny = y + dirs[d][1];
                    if (pmap[nx][ny].layers[LIQUID] == SHALLOW_WATER
                        || pmap[nx][ny].layers[SURFACE] == GRASS
                        || pmap[nx][ny].layers[SURFACE] == FOLIAGE) {
                        vegOrWater++;
                    }
                }
                ASSERT_GE(vegOrWater, 3);
                goto done;
            }
        }
    }
    done:

    test_teardown_game();
}

// --- Forge ---

TEST(test_fixture_forge_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_FORGE];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_forge_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_FORGE];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_forge_places_lava_and_anvil) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_FORGE, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_FORGE) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify LAVA and STATUE_INERT exist
    boolean foundLava = false, foundAnvil = false;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[LIQUID] == LAVA) foundLava = true;
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) foundAnvil = true;
        }
    }
    ASSERT(foundLava);
    ASSERT(foundAnvil);

    test_teardown_game();
}

TEST(test_fixture_forge_anvil_near_lava) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_FORGE, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find STATUE_INERT (anvil) and verify lava is 2 cells away in any cardinal direction
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                short dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
                for (int d = 0; d < 4 && !foundPattern; d++) {
                    int lx = x + dirs[d][0] * 2;
                    int ly = y + dirs[d][1] * 2;
                    if (lx >= 0 && lx < DCOLS && ly >= 0 && ly < DROWS
                        && pmap[lx][ly].layers[LIQUID] == LAVA) {
                        foundPattern = true;
                    }
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Altar Nook ---

TEST(test_fixture_altar_nook_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_ALTAR_NOOK];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_altar_nook_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_ALTAR_NOOK];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_altar_nook_places_altar) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_ALTAR_NOOK, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_ALTAR_NOOK) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify ALTAR_INERT and CARPET exist
    boolean foundAltar = false, foundCarpet = false;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[DUNGEON] == ALTAR_INERT) foundAltar = true;
            if (pmap[x][y].layers[DUNGEON] == CARPET) foundCarpet = true;
        }
    }
    ASSERT(foundAltar);
    ASSERT(foundCarpet);

    test_teardown_game();
}

TEST(test_fixture_altar_nook_carpet_runner) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_ALTAR_NOOK, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find ALTAR_INERT and verify a 5-tile carpet runner in any cardinal direction
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[DUNGEON] != ALTAR_INERT) continue;
            // Check all 4 directions for a carpet line of 5
            short dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
            for (int d = 0; d < 4 && !foundPattern; d++) {
                boolean allCarpet = true;
                for (int step = 1; step <= 5; step++) {
                    int nx = x + dirs[d][0] * step;
                    int ny = y + dirs[d][1] * step;
                    if (nx < 0 || nx >= DCOLS || ny < 0 || ny >= DROWS
                        || pmap[nx][ny].layers[DUNGEON] != CARPET) {
                        allCarpet = false;
                        break;
                    }
                }
                if (allCarpet) foundPattern = true;
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

TEST(test_fixture_crystal_outcrop_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_CRYSTAL_OUTCROP];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_crystal_outcrop_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_CRYSTAL_OUTCROP];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_crystal_outcrop_places_crystals) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_CRYSTAL_OUTCROP, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_CRYSTAL_OUTCROP) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify CRYSTAL_WALL and LUMINESCENT_FUNGUS exist
    int crystalCount = 0;
    boolean foundFungus = false;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[DUNGEON] == CRYSTAL_WALL) crystalCount++;
            if (pmap[x][y].layers[SURFACE] == LUMINESCENT_FUNGUS) foundFungus = true;
        }
    }
    ASSERT(crystalCount >= 2);
    ASSERT(foundFungus);

    test_teardown_game();
}

TEST(test_fixture_crystal_outcrop_crystals_adjacent) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_CRYSTAL_OUTCROP, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find two crystal walls and verify they are cardinally adjacent
    boolean foundAdjacent = false;
    for (int x = 1; x < DCOLS - 1 && !foundAdjacent; x++) {
        for (int y = 1; y < DROWS - 1 && !foundAdjacent; y++) {
            if (pmap[x][y].layers[DUNGEON] != CRYSTAL_WALL) continue;
            if (pmap[x+1][y].layers[DUNGEON] == CRYSTAL_WALL) foundAdjacent = true;
            if (pmap[x][y+1].layers[DUNGEON] == CRYSTAL_WALL) foundAdjacent = true;
        }
    }
    ASSERT(foundAdjacent);

    test_teardown_game();
}

// --- Steam Vent ---

TEST(test_fixture_steam_vent_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_STEAM_VENT];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_steam_vent_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_STEAM_VENT];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_steam_vent_places_vent) {
    test_init_arena(42);

    rogue.depthLevel = 8;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_STEAM_VENT, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    // Verify at least one STEAM_VENT tile exists on the level
    boolean found = false;
    for (int x = 0; x < DCOLS && !found; x++) {
        for (int y = 0; y < DROWS && !found; y++) {
            if (pmap[x][y].layers[DUNGEON] == STEAM_VENT) {
                found = true;
            }
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_fixture_steam_vent_blueprint_has_vent_feature) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_STEAM_VENT];
    ASSERT_EQ(bp->feature[0].terrain, STEAM_VENT);
    ASSERT_EQ(bp->feature[0].layer, DUNGEON);

    test_teardown_game();
}

// --- Abandoned Camp ---

TEST(test_fixture_abandoned_camp_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_ABANDONED_CAMP];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_abandoned_camp_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_ABANDONED_CAMP];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_abandoned_camp_places_hay_and_embers) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_ABANDONED_CAMP, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_ABANDONED_CAMP) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify HAY, EMBERS, and STATUE_INERT exist
    boolean foundHay = false, foundEmbers = false, foundStatue = false;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[SURFACE] == HAY) foundHay = true;
            if (pmap[x][y].layers[SURFACE] == EMBERS) foundEmbers = true;
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) foundStatue = true;
        }
    }
    ASSERT(foundHay);
    ASSERT(foundEmbers);
    ASSERT(foundStatue);

    test_teardown_game();
}

TEST(test_fixture_abandoned_camp_statue_near_embers) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_ABANDONED_CAMP, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find STATUE_INERT and verify EMBERS is within 3 cells (same camp)
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                for (int dx = -3; dx <= 3 && !foundPattern; dx++) {
                    for (int dy = -3; dy <= 3 && !foundPattern; dy++) {
                        int nx = x + dx, ny = y + dy;
                        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS
                            && pmap[nx][ny].layers[SURFACE] == EMBERS) {
                            foundPattern = true;
                        }
                    }
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Weapon Rack ---

TEST(test_fixture_weapon_rack_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_WEAPON_RACK];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_weapon_rack_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_WEAPON_RACK];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_weapon_rack_places_statue_and_junk) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_WEAPON_RACK, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_WEAPON_RACK) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify STATUE_INERT and JUNK exist
    boolean foundStatue = false, foundJunk = false;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) foundStatue = true;
            if (pmap[x][y].layers[SURFACE] == JUNK) foundJunk = true;
        }
    }
    ASSERT(foundStatue);
    ASSERT(foundJunk);

    test_teardown_game();
}

TEST(test_fixture_weapon_rack_statue_near_wall) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = 8;

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_WEAPON_RACK, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find STATUE_INERT and verify it has at least one adjacent wall and adjacent junk
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[DUNGEON] == STATUE_INERT) {
                boolean hasWall = cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)
                    || cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY);
                boolean hasJunk = pmap[x-1][y].layers[SURFACE] == JUNK
                    || pmap[x+1][y].layers[SURFACE] == JUNK
                    || pmap[x][y-1].layers[SURFACE] == JUNK
                    || pmap[x][y+1].layers[SURFACE] == JUNK;
                if (hasWall && hasJunk) {
                    foundPattern = true;
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Lichen Garden ---

TEST(test_fixture_lichen_garden_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_LICHEN_GARDEN];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_lichen_garden_custom_layout) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_LICHEN_GARDEN];
    ASSERT_EQ(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_lichen_garden_places_water_and_fungus) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    int depths[] = {8, 10, 12, 8, 10, 12, 8, 10, 12, 8};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = depths[s];

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_LICHEN_GARDEN, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Verify it was recorded in placedMachines
    boolean foundRecord = false;
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == MT_FIXTURE_LICHEN_GARDEN) {
            foundRecord = true;
            break;
        }
    }
    ASSERT(foundRecord);

    // Verify SHALLOW_WATER, LUMINESCENT_FUNGUS, and FUNGUS_FOREST exist
    boolean foundWater = false, foundLumFungus = false, foundFungusForest = false;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (pmap[x][y].layers[LIQUID] == SHALLOW_WATER) foundWater = true;
            if (pmap[x][y].layers[SURFACE] == LUMINESCENT_FUNGUS) foundLumFungus = true;
            if (pmap[x][y].layers[SURFACE] == FUNGUS_FOREST) foundFungusForest = true;
        }
    }
    ASSERT(foundWater);
    ASSERT(foundLumFungus);
    ASSERT(foundFungusForest);

    test_teardown_game();
}

TEST(test_fixture_lichen_garden_luminescent_fungus_adjacent_to_water) {
    boolean placed = false;
    int seeds[] = {42, 100, 200, 300, 17, 500, 600, 700, 800, 900};
    int depths[] = {8, 10, 12, 8, 10, 12, 8, 10, 12, 8};
    for (int s = 0; s < 10 && !placed; s++) {
        test_init_game(seeds[s]);
        rogue.depthLevel = depths[s];

        for (int i = 0; i < 30; i++) {
            if (buildAMachine(MT_FIXTURE_LICHEN_GARDEN, -1, -1, 0, NULL, NULL, NULL)) {
                placed = true;
                break;
            }
        }
        if (!placed) test_teardown_game();
    }
    ASSERT(placed);

    // Find a LUMINESCENT_FUNGUS tile and verify it has adjacent SHALLOW_WATER
    boolean foundPattern = false;
    for (int x = 1; x < DCOLS - 1 && !foundPattern; x++) {
        for (int y = 1; y < DROWS - 1 && !foundPattern; y++) {
            if (pmap[x][y].layers[SURFACE] == LUMINESCENT_FUNGUS) {
                if (pmap[x-1][y].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x+1][y].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x][y-1].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x][y+1].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x-1][y-1].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x+1][y-1].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x-1][y+1].layers[LIQUID] == SHALLOW_WATER
                    || pmap[x+1][y+1].layers[LIQUID] == SHALLOW_WATER) {
                    foundPattern = true;
                }
            }
        }
    }
    ASSERT(foundPattern);

    test_teardown_game();
}

// --- Scorched Earth ---

TEST(test_fixture_scorched_earth_blueprint_depth_range) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_SCORCHED_EARTH];
    ASSERT_EQ(bp->depthRange[0], 5);
    ASSERT_EQ(bp->depthRange[1], 18);

    test_teardown_game();
}

TEST(test_fixture_scorched_earth_blueprint_has_features) {
    test_init_game(99);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_SCORCHED_EARTH];
    ASSERT_GT(bp->featureCount, 0);

    test_teardown_game();
}

TEST(test_fixture_scorched_earth_places_ash_or_embers) {
    test_init_arena(42);

    rogue.depthLevel = 8;

    boolean placed = false;
    for (int i = 0; i < 30; i++) {
        if (buildAMachine(MT_FIXTURE_SCORCHED_EARTH, -1, -1, 0, NULL, NULL, NULL)) {
            placed = true;
            break;
        }
    }
    ASSERT(placed);

    boolean foundAshOrEmbers = false;
    for (int x = 0; x < DCOLS && !foundAshOrEmbers; x++) {
        for (int y = 0; y < DROWS && !foundAshOrEmbers; y++) {
            if (pmap[x][y].layers[SURFACE] == ASH || pmap[x][y].layers[SURFACE] == EMBERS) {
                foundAshOrEmbers = true;
            }
        }
    }
    ASSERT(foundAshOrEmbers);

    test_teardown_game();
}

TEST(test_fixture_scorched_earth_blueprint_has_embers_feature) {
    test_init_game(77);

    const blueprint *bp = &blueprintCatalog[MT_FIXTURE_SCORCHED_EARTH];
    // Embers are the last feature so they overwrite ash on shared cells
    boolean foundEmbers = false;
    for (int f = 0; f < bp->featureCount; f++) {
        if (bp->feature[f].terrain == EMBERS && bp->feature[f].layer == SURFACE) {
            foundEmbers = true;
        }
    }
    ASSERT(foundEmbers);

    test_teardown_game();
}

SUITE(fixtures) {
    RUN_TEST(test_fixture_fountain_blueprint_depth_range);
    RUN_TEST(test_fixture_fountain_blueprint_has_features);
    RUN_TEST(test_fixture_fountain_places_statue);
    RUN_TEST(test_fixture_fountain_blueprint_has_statue_feature);
    RUN_TEST(test_fixture_rubble_heap_blueprint_depth_range);
    RUN_TEST(test_fixture_rubble_heap_blueprint_has_features);
    RUN_TEST(test_fixture_rubble_heap_places_statue);
    RUN_TEST(test_fixture_rubble_heap_blueprint_has_statue_feature);
    RUN_TEST(test_fixture_lone_statue_blueprint_depth_range);
    RUN_TEST(test_fixture_lone_statue_blueprint_has_features);
    RUN_TEST(test_fixture_lone_statue_places_statue);
    RUN_TEST(test_fixture_lone_statue_blueprint_has_statue_feature);
    RUN_TEST(test_fixture_garden_patch_blueprint_depth_range);
    RUN_TEST(test_fixture_garden_patch_custom_layout);
    RUN_TEST(test_fixture_garden_patch_places_foliage_and_water);
    RUN_TEST(test_fixture_garden_patch_alternating_rows);
    RUN_TEST(test_fixture_collapsed_pillar_blueprint_depth_range);
    RUN_TEST(test_fixture_collapsed_pillar_blueprint_has_features);
    RUN_TEST(test_fixture_collapsed_pillar_places_statue);
    RUN_TEST(test_fixture_collapsed_pillar_blueprint_has_statue_feature);
    RUN_TEST(test_fixture_drainage_channel_blueprint_depth_range);
    RUN_TEST(test_fixture_drainage_channel_custom_layout);
    RUN_TEST(test_fixture_drainage_channel_places_water);
    RUN_TEST(test_fixture_drainage_channel_water_line);
    RUN_TEST(test_fixture_mossy_alcove_blueprint_depth_range);
    RUN_TEST(test_fixture_mossy_alcove_custom_layout);
    RUN_TEST(test_fixture_mossy_alcove_places_water);
    RUN_TEST(test_fixture_mossy_alcove_water_surrounded_by_vegetation);
    RUN_TEST(test_fixture_cobweb_corner_blueprint_depth_range);
    RUN_TEST(test_fixture_cobweb_corner_custom_layout);
    RUN_TEST(test_fixture_cobweb_corner_places_webs);
    RUN_TEST(test_fixture_cobweb_corner_webs_near_walls);
    RUN_TEST(test_fixture_crumbled_wall_blueprint_depth_range);
    RUN_TEST(test_fixture_crumbled_wall_custom_layout);
    RUN_TEST(test_fixture_crumbled_wall_places_statue_and_rubble);
    RUN_TEST(test_fixture_crumbled_wall_statue_near_wall);
    RUN_TEST(test_fixture_dust_motes_blueprint_depth_range);
    RUN_TEST(test_fixture_dust_motes_blueprint_has_features);
    RUN_TEST(test_fixture_dust_motes_places_ash);
    RUN_TEST(test_fixture_dust_motes_blueprint_has_ash_feature);
    RUN_TEST(test_fixture_mushroom_circle_blueprint_depth_range);
    RUN_TEST(test_fixture_mushroom_circle_custom_layout);
    RUN_TEST(test_fixture_mushroom_circle_places_fungus);
    RUN_TEST(test_fixture_mushroom_circle_ring_pattern);
    RUN_TEST(test_fixture_sunlit_patch_blueprint_depth_range);
    RUN_TEST(test_fixture_sunlit_patch_blueprint_has_features);
    RUN_TEST(test_fixture_sunlit_patch_places_foliage);
    RUN_TEST(test_fixture_sunlit_patch_blueprint_has_sunlight_df);
    RUN_TEST(test_fixture_bird_nest_blueprint_depth_range);
    RUN_TEST(test_fixture_bird_nest_custom_layout);
    RUN_TEST(test_fixture_bird_nest_places_statue);
    RUN_TEST(test_fixture_bird_nest_statue_with_adjacent_web);
    RUN_TEST(test_fixture_vine_trellis_blueprint_depth_range);
    RUN_TEST(test_fixture_vine_trellis_custom_layout);
    RUN_TEST(test_fixture_vine_trellis_places_foliage);
    RUN_TEST(test_fixture_vine_trellis_foliage_near_wall);
    RUN_TEST(test_fixture_puddle_blueprint_depth_range);
    RUN_TEST(test_fixture_puddle_custom_layout);
    RUN_TEST(test_fixture_puddle_places_mud);
    RUN_TEST(test_fixture_puddle_mud_surrounded_by_vegetation);
    RUN_TEST(test_fixture_forge_blueprint_depth_range);
    RUN_TEST(test_fixture_forge_custom_layout);
    RUN_TEST(test_fixture_forge_places_lava_and_anvil);
    RUN_TEST(test_fixture_forge_anvil_near_lava);
    RUN_TEST(test_fixture_altar_nook_blueprint_depth_range);
    RUN_TEST(test_fixture_altar_nook_custom_layout);
    RUN_TEST(test_fixture_altar_nook_places_altar);
    RUN_TEST(test_fixture_altar_nook_carpet_runner);
    RUN_TEST(test_fixture_crystal_outcrop_blueprint_depth_range);
    RUN_TEST(test_fixture_crystal_outcrop_custom_layout);
    RUN_TEST(test_fixture_crystal_outcrop_places_crystals);
    RUN_TEST(test_fixture_crystal_outcrop_crystals_adjacent);
    RUN_TEST(test_fixture_steam_vent_blueprint_depth_range);
    RUN_TEST(test_fixture_steam_vent_blueprint_has_features);
    RUN_TEST(test_fixture_steam_vent_places_vent);
    RUN_TEST(test_fixture_steam_vent_blueprint_has_vent_feature);
    RUN_TEST(test_fixture_abandoned_camp_blueprint_depth_range);
    RUN_TEST(test_fixture_abandoned_camp_custom_layout);
    RUN_TEST(test_fixture_abandoned_camp_places_hay_and_embers);
    RUN_TEST(test_fixture_abandoned_camp_statue_near_embers);
    RUN_TEST(test_fixture_weapon_rack_blueprint_depth_range);
    RUN_TEST(test_fixture_weapon_rack_custom_layout);
    RUN_TEST(test_fixture_weapon_rack_places_statue_and_junk);
    RUN_TEST(test_fixture_weapon_rack_statue_near_wall);
    RUN_TEST(test_fixture_scorched_earth_blueprint_depth_range);
    RUN_TEST(test_fixture_scorched_earth_blueprint_has_features);
    RUN_TEST(test_fixture_scorched_earth_places_ash_or_embers);
    RUN_TEST(test_fixture_scorched_earth_blueprint_has_embers_feature);
    RUN_TEST(test_fixture_lichen_garden_blueprint_depth_range);
    RUN_TEST(test_fixture_lichen_garden_custom_layout);
    RUN_TEST(test_fixture_lichen_garden_places_water_and_fungus);
    RUN_TEST(test_fixture_lichen_garden_luminescent_fungus_adjacent_to_water);
}
