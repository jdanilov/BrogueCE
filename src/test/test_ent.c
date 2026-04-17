// test_ent.c — Tests for the ent monster and its foliage trail

#include "test_harness.h"

TEST(test_ent_catalog_entry) {
    // Verify the ent monster catalog has expected properties
    creatureType *ent = &monsterCatalog[MK_ENT];
    ASSERT_EQ(ent->monsterID, MK_ENT);
    ASSERT_EQ(ent->maxHP, 80);
    ASSERT_EQ(ent->defense, 40);
    ASSERT_EQ(ent->movementSpeed, 200);
    ASSERT_EQ(ent->attackSpeed, 200);
    ASSERT_EQ(ent->DFChance, 100);
    ASSERT_EQ(ent->DFType, DF_ENT_FOLIAGE);
    ASSERT(ent->flags & MONST_MAINTAINS_DISTANCE);
    ASSERT(ent->flags & MONST_CARRY_ITEM_100);
    ASSERT(ent->flags & MONST_FLEES_NEAR_DEATH);
    ASSERT(ent->abilityFlags & MA_CAST_SUMMON);
    ASSERT(ent->isLarge);

    test_init_arena(1);
    test_teardown_game();
}

TEST(test_ent_foliage_df_spawns_correct_terrain) {
    test_init_arena(42);

    // Pick a clear floor cell
    short x = player.loc.x + 5;
    short y = player.loc.y;
    ASSERT_EQ(pmap[x][y].layers[SURFACE], NOTHING);

    // Spawn the ent foliage DF
    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_ENT_FOLIAGE], true, false);
    ASSERT_EQ(pmap[x][y].layers[SURFACE], ENT_FOLIAGE);

    test_teardown_game();
}

TEST(test_ent_foliage_obstructs_vision) {
    test_init_arena(42);

    short x = player.loc.x + 3;
    short y = player.loc.y;

    // Place ent foliage
    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_ENT_FOLIAGE], true, false);

    // ENT_FOLIAGE should have T_OBSTRUCTS_VISION
    ASSERT(cellHasTerrainFlag((pos){x, y}, T_OBSTRUCTS_VISION));

    test_teardown_game();
}

TEST(test_ent_foliage_is_flammable) {
    test_init_arena(42);

    short x = player.loc.x + 5;
    short y = player.loc.y;

    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_ENT_FOLIAGE], true, false);

    // Should have T_IS_FLAMMABLE
    ASSERT(cellHasTerrainFlag((pos){x, y}, T_IS_FLAMMABLE));

    test_teardown_game();
}

TEST(test_ent_foliage_decays_to_fungus) {
    test_init_arena(42);
    test_reseed(42);

    short x = player.loc.x + 5;
    short y = player.loc.y;

    // Place ent foliage
    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_ENT_FOLIAGE], true, false);
    ASSERT_EQ(pmap[x][y].layers[SURFACE], ENT_FOLIAGE);

    // Advance many environment ticks — promoteChance=200 (2% per turn)
    for (int i = 0; i < 300 && pmap[x][y].layers[SURFACE] == ENT_FOLIAGE; i++) {
        test_advance_environment(1);
    }

    // Should have decayed to ENT_FUNGUS
    ASSERT_EQ(pmap[x][y].layers[SURFACE], ENT_FUNGUS);

    test_teardown_game();
}

TEST(test_ent_fungus_decays_to_nothing) {
    test_init_arena(42);
    test_reseed(42);

    short x = player.loc.x + 5;
    short y = player.loc.y;

    // Place ent fungus directly
    test_set_terrain(x, y, SURFACE, ENT_FUNGUS);
    ASSERT_EQ(pmap[x][y].layers[SURFACE], ENT_FUNGUS);

    // Advance many environment ticks
    for (int i = 0; i < 300 && pmap[x][y].layers[SURFACE] == ENT_FUNGUS; i++) {
        test_advance_environment(1);
    }

    // Should have decayed to NOTHING
    ASSERT_EQ(pmap[x][y].layers[SURFACE], NOTHING);

    test_teardown_game();
}

TEST(test_ent_foliage_no_promote_on_step) {
    test_init_arena(42);

    // Place ent foliage adjacent to player
    short x = player.loc.x + 1;
    short y = player.loc.y;
    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_ENT_FOLIAGE], true, false);
    ASSERT_EQ(pmap[x][y].layers[SURFACE], ENT_FOLIAGE);

    // Step onto it
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // The foliage should NOT have been trampled/promoted by stepping
        // (it should still be ENT_FOLIAGE, only decaying by timer)
        ASSERT_EQ(pmap[x][y].layers[SURFACE], ENT_FOLIAGE);
    }

    test_teardown_game();
}

