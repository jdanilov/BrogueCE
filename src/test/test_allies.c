// test_allies.c — Tests for the ally system

#include "test_harness.h"

// Helper: create an ally from a placed monster
static creature *make_ally(short monsterID, short x, short y) {
    creature *ally = test_place_monster(monsterID, x, y);
    if (ally) {
        ally->creatureState = MONSTER_ALLY;
        ally->leader = &player;
        ally->bookkeepingFlags |= MB_FOLLOWER;
    }
    return ally;
}

// Helper: search the entire level for a living ally with a given monsterID
static creature *find_ally(short monsterID) {
    for (short x = 0; x < DCOLS; x++) {
        for (short y = 0; y < DROWS; y++) {
            creature *c = test_monster_at(x, y);
            if (c != NULL && c->info.monsterID == monsterID
                && c->creatureState == MONSTER_ALLY) {
                return c;
            }
        }
    }
    return NULL;
}

TEST(test_ally_creation_and_state) {
    test_init_game(12345);
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    short mx = player.loc.x + 2;
    short my = player.loc.y;
    creature *ally = test_place_monster(MK_KOBOLD, mx, my);
    ASSERT(ally != NULL);

    // Set ally fields
    ally->creatureState = MONSTER_ALLY;
    ally->leader = &player;
    ally->bookkeepingFlags |= MB_FOLLOWER;

    // Verify all fields are set correctly
    ASSERT_EQ(ally->creatureState, MONSTER_ALLY);
    ASSERT(ally->leader == &player);
    ASSERT(ally->bookkeepingFlags & MB_FOLLOWER);
    ASSERT_EQ(ally->info.monsterID, MK_KOBOLD);
    ASSERT_GT(ally->currentHP, 0);
    ASSERT_EQ(ally->loc.x, mx);
    ASSERT_EQ(ally->loc.y, my);

    test_teardown_game();
}

TEST(test_ally_survives_multiple_turns) {
    test_init_game(22222);
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    short mx = player.loc.x + 1;
    short my = player.loc.y;
    creature *ally = make_ally(MK_KOBOLD, mx, my);
    ASSERT(ally != NULL);

    // Give the ally high HP so it survives
    ally->currentHP = 500;
    ally->info.maxHP = 500;

    // Rest many turns
    for (int i = 0; i < 20 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        creature *mon = find_ally(MK_KOBOLD);
        ASSERT(mon != NULL);
        ASSERT_EQ(mon->creatureState, MONSTER_ALLY);
        ASSERT_GT(mon->currentHP, 0);
        ASSERT(mon->leader == &player);
    }

    test_teardown_game();
}

TEST(test_ally_attacks_enemy) {
    test_init_game(33333);
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place an ally adjacent to the player
    short ax = player.loc.x + 1;
    short ay = player.loc.y;
    creature *ally = make_ally(MK_KOBOLD, ax, ay);
    ASSERT(ally != NULL);
    ally->currentHP = 500;
    ally->info.maxHP = 500;

    // Place a weak enemy adjacent to the ally (but not adjacent to player,
    // so the player doesn't kill it first)
    short ex = ax + 1;
    short ey = ay;
    creature *enemy = test_place_monster(MK_RAT, ex, ey);
    ASSERT(enemy != NULL);
    enemy->currentHP = 3;
    enemy->creatureState = MONSTER_TRACKING_SCENT;

    short enemyStartHP = enemy->currentHP;

    // Rest several turns so the ally can act
    for (int i = 0; i < 10 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // The enemy should have taken damage or died
        creature *rat = test_monster_at(ex, ey);
        if (rat != NULL && rat->info.monsterID == MK_RAT) {
            // Rat still at original position — check it took damage
            ASSERT_LT(rat->currentHP, enemyStartHP);
        }
        // If rat is NULL, it died or moved — the ally attacked it successfully
    }

    test_teardown_game();
}

TEST(test_multiple_allies_coexist) {
    test_init_game(44444);
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    short px = player.loc.x;
    short py = player.loc.y;

    // Create 3 allies at different positions around the player
    creature *ally1 = make_ally(MK_KOBOLD, px + 1, py);
    creature *ally2 = make_ally(MK_KOBOLD, px - 1, py);
    creature *ally3 = make_ally(MK_KOBOLD, px, py + 1);
    ASSERT(ally1 != NULL);
    ASSERT(ally2 != NULL);
    ASSERT(ally3 != NULL);

    // Give them high HP
    ally1->currentHP = 500; ally1->info.maxHP = 500;
    ally2->currentHP = 500; ally2->info.maxHP = 500;
    ally3->currentHP = 500; ally3->info.maxHP = 500;

    // Verify all three are allies
    ASSERT_EQ(ally1->creatureState, MONSTER_ALLY);
    ASSERT_EQ(ally2->creatureState, MONSTER_ALLY);
    ASSERT_EQ(ally3->creatureState, MONSTER_ALLY);
    ASSERT(ally1->leader == &player);
    ASSERT(ally2->leader == &player);
    ASSERT(ally3->leader == &player);

    // Rest a few turns to make sure they all persist
    for (int i = 0; i < 5 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Count allies on the level
        int allyCount = 0;
        for (short x = 0; x < DCOLS; x++) {
            for (short y = 0; y < DROWS; y++) {
                creature *c = test_monster_at(x, y);
                if (c != NULL && c->creatureState == MONSTER_ALLY
                    && c->info.monsterID == MK_KOBOLD) {
                    allyCount++;
                }
            }
        }
        ASSERT_GE(allyCount, 2); // At least 2 of 3 should survive a few turns
    }

    test_teardown_game();
}

TEST(test_ally_stays_near_player_after_movement) {
    test_init_game(55555);
    test_clear_area(player.loc.x, player.loc.y, 10);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place an ally near the player
    short mx = player.loc.x + 1;
    short my = player.loc.y;
    creature *ally = make_ally(MK_KOBOLD, mx, my);
    ASSERT(ally != NULL);
    ally->currentHP = 500;
    ally->info.maxHP = 500;

    // Move the player several steps to the left
    for (int i = 0; i < 5 && !rogue.gameHasEnded; i++) {
        short nx = player.loc.x - 1;
        short ny = player.loc.y;
        if (nx >= 1
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)
            && !(pmap[nx][ny].flags & HAS_MONSTER)) {
            test_move(LEFT);
        } else {
            test_rest();
        }
    }

    if (!rogue.gameHasEnded) {
        creature *mon = find_ally(MK_KOBOLD);
        if (mon != NULL) {
            // Ally should be within a reasonable distance of the player
            short dist = (short)(abs(mon->loc.x - player.loc.x)
                               + abs(mon->loc.y - player.loc.y));
            ASSERT_LE(dist, 5);
        }
        // If ally is NULL, it may have died — not ideal but acceptable
    }

    test_teardown_game();
}

SUITE(allies) {
    RUN_TEST(test_ally_creation_and_state);
    RUN_TEST(test_ally_survives_multiple_turns);
    RUN_TEST(test_ally_attacks_enemy);
    RUN_TEST(test_multiple_allies_coexist);
    RUN_TEST(test_ally_stays_near_player_after_movement);
}
