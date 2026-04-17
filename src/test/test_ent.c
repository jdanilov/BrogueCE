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

SUITE(ent) {
    RUN_TEST(test_ent_catalog_entry);
    RUN_TEST(test_ent_foliage_df_spawns_correct_terrain);
    RUN_TEST(test_ent_foliage_obstructs_vision);
    RUN_TEST(test_ent_foliage_is_flammable);
    RUN_TEST(test_ent_foliage_decays_to_fungus);
    RUN_TEST(test_ent_fungus_decays_to_nothing);
    RUN_TEST(test_ent_foliage_no_promote_on_step);
    RUN_TEST(test_ent_placement_and_df_trail);
}