TEST(test_ent_placement_and_df_trail) {
    test_init_arena(42);
    test_set_player_hp(500, 500);

    // Place an ent
    short mx = player.loc.x + 5;
    short my = player.loc.y;
    creature *ent = test_place_monster(MK_ENT, mx, my);
    ASSERT(ent != NULL);
    ASSERT_EQ(ent->info.monsterID, MK_ENT);
    ASSERT_EQ(ent->info.DFChance, 100);
    ASSERT_EQ(ent->info.DFType, DF_ENT_FOLIAGE);

    test_teardown_game();
}

TEST(test_ent_panic_foliage_burst) {
    test_init_arena(99);
    test_reseed(99);
    test_set_player_hp(500, 500);

    // Place ent far from player so closestFearedEnemy >= 3
    short mx = player.loc.x + 8;
    short my = player.loc.y;
    creature *ent = test_place_monster(MK_ENT, mx, my);
    ASSERT(ent != NULL);

    // Set state to WANDERING so the else-if chain in updateMonsterState
    // falls through to the FLEES_NEAR_DEATH check
    ent->creatureState = MONSTER_WANDERING;

    // Reduce HP below 25% max (80/4=20) to force fleeing from non-fleeing state
    ent->currentHP = 15;

    // Call updateMonsterState directly to trigger the state transition
    updateMonsterState(ent);

    // Count foliage tiles in the radius-4 area around the ent's position
    int foliageCount = 0;
    for (short dy = -4; dy <= 4; dy++) {
        for (short dx = -4; dx <= 4; dx++) {
            short fx = mx + dx;
            short fy = my + dy;
            if (coordinatesAreInMap(fx, fy)
                && pmap[fx][fy].layers[SURFACE] == ENT_FOLIAGE) {
                foliageCount++;
            }
        }
    }

    // With 50% chance per tile over ~81 tiles, expect roughly 30-50 foliage tiles.
    // Use a generous lower bound to avoid flaky tests.
    ASSERT(foliageCount >= 10);
    ASSERT_EQ(ent->creatureState, MONSTER_FLEEING);

    test_teardown_game();
}

TEST(test_ent_panic_burst_fires_only_once) {
    test_init_arena(99);
    test_reseed(99);
    test_set_player_hp(500, 500);

    short mx = player.loc.x + 8;
    short my = player.loc.y;
    creature *ent = test_place_monster(MK_ENT, mx, my);
    ASSERT(ent != NULL);

    // Trigger first burst: set wandering + low HP, call updateMonsterState
    ent->creatureState = MONSTER_WANDERING;
    ent->currentHP = 15;
    updateMonsterState(ent);
    ASSERT_EQ(ent->creatureState, MONSTER_FLEEING);

    // Clear all foliage to detect any new spawns from a second call
    for (short dy = -4; dy <= 4; dy++) {
        for (short dx = -4; dx <= 4; dx++) {
            short fx = mx + dx;
            short fy = my + dy;
            if (coordinatesAreInMap(fx, fy)
                && pmap[fx][fy].layers[SURFACE] == ENT_FOLIAGE) {
                test_set_terrain(fx, fy, SURFACE, NOTHING);
            }
        }
    }

    // Call updateMonsterState again — ent is already MONSTER_FLEEING, no second burst
    updateMonsterState(ent);

    // Count foliage — should be zero since no burst fired and no DF trail from updateMonsterState
    int foliageAfterSecond = 0;
    for (short dy = -4; dy <= 4; dy++) {
        for (short dx = -4; dx <= 4; dx++) {
            short fx = mx + dx;
            short fy = my + dy;
            if (coordinatesAreInMap(fx, fy)
                && pmap[fx][fy].layers[SURFACE] == ENT_FOLIAGE) {
                foliageAfterSecond++;
            }
        }
    }

    // No burst should have occurred (updateMonsterState doesn't spawn DF trail)
    ASSERT_EQ(foliageAfterSecond, 0);

    test_teardown_game();
}

SUITE(ent) {
    RUN_TEST(test_ent_catalog_entry);
    RUN_TEST(test_ent_foliage_df_spawns_correct_terrain);
    RUN_TEST(test_ent_foliage_obstructs_vision);
    RUN_TEST(test_ent_foliage_is_flammable);
    RUN_TEST(test_ent_foliage_decays_to_fungus);
    RUN_TEST(test_ent_fungus_decays_to_nothing);
    RUN_TEST(test_ent_foliage_no_promote_on_step);
    RUN_TEST(test_ent_placement_and_df_trail);
    RUN_TEST(test_ent_panic_foliage_burst);
    RUN_TEST(test_ent_panic_burst_fires_only_once);
}
