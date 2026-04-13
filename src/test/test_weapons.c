// test_weapons.c — Tests for special weapon mechanics

#include "test_harness.h"

// Helper: equip a weapon, unequipping any current weapon first.
static void equip_weapon(item *theItem) {
    if (rogue.weapon) {
        rogue.weapon->flags &= ~ITEM_EQUIPPED;
    }
    rogue.weapon = theItem;
    theItem->flags |= ITEM_EQUIPPED;
}

TEST(test_whip_attacks_at_range_2) {
    test_init_arena(12345);

    // Give and equip a whip with high enchant for guaranteed hit
    item *whip = test_give_item(WEAPON, WHIP, 10);
    ASSERT(whip != NULL);
    ASSERT(whip->flags & ITEM_ATTACKS_EXTEND);
    equip_weapon(whip);

    // Place a rat 2 cells to the right of the player.
    short ratX = player.loc.x + 2;
    short ratY = player.loc.y;

    creature *rat = test_place_monster(MK_RAT, ratX, ratY);
    ASSERT(rat != NULL);
    short ratStartHP = rat->currentHP;
    ASSERT_GT(ratStartHP, 0);

    // Verify no monster at distance 1 (whip needs empty adjacent cell)
    ASSERT(test_monster_at(player.loc.x + 1, player.loc.y) == NULL);

    // Move RIGHT — whip should hit the rat at range 2
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        creature *ratAfter = test_monster_at(ratX, ratY);
        if (ratAfter) {
            // Rat took damage
            ASSERT_LT(ratAfter->currentHP, ratStartHP);
        }
        // If ratAfter is NULL, the rat died — also valid
    }

    test_teardown_game();
}

TEST(test_axe_hits_all_adjacent) {
    test_init_arena(54321);

    // Give and equip a war axe (ITEM_ATTACKS_ALL_ADJACENT)
    item *axe = test_give_item(WEAPON, WAR_AXE, 5);
    ASSERT(axe != NULL);
    ASSERT(axe->flags & ITEM_ATTACKS_ALL_ADJACENT);
    equip_weapon(axe);

    // Boost player HP so we survive any counter-attacks
    test_set_player_hp(200, 200);

    // Place rats adjacent to the player in multiple directions
    short px = player.loc.x;
    short py = player.loc.y;

    // Rat to the right (we will attack this one by moving RIGHT)
    creature *rat1 = test_place_monster(MK_RAT, px + 1, py);
    ASSERT(rat1 != NULL);
    short rat1HP = rat1->currentHP;

    // Rat below the player (adjacent, should also be hit by sweep)
    creature *rat2 = test_place_monster(MK_RAT, px, py + 1);
    ASSERT(rat2 != NULL);
    short rat2HP = rat2->currentHP;

    // Rat above the player (adjacent, should also be hit by sweep)
    creature *rat3 = test_place_monster(MK_RAT, px, py - 1);
    ASSERT(rat3 != NULL);
    short rat3HP = rat3->currentHP;

    // Move RIGHT to attack rat1 — axe should sweep and hit all adjacent
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // The primary target (rat1) should have been hit or killed
        creature *rat1After = test_monster_at(px + 1, py);
        boolean rat1Hit = (rat1After == NULL || rat1After->currentHP < rat1HP);
        ASSERT(rat1Hit);

        // At least one of the other adjacent rats should also have taken damage
        // (the sweep from ITEM_ATTACKS_ALL_ADJACENT)
        creature *rat2After = test_monster_at(px, py + 1);
        creature *rat3After = test_monster_at(px, py - 1);
        boolean rat2Hit = (rat2After == NULL || rat2After->currentHP < rat2HP);
        boolean rat3Hit = (rat3After == NULL || rat3After->currentHP < rat3HP);
        ASSERT(rat2Hit || rat3Hit);
    }

    test_teardown_game();
}

TEST(test_spear_penetrates) {
    test_init_arena(99999);

    // Give and equip a pike (ITEM_ATTACKS_PENETRATE)
    item *pike = test_give_item(WEAPON, PIKE, 5);
    ASSERT(pike != NULL);
    ASSERT(pike->flags & ITEM_ATTACKS_PENETRATE);
    equip_weapon(pike);

    // Boost player HP
    test_set_player_hp(200, 200);

    short px = player.loc.x;
    short py = player.loc.y;

    // Place rat at distance 1 (adjacent, right)
    creature *rat1 = test_place_monster(MK_RAT, px + 1, py);
    ASSERT(rat1 != NULL);
    short rat1HP = rat1->currentHP;

    // Place rat at distance 2 (behind the first, also right)
    creature *rat2 = test_place_monster(MK_RAT, px + 2, py);
    ASSERT(rat2 != NULL);
    short rat2HP = rat2->currentHP;

    // Move RIGHT to attack — spear should penetrate and hit both
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // First rat should have been hit or killed
        creature *rat1After = test_monster_at(px + 1, py);
        boolean rat1Hit = (rat1After == NULL || rat1After->currentHP < rat1HP);
        ASSERT(rat1Hit);

        // Second rat (behind first) should also have been hit by penetration
        creature *rat2After = test_monster_at(px + 2, py);
        boolean rat2Hit = (rat2After == NULL || rat2After->currentHP < rat2HP);
        ASSERT(rat2Hit);
    }

    test_teardown_game();
}

TEST(test_weapon_equip_changes_rogue_weapon) {
    test_init_game(12345);

    // Give and equip a sword
    item *sword = test_give_item(WEAPON, SWORD, 0);
    ASSERT(sword != NULL);
    equip_weapon(sword);

    ASSERT(rogue.weapon == sword);
    ASSERT(sword->flags & ITEM_EQUIPPED);

    // Give and equip a different weapon (broadsword)
    item *broadsword = test_give_item(WEAPON, BROADSWORD, 0);
    ASSERT(broadsword != NULL);
    equip_weapon(broadsword);

    // rogue.weapon should now point to the broadsword
    ASSERT(rogue.weapon == broadsword);
    ASSERT(broadsword->flags & ITEM_EQUIPPED);

    // The old sword should no longer be equipped
    ASSERT(!(sword->flags & ITEM_EQUIPPED));

    test_teardown_game();
}

TEST(test_enchanted_weapon_kills_weak_monster) {
    test_init_arena(77777);

    // Give and equip a highly enchanted sword (+5)
    item *sword = test_give_item(WEAPON, SWORD, 5);
    ASSERT(sword != NULL);
    equip_weapon(sword);

    // Boost player HP to survive any counter-attack
    test_set_player_hp(200, 200);

    short px = player.loc.x;
    short py = player.loc.y;

    // Place a rat and set it to 1 HP — a +5 sword should always kill it
    creature *rat = test_place_monster(MK_RAT, px + 1, py);
    ASSERT(rat != NULL);
    rat->currentHP = 1;

    // Move RIGHT to attack
    test_move(RIGHT);

    if (!rogue.gameHasEnded) {
        // The 1-HP rat should be dead
        creature *ratAfter = test_monster_at(px + 1, py);
        ASSERT(ratAfter == NULL);
        ASSERT(!(pmap[px + 1][py].flags & HAS_MONSTER));
    }

    test_teardown_game();
}

SUITE(weapons) {
    RUN_TEST(test_whip_attacks_at_range_2);
    RUN_TEST(test_axe_hits_all_adjacent);
    RUN_TEST(test_spear_penetrates);
    RUN_TEST(test_weapon_equip_changes_rogue_weapon);
    RUN_TEST(test_enchanted_weapon_kills_weak_monster);
}
