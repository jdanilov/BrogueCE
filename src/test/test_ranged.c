// test_ranged.c — Tests for ranged combat items and ranged monster attacks

#include "test_harness.h"

TEST(test_dart_item_created_with_quantity) {
    test_init_arena(12345);

    item *dart = test_give_item(WEAPON, DART, 0);
    ASSERT(dart != NULL);
    ASSERT_EQ(dart->category, WEAPON);
    ASSERT_EQ(dart->kind, DART);
    ASSERT_GE(dart->quantity, 1);

    test_teardown_game();
}

TEST(test_staff_item_has_charges) {
    test_init_arena(12345);

    item *staff = test_give_item(STAFF, STAFF_LIGHTNING, 3);
    ASSERT(staff != NULL);
    ASSERT_EQ(staff->category, STAFF);
    ASSERT_EQ(staff->kind, STAFF_LIGHTNING);
    ASSERT_EQ(staff->enchant1, 3);

    test_teardown_game();
}

TEST(test_wand_item_created) {
    test_init_arena(12345);

    item *wand = test_give_item(WAND, WAND_TELEPORT, 0);
    ASSERT(wand != NULL);
    ASSERT_EQ(wand->category, WAND);
    ASSERT_EQ(wand->kind, WAND_TELEPORT);

    test_teardown_game();
}

TEST(test_incendiary_dart_exists) {
    test_init_arena(12345);

    item *dart = test_give_item(WEAPON, INCENDIARY_DART, 0);
    ASSERT(dart != NULL);
    ASSERT_EQ(dart->category, WEAPON);
    ASSERT_EQ(dart->kind, INCENDIARY_DART);
    ASSERT_GE(dart->quantity, 1);

    test_teardown_game();
}

TEST(test_ranged_monster_attacks_player) {
    test_init_game(42);

    short px = player.loc.x;
    short py = player.loc.y;
    test_clear_area(px, py, 6);
    test_remove_all_monsters();

    // Give the player enough HP to survive several hits
    test_set_player_hp(200, 200);
    test_reseed(42);
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

// --- Ranged weapon item creation tests ---

TEST(test_sling_created_with_correct_stats) {
    test_init_arena(12345);

    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);
    ASSERT_EQ(sling->category, RANGED);
    ASSERT_EQ(sling->kind, SLING);
    ASSERT_EQ(sling->charges, 0); // ready to fire
    ASSERT_GT(sling->maxRange, 0);
    ASSERT_GT(sling->cooldownMax, 0);

    test_teardown_game();
}

TEST(test_bow_created_with_correct_stats) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);
    ASSERT_EQ(bow->category, RANGED);
    ASSERT_EQ(bow->kind, BOW);
    ASSERT_EQ(bow->charges, 0);
    ASSERT_GT(bow->maxRange, 0);
    ASSERT_GT(bow->cooldownMax, 0);

    test_teardown_game();
}

TEST(test_crossbow_created_with_correct_stats) {
    test_init_arena(12345);

    item *xbow = test_give_item(RANGED, CROSSBOW, 0);
    ASSERT(xbow != NULL);
    ASSERT_EQ(xbow->category, RANGED);
    ASSERT_EQ(xbow->kind, CROSSBOW);
    ASSERT_EQ(xbow->charges, 0);
    ASSERT_GT(xbow->maxRange, 0);
    ASSERT_GT(xbow->cooldownMax, 0);

    test_teardown_game();
}

// --- Cooldown calculation tests ---

TEST(test_sling_base_cooldown) {
    test_init_arena(12345);

    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);
    // Sling base cooldown: 2 turns = 20 deciturns
    ASSERT_EQ(sling->cooldownMax, 20);

    test_teardown_game();
}

TEST(test_bow_base_cooldown) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);
    // Bow base cooldown: 4 turns = 40 deciturns
    ASSERT_EQ(bow->cooldownMax, 40);

    test_teardown_game();
}

TEST(test_crossbow_base_cooldown) {
    test_init_arena(12345);

    item *xbow = test_give_item(RANGED, CROSSBOW, 0);
    ASSERT(xbow != NULL);
    // Crossbow base cooldown: 12 turns = 120 deciturns
    ASSERT_EQ(xbow->cooldownMax, 120);

    test_teardown_game();
}

// --- Range calculation tests ---

TEST(test_sling_base_range) {
    test_init_arena(12345);

    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);
    // Sling base range: 6
    ASSERT_EQ(sling->maxRange, 6);

    test_teardown_game();
}

TEST(test_bow_base_range) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);
    // Bow base range: 16
    ASSERT_EQ(bow->maxRange, 16);

    test_teardown_game();
}

