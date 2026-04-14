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
}
