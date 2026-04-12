// test_ranged.c — Tests for ranged combat items and ranged monster attacks

#include "test_harness.h"

TEST(test_dart_item_created_with_quantity) {
    test_init_game(12345);

    item *dart = test_give_item(WEAPON, DART, 0);
    ASSERT(dart != NULL);
    ASSERT_EQ(dart->category, WEAPON);
    ASSERT_EQ(dart->kind, DART);
    ASSERT_GE(dart->quantity, 1);

    test_teardown_game();
}

TEST(test_staff_item_has_charges) {
    test_init_game(12345);

    item *staff = test_give_item(STAFF, STAFF_LIGHTNING, 3);
    ASSERT(staff != NULL);
    ASSERT_EQ(staff->category, STAFF);
    ASSERT_EQ(staff->kind, STAFF_LIGHTNING);
    ASSERT_EQ(staff->enchant1, 3);

    test_teardown_game();
}

TEST(test_wand_item_created) {
    test_init_game(12345);

    item *wand = test_give_item(WAND, WAND_TELEPORT, 0);
    ASSERT(wand != NULL);
    ASSERT_EQ(wand->category, WAND);
    ASSERT_EQ(wand->kind, WAND_TELEPORT);

    test_teardown_game();
}

TEST(test_incendiary_dart_exists) {
    test_init_game(12345);

    item *dart = test_give_item(WEAPON, INCENDIARY_DART, 0);
    ASSERT(dart != NULL);
    ASSERT_EQ(dart->category, WEAPON);
    ASSERT_EQ(dart->kind, INCENDIARY_DART);
    ASSERT_GE(dart->quantity, 1);

    test_teardown_game();
}

TEST(test_ranged_monster_attacks_player) {
    test_init_game(42);

    // Set up a clear arena so the turret has line of sight
    short px = player.loc.x;
    short py = player.loc.y;
    test_clear_area(px, py, 6);
    test_remove_all_monsters();

    // Give the player enough HP to survive several hits
    test_set_player_hp(200, 200);
    short startHP = player.currentHP;

    // Place an arrow turret 4 cells to the right of the player
    short tx = px + 4;
    short ty = py;
    ASSERT(tx >= 0 && tx < DCOLS);
    ASSERT(ty >= 0 && ty < DROWS);

    creature *turret = test_place_monster(MK_ARROW_TURRET, tx, ty);
    ASSERT(turret != NULL);
    ASSERT_GT(turret->currentHP, 0);

    // Rest several turns to give the turret time to attack
    for (int i = 0; i < 20 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    // The turret should have dealt ranged damage to the player
    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.currentHP, startHP);
    }

    test_teardown_game();
}

SUITE(ranged) {
    RUN_TEST(test_dart_item_created_with_quantity);
    RUN_TEST(test_staff_item_has_charges);
    RUN_TEST(test_wand_item_created);
    RUN_TEST(test_incendiary_dart_exists);
    RUN_TEST(test_ranged_monster_attacks_player);
}
