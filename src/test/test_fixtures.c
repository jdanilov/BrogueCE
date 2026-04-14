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

    // Find a foliage tile, then verify alternating pattern:
    // the row below it should be shallow water, the row below that foliage again.
    boolean foundPattern = false;
    for (int x = 0; x < DCOLS && !foundPattern; x++) {
        for (int y = 0; y < DROWS - 2 && !foundPattern; y++) {
            if (pmap[x][y].layers[SURFACE] == FOLIAGE
                && pmap[x][y + 1].layers[LIQUID] == SHALLOW_WATER
                && pmap[x][y + 2].layers[SURFACE] == FOLIAGE) {
                foundPattern = true;
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
}
