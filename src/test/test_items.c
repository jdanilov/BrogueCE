// test_items.c — Tests for item system

#include "test_harness.h"

TEST(test_give_weapon_to_player) {
    test_init_game(12345);

    item *sword = test_give_item(WEAPON, SWORD, 3);
    ASSERT(sword != NULL);
    ASSERT_EQ(sword->category, WEAPON);
    ASSERT_EQ(sword->kind, SWORD);
    ASSERT_EQ(sword->enchant1, 3);

    test_teardown_game();
}

TEST(test_give_armor_to_player) {
    test_init_game(12345);

    item *armor = test_give_item(ARMOR, CHAIN_MAIL, 2);
    ASSERT(armor != NULL);
    ASSERT_EQ(armor->category, ARMOR);
    ASSERT_EQ(armor->kind, CHAIN_MAIL);
    ASSERT_EQ(armor->enchant1, 2);

    test_teardown_game();
}

TEST(test_pack_items_linked_list) {
    test_init_game(12345);

    // Count initial pack items
    int initialCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        initialCount++;
    }

    // Add an item
    test_give_item(POTION, 0, 0);

    int newCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        newCount++;
    }

    ASSERT_EQ(newCount, initialCount + 1);

    test_teardown_game();
}

TEST(test_floor_items_exist_on_level) {
    test_init_game(12345);

    // There should be some items on the floor of level 1
    int floorCount = 0;
    for (item *it = floorItems->nextItem; it != NULL; it = it->nextItem) {
        floorCount++;
    }

    ASSERT_GT(floorCount, 0);

    test_teardown_game();
}

SUITE(items) {
    RUN_TEST(test_give_weapon_to_player);
    RUN_TEST(test_give_armor_to_player);
    RUN_TEST(test_pack_items_linked_list);
    RUN_TEST(test_floor_items_exist_on_level);
}