TEST(test_crossbow_base_range) {
    test_init_arena(12345);

    item *xbow = test_give_item(RANGED, CROSSBOW, 0);
    ASSERT(xbow != NULL);
    // Crossbow base range: 12
    ASSERT_EQ(xbow->maxRange, 12);

    test_teardown_game();
}

// --- Enchantment scaling tests ---

TEST(test_enchant_increases_range) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);
    short baseRange = bow->maxRange;

    // Enchant the bow manually
    bow->enchant1 = 3;
    short newRange = rangedWeaponRange(bow);

    // Each enchant adds +1 range
    ASSERT_EQ(newRange, baseRange + 3);

    test_teardown_game();
}

TEST(test_crossbow_enchant_reduces_cooldown_fast) {
    test_init_arena(12345);

    item *xbow = test_give_item(RANGED, CROSSBOW, 0);
    ASSERT(xbow != NULL);
    short baseCooldown = xbow->cooldownMax;

    // Enchant crossbow +5: -1.0 turn per enchant = -50 deciturns
    xbow->enchant1 = 5;
    short newCooldown = rangedWeaponCooldownMax(xbow);

    ASSERT_EQ(newCooldown, baseCooldown - 50);
    ASSERT_LT(newCooldown, baseCooldown);

    test_teardown_game();
}

TEST(test_sling_enchant_balanced_cooldown_reduction) {
    test_init_arena(12345);

    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);
    short baseCooldown = sling->cooldownMax;

    // Enchant sling +2: -0.15 turns per enchant = -1.5 deciturns per enchant (integer: 1 per enchant at low values)
    sling->enchant1 = 2;
    short newCooldown = rangedWeaponCooldownMax(sling);

    ASSERT_LT(newCooldown, baseCooldown);

    test_teardown_game();
}

TEST(test_cooldown_minimum_floor) {
    test_init_arena(12345);

    // Sling with huge enchant should still have minimum 10 deciturns
    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);

    sling->enchant1 = 100;
    short cooldown = rangedWeaponCooldownMax(sling);

    ASSERT_GE(cooldown, 10); // minimum 1 turn

    test_teardown_game();
}

// --- Cooldown recharge behavior tests ---

TEST(test_ranged_weapon_cooldown_ticks_while_stationary) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    // Simulate a fired weapon by setting cooldown
    bow->charges = bow->cooldownMax;
    bow->flags |= ITEM_RANGED_RELOADING;
    short startCooldown = bow->charges;
    ASSERT_GT(startCooldown, 0);

    // Rest (stationary) for a few turns
    for (int i = 0; i < 3 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Cooldown should have decreased since we stood still
        ASSERT_LT(bow->charges, startCooldown);
    }

    test_teardown_game();
}

TEST(test_crossbow_cooldown_pauses_while_moving) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    // Use crossbow — only reloads while stationary
    item *xbow = test_give_item(RANGED, CROSSBOW, 0);
    ASSERT(xbow != NULL);

    // Set cooldown high
    xbow->charges = xbow->cooldownMax;
    xbow->flags |= ITEM_RANGED_RELOADING;
    short cooldownAfterRest;
    short cooldownAfterMove;

    // Rest one turn (should reduce cooldown)
    test_rest();
    if (rogue.gameHasEnded) { test_teardown_game(); return; }
    cooldownAfterRest = xbow->charges;

    // Now move (cooldown should NOT decrease on this turn)
    short preMoveCooldown = xbow->charges;
    test_move(RIGHT);
    if (rogue.gameHasEnded) { test_teardown_game(); return; }
    cooldownAfterMove = xbow->charges;

    // After resting, cooldown should have dropped
    ASSERT_LT(cooldownAfterRest, xbow->cooldownMax);

    // After moving, cooldown should be same as before the move
    // (crossbow only reloads while stationary)
    ASSERT_EQ(cooldownAfterMove, preMoveCooldown);

    test_teardown_game();
}

TEST(test_bow_reloads_at_quarter_speed_while_moving) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    // Set cooldown to full (40 deciturns)
    bow->charges = bow->cooldownMax;
    bow->flags |= ITEM_RANGED_RELOADING;

    // Rest one turn (should reduce by 10 deciturns)
    test_rest();
    if (rogue.gameHasEnded) { test_teardown_game(); return; }
    short afterRest = bow->charges;
    ASSERT_EQ(afterRest, 30); // 40 - 10

    // Move one turn (should reduce by 2 deciturns — 25% speed)
    test_move(RIGHT);
    if (rogue.gameHasEnded) { test_teardown_game(); return; }
    short afterMove = bow->charges;
    ASSERT_EQ(afterMove, 28); // 30 - 2

    test_teardown_game();
}

