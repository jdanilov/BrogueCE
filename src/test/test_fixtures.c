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
    test_init_game(42);

    rogue.depthLevel = 3;

    // Try to build the fountain (retry loop because area placement can fail)
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

SUITE(fixtures) {
    RUN_TEST(test_fixture_fountain_blueprint_depth_range);
    RUN_TEST(test_fixture_fountain_blueprint_has_features);
    RUN_TEST(test_fixture_fountain_places_statue);
    RUN_TEST(test_fixture_fountain_blueprint_has_statue_feature);
}
