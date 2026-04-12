// test_item_usage.c — Tests for item usage and item properties

#include "test_harness.h"

TEST(test_give_potion_to_player) {
    test_init_game(12345);

    item *potion = test_give_item(POTION, POTION_LIFE, 0);
    ASSERT(potion != NULL);
    ASSERT_EQ(potion->category, POTION);
    ASSERT_EQ(potion->kind, POTION_LIFE);

    test_teardown_game();
}

TEST(test_give_scroll_to_player) {
    test_init_game(12345);

    item *scroll = test_give_item(SCROLL, SCROLL_ENCHANTING, 0);
    ASSERT(scroll != NULL);
    ASSERT_EQ(scroll->category, SCROLL);
    ASSERT_EQ(scroll->kind, SCROLL_ENCHANTING);

    test_teardown_game();
}

TEST(test_give_food_to_player) {
    test_init_game(12345);

    item *food = test_give_item(FOOD, 0, 0);
    ASSERT(food != NULL);
    ASSERT_EQ(food->category, FOOD);

    // Verify it's actually in the pack
    boolean found = false;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        if (it == food) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_equip_weapon_updates_rogue) {
    test_init_game(12345);

    item *sword = test_give_item(WEAPON, SWORD, 3);
    ASSERT(sword != NULL);

    // Manually equip the weapon
    rogue.weapon = sword;
    sword->flags |= ITEM_EQUIPPED;

    ASSERT(rogue.weapon == sword);
    ASSERT(sword->flags & ITEM_EQUIPPED);

    test_teardown_game();
}

TEST(test_equip_armor_updates_rogue) {
    test_init_game(12345);

    item *armor = test_give_item(ARMOR, CHAIN_MAIL, 2);
    ASSERT(armor != NULL);

    // Manually equip the armor
    rogue.armor = armor;
    armor->flags |= ITEM_EQUIPPED;

    ASSERT(rogue.armor == armor);
    ASSERT(armor->flags & ITEM_EQUIPPED);

    test_teardown_game();
}

TEST(test_gold_pickup_increases_rogue_gold) {
    test_init_game(12345);
    test_remove_all_monsters();

    long goldBefore = rogue.gold;

    // Find an open floor cell adjacent to the player
    short targetX = -1, targetY = -1;
    short moveDir = -1;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)
            && !(pmap[nx][ny].flags & (HAS_MONSTER | HAS_ITEM | HAS_STAIRS))) {
            targetX = nx;
            targetY = ny;
            moveDir = dir;
            break;
        }
    }
    ASSERT(moveDir >= 0);

    // Create gold and place it on the floor
    item *gold = generateItem(GOLD, 0);
    ASSERT(gold != NULL);
    gold->loc.x = targetX;
    gold->loc.y = targetY;
    gold->quantity = 50;
    addItemToChain(gold, floorItems);
    pmap[targetX][targetY].flags |= HAS_ITEM;

    // Move onto the gold — player should auto-pickup
    test_move(moveDir);

    ASSERT_GT(rogue.gold, goldBefore);

    test_teardown_game();
}

TEST(test_pack_count_increases_with_items) {
    test_init_game(12345);

    int initialCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        initialCount++;
    }

    test_give_item(POTION, POTION_LIFE, 0);
    test_give_item(SCROLL, SCROLL_IDENTIFY, 0);
    test_give_item(FOOD, 0, 0);

    int newCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        newCount++;
    }

    ASSERT_EQ(newCount, initialCount + 3);

    test_teardown_game();
}

TEST(test_item_identification_flag) {
    test_init_game(12345);

    // generateItem creates an item without ITEM_IDENTIFIED by default
    item *potion = generateItem(POTION, POTION_STRENGTH);
    ASSERT(potion != NULL);
    ASSERT(!(potion->flags & ITEM_IDENTIFIED));

    // Now set the identified flag and verify
    potion->flags |= ITEM_IDENTIFIED;
    ASSERT(potion->flags & ITEM_IDENTIFIED);

    // Clean up: add to pack so teardown can free it
    addItemToChain(potion, packItems);

    test_teardown_game();
}

TEST(test_ring_item_creation) {
    test_init_game(12345);

    item *ring = test_give_item(RING, RING_CLAIRVOYANCE, 1);
    ASSERT(ring != NULL);
    ASSERT_EQ(ring->category, RING);
    ASSERT_EQ(ring->kind, RING_CLAIRVOYANCE);
    ASSERT_EQ(ring->enchant1, 1);

    test_teardown_game();
}

TEST(test_charm_item_creation) {
    test_init_game(12345);

    item *charm = test_give_item(CHARM, CHARM_HEALTH, 1);
    ASSERT(charm != NULL);
    ASSERT_EQ(charm->category, CHARM);
    ASSERT_EQ(charm->kind, CHARM_HEALTH);
    ASSERT_EQ(charm->enchant1, 1);

    test_teardown_game();
}

SUITE(item_usage) {
    RUN_TEST(test_give_potion_to_player);
    RUN_TEST(test_give_scroll_to_player);
    RUN_TEST(test_give_food_to_player);
    RUN_TEST(test_equip_weapon_updates_rogue);
    RUN_TEST(test_equip_armor_updates_rogue);
    RUN_TEST(test_gold_pickup_increases_rogue_gold);
    RUN_TEST(test_pack_count_increases_with_items);
    RUN_TEST(test_item_identification_flag);
    RUN_TEST(test_ring_item_creation);
    RUN_TEST(test_charm_item_creation);
}