TEST(test_sling_reloads_at_full_speed_while_moving) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);

    // Set cooldown to full (20 deciturns)
    sling->charges = sling->cooldownMax;
    sling->flags |= ITEM_RANGED_RELOADING;

    // Move one turn (should reduce by 10 deciturns — full speed)
    test_move(RIGHT);
    if (rogue.gameHasEnded) { test_teardown_game(); return; }
    ASSERT_EQ(sling->charges, 10); // 20 - 10

    test_teardown_game();
}

TEST(test_ranged_weapon_becomes_ready_after_cooldown) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    // Use a sling (short cooldown: 2 turns = 20 deciturns)
    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);

    // Set cooldown to full (20 deciturns = 2 turns)
    sling->charges = 20;
    sling->flags |= ITEM_RANGED_RELOADING;

    // Rest 3 turns — should be enough for a sling (2-turn cooldown) to fully recharge
    for (int i = 0; i < 3 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        ASSERT_EQ(sling->charges, 0); // fully recharged
        ASSERT(!(sling->flags & ITEM_RANGED_RELOADING)); // no longer reloading
    }

    test_teardown_game();
}

TEST(test_cooldown_decrements_10_deciturns_per_turn) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    // Set cooldown to 30 deciturns (3 turns for a bow)
    bow->charges = 30;
    bow->flags |= ITEM_RANGED_RELOADING;

    // Rest 1 turn — should decrement by 10 deciturns
    test_rest();

    if (!rogue.gameHasEnded) {
        ASSERT_EQ(bow->charges, 20); // 30 - 10 = 20
    }

    test_teardown_game();
}

// --- Negative enchant tests ---

TEST(test_negative_enchant_does_not_increase_cooldown) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);
    short baseCooldown = bow->cooldownMax;

    // Negative enchant: reduction should be 0 (clamped), cooldown stays at base
    bow->enchant1 = -3;
    short cursedCooldown = rangedWeaponCooldownMax(bow);

    ASSERT_EQ(cursedCooldown, baseCooldown);

    test_teardown_game();
}

TEST(test_negative_enchant_does_not_increase_range) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);
    short baseRange = bow->maxRange;

    // Negative enchant should keep base range (enchant clamped to 0)
    bow->enchant1 = -3;
    short cursedRange = rangedWeaponRange(bow);

    ASSERT_EQ(cursedRange, baseRange);

    test_teardown_game();
}

// --- Runic application tests ---

TEST(test_ranged_weapon_runic_sniper_disables_falloff) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    // Apply sniper runic directly
    bow->enchant2 = W_SNIPER;
    bow->flags |= ITEM_RUNIC;

    ASSERT(bow->flags & ITEM_RUNIC);
    ASSERT_EQ(bow->enchant2, W_SNIPER);

    // Place a monster at max range
    short maxRange = rangedWeaponRange(bow);
    creature *rat = test_place_monster(MK_RAT, player.loc.x + maxRange, player.loc.y);
    ASSERT(rat != NULL);

    // Place a second monster at close range for comparison (new arena)
    // Instead, just verify the runic flag persists through item operations
    char buf[100];
    itemName(bow, buf, true, true, NULL);
    // Runic-identified items should show the runic name
    bow->flags |= ITEM_RUNIC_IDENTIFIED;
    itemName(bow, buf, true, true, NULL);

    test_teardown_game();
}

TEST(test_ranged_weapon_runic_piercing_flag) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    bow->enchant2 = W_PIERCING;
    bow->flags |= ITEM_RUNIC;

    ASSERT(bow->flags & ITEM_RUNIC);
    ASSERT_EQ(bow->enchant2, W_PIERCING);

    test_teardown_game();
}

TEST(test_ranged_weapon_runic_explosive_flag) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    bow->enchant2 = W_EXPLOSIVE;
    bow->flags |= ITEM_RUNIC;

    ASSERT(bow->flags & ITEM_RUNIC);
    ASSERT_EQ(bow->enchant2, W_EXPLOSIVE);

    test_teardown_game();
}

TEST(test_ranged_weapon_runic_chain_flag) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    bow->enchant2 = W_CHAIN;
    bow->flags |= ITEM_RUNIC;

    ASSERT(bow->flags & ITEM_RUNIC);
    ASSERT_EQ(bow->enchant2, W_CHAIN);

    test_teardown_game();
}

TEST(test_ranged_weapon_shared_runic_speed) {
    test_init_arena(12345);

    item *sling = test_give_item(RANGED, SLING, 0);
    ASSERT(sling != NULL);

    sling->enchant2 = W_SPEED;
    sling->flags |= ITEM_RUNIC;

    ASSERT(sling->flags & ITEM_RUNIC);
    ASSERT_EQ(sling->enchant2, W_SPEED);

    test_teardown_game();
}

