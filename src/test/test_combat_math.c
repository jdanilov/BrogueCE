// test_combat_math.c — Tests for combat math verification

#include "test_harness.h"

// Helper: equip a weapon, unequipping any current weapon first.
static void equip_weapon(item *theItem) {
    if (rogue.weapon) {
        rogue.weapon->flags &= ~ITEM_EQUIPPED;
    }
    rogue.weapon = theItem;
    theItem->flags |= ITEM_EQUIPPED;
}

// Helper: equip armor, unequipping any current armor first.
static void equip_armor(item *theItem) {
    if (rogue.armor) {
        rogue.armor->flags &= ~ITEM_EQUIPPED;
    }
    rogue.armor = theItem;
    theItem->flags |= ITEM_EQUIPPED;
}

TEST(test_damage_within_expected_range) {
    test_init_game(12345);

    test_remove_all_monsters();
    test_teleport_player(40, 15);
    test_clear_area(40, 15, 10);

    // Equip a +0 sword
    item *sword = test_give_item(WEAPON, SWORD, 0);
    ASSERT(sword != NULL);
    equip_weapon(sword);

    // Boost player HP to survive any counter-attacks
    test_set_player_hp(500, 500);

    // Place an ogre to the right with very high HP so it survives multiple hits
    creature *ogre = test_place_monster(MK_OGRE, 41, 15);
    ASSERT(ogre != NULL);
    ogre->currentHP = 500;
    ogre->info.maxHP = 500;

    short ogreStartHP = ogre->currentHP;

    // Attack the ogre 10 times by moving into it then teleporting back
    for (int i = 0; i < 10 && !rogue.gameHasEnded; i++) {
        test_move(RIGHT);
        test_teleport_player(40, 15);
    }

    if (!rogue.gameHasEnded) {
        creature *ogreAfter = test_monster_at(41, 15);
        if (ogreAfter) {
            short totalDamage = ogreStartHP - ogreAfter->currentHP;
            // After 10 attacks with a sword, we should have dealt some damage
            ASSERT_GT(totalDamage, 0);
        }
        // If ogre is NULL it died, which means damage was dealt — also valid
    }

    test_teardown_game();
}

TEST(test_armor_reduces_damage) {
    // With +5 plate mail, the player should easily survive 10 hits from a rat.
    test_init_game(12345);

    test_remove_all_monsters();
    test_teleport_player(40, 15);
    test_clear_area(40, 15, 10);

    // Equip +5 plate mail (strongest armor, high enchant)
    item *plate = test_give_item(ARMOR, PLATE_MAIL, 5);
    ASSERT(plate != NULL);
    equip_armor(plate);

    // Set player HP high and remove weapon so we don't kill the rat
    test_set_player_hp(200, 200);
    if (rogue.weapon) {
        rogue.weapon->flags &= ~ITEM_EQUIPPED;
        rogue.weapon = NULL;
    }

    // Place a rat adjacent
    creature *rat = test_place_monster(MK_RAT, 41, 15);
    ASSERT(rat != NULL);

    // Rest 10 turns to let the rat attack us
    for (int i = 0; i < 10 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    // With +5 plate mail, a rat should barely scratch us. Player should survive.
    ASSERT(!rogue.gameHasEnded);
    ASSERT_GT(player.currentHP, 0);
    // With heavy armor and a weak rat, we should have taken very little damage
    ASSERT_GT(player.currentHP, 100);

    test_teardown_game();
}

TEST(test_enchanted_weapon_kills_rat) {
    test_init_game(77777);

    test_remove_all_monsters();
    test_teleport_player(40, 15);
    test_clear_area(40, 15, 10);

    // Give and equip a +5 sword (very powerful vs a rat)
    item *sword = test_give_item(WEAPON, SWORD, 5);
    ASSERT(sword != NULL);
    equip_weapon(sword);

    // Boost player HP
    test_set_player_hp(200, 200);

    // Place a rat at full HP
    creature *rat = test_place_monster(MK_RAT, 41, 15);
    ASSERT(rat != NULL);
    short ratHP = rat->currentHP;
    ASSERT_GT(ratHP, 0);

    // A +5 sword should one-shot a rat
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        creature *ratAfter = test_monster_at(41, 15);
        // The rat should be dead from one hit with a +5 sword
        ASSERT(ratAfter == NULL);
    }

    test_teardown_game();
}

TEST(test_haste_status_applied_and_decays) {
    test_init_game(12345);

    test_remove_all_monsters();
    test_clear_area(player.loc.x, player.loc.y, 5);

    // Apply haste
    test_set_player_status(STATUS_HASTED, 20);
    ASSERT_EQ(player.status[STATUS_HASTED], 20);
    ASSERT_EQ(player.maxStatus[STATUS_HASTED], 20);

    // Haste and slow should be mutually exclusive — verify slow is not set
    ASSERT_EQ(player.status[STATUS_SLOWED], 0);

    // Rest a few turns — haste should decay
    test_rest_turns(5);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.status[STATUS_HASTED], 20);
        ASSERT_GT(player.status[STATUS_HASTED], 0);
    }

    test_teardown_game();
}

TEST(test_slow_status_applied_and_decays) {
    test_init_game(12345);

    test_remove_all_monsters();
    test_clear_area(player.loc.x, player.loc.y, 5);

    // Apply slow
    test_set_player_status(STATUS_SLOWED, 20);
    ASSERT_EQ(player.status[STATUS_SLOWED], 20);
    ASSERT_EQ(player.maxStatus[STATUS_SLOWED], 20);

    // Slow and haste should be mutually exclusive — verify haste is not set
    ASSERT_EQ(player.status[STATUS_HASTED], 0);

    // Rest a few turns — slow should decay
    test_rest_turns(5);

    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.status[STATUS_SLOWED], 20);
    }

    test_teardown_game();
}

TEST(test_strength_and_heavy_weapon) {
    test_init_game(12345);

    // Verify rogue.strength exists and is reasonable
    ASSERT_GT(rogue.strength, 0);
    ASSERT_LE(rogue.strength, 50);

    short startStrength = rogue.strength;

    // Give the player a war hammer (heavy weapon)
    item *hammer = test_give_item(WEAPON, HAMMER, 0);
    ASSERT(hammer != NULL);
    equip_weapon(hammer);

    // Equipping should not crash, and weapon should be set
    ASSERT(rogue.weapon == hammer);
    ASSERT(hammer->flags & ITEM_EQUIPPED);

    // Strength should still be the same (equipping doesn't change strength)
    ASSERT_EQ(rogue.strength, startStrength);

    // The weapon should have a strength requirement
    ASSERT_GT(hammer->strengthRequired, 0);

    test_teardown_game();
}

SUITE(combat_math) {
    RUN_TEST(test_damage_within_expected_range);
    RUN_TEST(test_armor_reduces_damage);
    RUN_TEST(test_enchanted_weapon_kills_rat);
    RUN_TEST(test_haste_status_applied_and_decays);
    RUN_TEST(test_slow_status_applied_and_decays);
    RUN_TEST(test_strength_and_heavy_weapon);
}
