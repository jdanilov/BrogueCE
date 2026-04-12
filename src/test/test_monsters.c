// test_monsters.c — Tests for monster AI behavior

#include "test_harness.h"

TEST(test_sleeping_monster_wakes_when_adjacent) {
    test_init_game(12345);

    // Set up a controlled arena around the player
    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a rat adjacent to the player
    short mx = player.loc.x + 1;
    short my = player.loc.y;
    creature *rat = test_place_monster(MK_RAT, mx, my);
    ASSERT(rat != NULL);

    // Force it to sleep
    rat->creatureState = MONSTER_SLEEPING;
    ASSERT_EQ(rat->creatureState, MONSTER_SLEEPING);

    // Rest a couple turns — the monster should wake up due to player adjacency
    for (int i = 0; i < 3 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // The monster should have woken up (no longer sleeping)
        creature *mon = test_monster_at(mx, my);
        if (mon != NULL) {
            ASSERT_NE(mon->creatureState, MONSTER_SLEEPING);
        }
        // If mon is NULL, the rat moved or died — either way it woke up
    }

    test_teardown_game();
}

TEST(test_monster_moves_toward_player) {
    test_init_game(12345);

    // Set up arena and remove other monsters
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a rat 4 cells to the right, set it to tracking
    short mx = player.loc.x + 4;
    short my = player.loc.y;
    creature *rat = test_place_monster(MK_RAT, mx, my);
    ASSERT(rat != NULL);
    rat->creatureState = MONSTER_TRACKING_SCENT;

    short startDist = (short)(abs(mx - player.loc.x) + abs(my - player.loc.y));

    // Let the monster act for several turns
    for (int i = 0; i < 6 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Find the rat — it may have moved
        creature *mon = NULL;
        for (short x = 0; x < DCOLS && mon == NULL; x++) {
            for (short y = 0; y < DROWS && mon == NULL; y++) {
                creature *c = test_monster_at(x, y);
                if (c != NULL && c->info.monsterID == MK_RAT) {
                    mon = c;
                }
            }
        }

        if (mon != NULL) {
            short endDist = (short)(abs(mon->loc.x - player.loc.x)
                                  + abs(mon->loc.y - player.loc.y));
            // Monster should have moved closer or be adjacent (dist <= 1)
            ASSERT(endDist < startDist || endDist <= 1);
        }
        // If mon is NULL, the rat died somehow — acceptable
    }

    test_teardown_game();
}

TEST(test_ally_follows_player) {
    test_init_game(12345);

    // Set up arena
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a kobold near the player and make it an ally
    short mx = player.loc.x + 1;
    short my = player.loc.y;
    creature *ally = test_place_monster(MK_KOBOLD, mx, my);
    ASSERT(ally != NULL);
    ally->creatureState = MONSTER_ALLY;
    ally->leader = &player;
    ally->bookkeepingFlags |= MB_FOLLOWER;

    // Move the player several steps to the left
    for (int i = 0; i < 4 && !rogue.gameHasEnded; i++) {
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
        // Find the ally
        creature *mon = NULL;
        for (short x = 0; x < DCOLS && mon == NULL; x++) {
            for (short y = 0; y < DROWS && mon == NULL; y++) {
                creature *c = test_monster_at(x, y);
                if (c != NULL && c->info.monsterID == MK_KOBOLD
                    && c->creatureState == MONSTER_ALLY) {
                    mon = c;
                }
            }
        }

        if (mon != NULL) {
            // Ally should be within a few cells of the player
            short dist = (short)(abs(mon->loc.x - player.loc.x)
                               + abs(mon->loc.y - player.loc.y));
            ASSERT_LE(dist, 4);
        }
        // If ally is NULL, it may have died — not ideal but acceptable
    }

    test_teardown_game();
}

