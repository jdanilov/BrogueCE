// test_combat.c — Tests for combat mechanics

#include "test_harness.h"

TEST(test_attack_monster_reduces_hp) {
    test_init_game(12345);

    // Find open floor adjacent to player for monster placement
    pos monsterPos;
    boolean placed = false;
    for (short dir = 0; dir < DIRECTION_COUNT && !placed; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)
            && !(pmap[nx][ny].flags & (HAS_MONSTER | HAS_STAIRS))) {
            monsterPos.x = nx;
            monsterPos.y = ny;
            placed = true;
        }
    }
    ASSERT(placed);

    // Place a rat (weak monster) adjacent to the player
    creature *rat = test_place_monster(MK_RAT, monsterPos.x, monsterPos.y);
    ASSERT(rat != NULL);
    short ratStartHP = rat->currentHP;
    ASSERT_GT(ratStartHP, 0);

    // Determine direction to the rat
    short dir = -1;
    for (short d = 0; d < DIRECTION_COUNT; d++) {
        if (player.loc.x + nbDirs[d][0] == monsterPos.x
            && player.loc.y + nbDirs[d][1] == monsterPos.y) {
            dir = d;
            break;
        }
    }
    ASSERT_GE(dir, 0);

    // Move into the rat (should attack)
    test_move(dir);

    // Rat should have taken damage or died
    creature *ratAfter = test_monster_at(monsterPos.x, monsterPos.y);
    if (ratAfter) {
        ASSERT_LT(ratAfter->currentHP, ratStartHP);
    }
    // If ratAfter is NULL, the rat died — also a valid outcome

    test_teardown_game();
}

TEST(test_player_takes_damage_from_monster) {
    test_init_arena(12345);

    // Place an ogre (hits hard) adjacent to the player
    creature *ogre = test_place_monster(MK_OGRE, player.loc.x + 1, player.loc.y);
    ASSERT(ogre != NULL);

    // Boost HP to survive multiple hits
    test_set_player_hp(200, 200);
    short startHP = player.currentHP;

    // Rest several turns to let the ogre attack us
    for (int i = 0; i < 8 && !rogue.gameHasEnded; i++) {
        test_rest();
    }

    // Player should have taken some damage (or died)
    if (!rogue.gameHasEnded) {
        ASSERT_LT(player.currentHP, startHP);
    }

    test_teardown_game();
}

TEST(test_killing_monster_removes_it) {
    test_init_game(12345);

    pos monsterPos;
    boolean placed = false;
    for (short dir = 0; dir < DIRECTION_COUNT && !placed; dir++) {
        short nx = player.loc.x + nbDirs[dir][0];
        short ny = player.loc.y + nbDirs[dir][1];
        if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS
            && !cellHasTerrainFlag((pos){ nx, ny }, T_OBSTRUCTS_PASSABILITY)
            && !(pmap[nx][ny].flags & (HAS_MONSTER | HAS_STAIRS))) {
            monsterPos.x = nx;
            monsterPos.y = ny;
            placed = true;
        }
    }
    ASSERT(placed);

    // Place a rat and weaken it to 1 HP
    creature *rat = test_place_monster(MK_RAT, monsterPos.x, monsterPos.y);
    ASSERT(rat != NULL);
    rat->currentHP = 1;

    // Determine direction
    short dir = -1;
    for (short d = 0; d < DIRECTION_COUNT; d++) {
        if (player.loc.x + nbDirs[d][0] == monsterPos.x
            && player.loc.y + nbDirs[d][1] == monsterPos.y) {
            dir = d;
            break;
        }
    }
    ASSERT_GE(dir, 0);

    // Attack should kill it
    test_move(dir);

    // The rat should be gone from the map
    creature *remains = test_monster_at(monsterPos.x, monsterPos.y);
    ASSERT(remains == NULL);
    ASSERT(!(pmap[monsterPos.x][monsterPos.y].flags & HAS_MONSTER));

    test_teardown_game();
}

SUITE(combat) {
    RUN_TEST(test_attack_monster_reduces_hp);
    RUN_TEST(test_player_takes_damage_from_monster);
    RUN_TEST(test_killing_monster_removes_it);
}