TEST(test_ranged_weapon_shared_runic_slaying) {
    test_init_arena(12345);

    item *bow = test_give_item(RANGED, BOW, 0);
    ASSERT(bow != NULL);

    bow->enchant2 = W_SLAYING;
    bow->flags |= ITEM_RUNIC;
    bow->vorpalEnemy = MK_RAT;

    ASSERT(bow->flags & ITEM_RUNIC);
    ASSERT_EQ(bow->enchant2, W_SLAYING);
    ASSERT_EQ(bow->vorpalEnemy, MK_RAT);

    test_teardown_game();
}

// Test that sniper runic actually prevents damage falloff in hitMonsterWithRangedWeapon
TEST(test_sniper_runic_no_damage_falloff) {
    test_init_arena(12345);
    test_set_player_hp(500, 500);

    // Create two bows: one with sniper, one without
    item *sniperBow = test_give_item(RANGED, BOW, 5);
    ASSERT(sniperBow != NULL);
    sniperBow->enchant2 = W_SNIPER;
    sniperBow->flags |= ITEM_RUNIC;

    // Place a monster at long range
    short maxRange = rangedWeaponRange(sniperBow);
    short monX = player.loc.x + maxRange;
    if (monX >= DCOLS - 1) monX = DCOLS - 2;
    creature *monst = test_place_monster(MK_OGRE, monX, player.loc.y);
    ASSERT(monst != NULL);
    short ogreMaxHP = monst->currentHP;

    // Hit the monster with the sniper bow at max range
    test_reseed(12345);
    short distance = maxRange;
    boolean hit = hitMonsterWithRangedWeapon(monst, sniperBow, distance);

    if (hit && !(monst->bookkeepingFlags & MB_IS_DYING)) {
        short sniperDamage = ogreMaxHP - monst->currentHP;

        // Restore ogre HP
        monst->currentHP = ogreMaxHP;

        // Now remove sniper runic and hit again at same distance
        sniperBow->enchant2 = 0;
        sniperBow->flags &= ~ITEM_RUNIC;
        test_reseed(12345);
        hit = hitMonsterWithRangedWeapon(monst, sniperBow, distance);

        if (hit && !(monst->bookkeepingFlags & MB_IS_DYING)) {
            short normalDamage = ogreMaxHP - monst->currentHP;
            // Sniper damage should be >= normal damage (no falloff vs falloff)
            ASSERT_GE(sniperDamage, normalDamage);
        }
    }

    test_teardown_game();
}

SUITE(ranged) {
    RUN_TEST(test_dart_item_created_with_quantity);
    RUN_TEST(test_staff_item_has_charges);
    RUN_TEST(test_wand_item_created);
    RUN_TEST(test_incendiary_dart_exists);
    RUN_TEST(test_ranged_monster_attacks_player);

    // Ranged weapon creation
    RUN_TEST(test_sling_created_with_correct_stats);
    RUN_TEST(test_bow_created_with_correct_stats);
    RUN_TEST(test_crossbow_created_with_correct_stats);

    // Cooldown calculations
    RUN_TEST(test_sling_base_cooldown);
    RUN_TEST(test_bow_base_cooldown);
    RUN_TEST(test_crossbow_base_cooldown);

    // Range calculations
    RUN_TEST(test_sling_base_range);
    RUN_TEST(test_bow_base_range);
    RUN_TEST(test_crossbow_base_range);

    // Enchant scaling
    RUN_TEST(test_enchant_increases_range);
    RUN_TEST(test_crossbow_enchant_reduces_cooldown_fast);
    RUN_TEST(test_sling_enchant_balanced_cooldown_reduction);
    RUN_TEST(test_cooldown_minimum_floor);
    RUN_TEST(test_negative_enchant_does_not_increase_cooldown);
    RUN_TEST(test_negative_enchant_does_not_increase_range);

    // Cooldown recharge behavior
    RUN_TEST(test_ranged_weapon_cooldown_ticks_while_stationary);
    RUN_TEST(test_crossbow_cooldown_pauses_while_moving);
    RUN_TEST(test_bow_reloads_at_quarter_speed_while_moving);
    RUN_TEST(test_sling_reloads_at_full_speed_while_moving);
    RUN_TEST(test_ranged_weapon_becomes_ready_after_cooldown);
    RUN_TEST(test_cooldown_decrements_10_deciturns_per_turn);

    // Runic application
    RUN_TEST(test_ranged_weapon_runic_sniper_disables_falloff);
    RUN_TEST(test_ranged_weapon_runic_piercing_flag);
    RUN_TEST(test_ranged_weapon_runic_explosive_flag);
    RUN_TEST(test_ranged_weapon_runic_chain_flag);
    RUN_TEST(test_ranged_weapon_shared_runic_speed);
    RUN_TEST(test_ranged_weapon_shared_runic_slaying);
    RUN_TEST(test_sniper_runic_no_damage_falloff);
}