TEST(test_monster_attacks_when_adjacent) {
    test_init_game(12345);

    // Set up arena
    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place an ogre adjacent to the player, in tracking state
    short mx = player.loc.x + 1;
    short my = player.loc.y;
    creature *ogre = test_place_monster(MK_OGRE, mx, my);
    ASSERT(ogre != NULL);
    ogre->creatureState = MONSTER_TRACKING_SCENT;

    short startHP = player.currentHP;

    // Rest several turns to let the ogre attack
    for (int i = 0; i < 8 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        // Player should have taken damage from the ogre
        ASSERT_LT(player.currentHP, startHP);
    }

    test_teardown_game();
}

TEST(test_immobile_monster_does_not_move) {
    test_init_game(12345);

    // Set up arena far from player so the totem doesn't interact
    test_clear_area(player.loc.x, player.loc.y, 10);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place a goblin totem a few cells away from the player
    short mx = player.loc.x + 4;
    short my = player.loc.y;
    creature *totem = test_place_monster(MK_GOBLIN_TOTEM, mx, my);
    ASSERT(totem != NULL);

    // Verify it has the MONST_IMMOBILE flag
    ASSERT(totem->info.flags & MONST_IMMOBILE);

    short startX = totem->loc.x;
    short startY = totem->loc.y;

    // Rest many turns
    for (int i = 0; i < 10 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    if (!rogue.gameHasEnded) {
        creature *mon = test_monster_at(startX, startY);
        ASSERT(mon != NULL);
        ASSERT_EQ(mon->loc.x, startX);
        ASSERT_EQ(mon->loc.y, startY);
    }

    test_teardown_game();
}

TEST(test_monster_count_after_kill) {
    test_init_game(12345);

    // Clear everything
    test_clear_area(player.loc.x, player.loc.y, 8);
    test_remove_all_monsters();
    test_set_player_hp(500, 500);

    // Place exactly 3 rats
    short px = player.loc.x;
    short py = player.loc.y;

    creature *rat1 = test_place_monster(MK_RAT, px + 2, py);
    creature *rat2 = test_place_monster(MK_RAT, px + 3, py);
    creature *rat3 = test_place_monster(MK_RAT, px + 4, py);
    ASSERT(rat1 != NULL);
    ASSERT(rat2 != NULL);
    ASSERT(rat3 != NULL);

    int countBefore = test_count_monsters();
    ASSERT_EQ(countBefore, 3);

    // Place a 1-HP rat adjacent to player and kill it
    creature *weakRat = test_place_monster(MK_RAT, px + 1, py);
    ASSERT(weakRat != NULL);
    weakRat->currentHP = 1;

    ASSERT_EQ(test_count_monsters(), 4);

    // Attack the adjacent rat (move right into it)
    test_move(RIGHT);

    // The weak rat should be dead; count should be 3
    if (!rogue.gameHasEnded) {
        ASSERT_EQ(test_count_monsters(), 3);
    }

    test_teardown_game();
}

TEST(test_placed_monster_has_correct_type) {
    test_init_game(12345);

    test_clear_area(player.loc.x, player.loc.y, 5);
    test_remove_all_monsters();

    short mx = player.loc.x + 3;
    short my = player.loc.y;
    creature *kobold = test_place_monster(MK_KOBOLD, mx, my);
    ASSERT(kobold != NULL);
    ASSERT_EQ(kobold->info.monsterID, MK_KOBOLD);
    ASSERT_GT(kobold->currentHP, 0);
    ASSERT_GT(kobold->info.maxHP, 0);
    ASSERT_EQ(kobold->loc.x, mx);
    ASSERT_EQ(kobold->loc.y, my);

    test_teardown_game();
}

SUITE(monsters) {
    RUN_TEST(test_sleeping_monster_wakes_when_adjacent);
    RUN_TEST(test_monster_moves_toward_player);
    RUN_TEST(test_ally_follows_player);
    RUN_TEST(test_monster_attacks_when_adjacent);
    RUN_TEST(test_immobile_monster_does_not_move);
    RUN_TEST(test_monster_count_after_kill);
    RUN_TEST(test_placed_monster_has_correct_type);
}
