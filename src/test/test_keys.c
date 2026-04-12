// test_keys.c — Tests for keys and locked doors

#include "test_harness.h"

TEST(test_key_item_creation) {
    test_init_game(42);

    item *key = test_give_item(KEY, KEY_DOOR, 0);
    ASSERT(key != NULL);
    ASSERT_EQ(key->category, KEY);
    ASSERT_EQ(key->kind, KEY_DOOR);

    test_teardown_game();
}

TEST(test_key_cage_creation) {
    test_init_game(42);

    item *key = test_give_item(KEY, KEY_CAGE, 0);
    ASSERT(key != NULL);
    ASSERT_EQ(key->category, KEY);
    ASSERT_EQ(key->kind, KEY_CAGE);

    test_teardown_game();
}

TEST(test_key_portal_creation) {
    test_init_game(42);

    item *key = test_give_item(KEY, KEY_PORTAL, 0);
    ASSERT(key != NULL);
    ASSERT_EQ(key->category, KEY);
    ASSERT_EQ(key->kind, KEY_PORTAL);

    test_teardown_game();
}

TEST(test_key_item_in_pack) {
    test_init_game(42);

    item *key = test_give_item(KEY, KEY_DOOR, 0);
    ASSERT(key != NULL);

    // Verify the key appears in the player's pack
    boolean found = false;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        if (it == key) {
            found = true;
            break;
        }
    }
    ASSERT(found);

    test_teardown_game();
}

TEST(test_key_is_identified_by_default) {
    test_init_game(42);

    // Keys are in the NEVER_IDENTIFIABLE / PRENAMED_CATEGORY group,
    // and test_give_item sets ITEM_IDENTIFIED. Verify the flag is set.
    item *key = test_give_item(KEY, KEY_DOOR, 0);
    ASSERT(key != NULL);
    ASSERT(key->flags & ITEM_IDENTIFIED);

    test_teardown_game();
}

TEST(test_locked_door_terrain_blocks_movement) {
    test_init_game(42);
    test_set_player_hp(500, 500);
    test_remove_all_monsters();

    // Find a clear area around the player and set up a locked door adjacent
    short px = player.loc.x;
    short py = player.loc.y;

    // Clear the area around the player to have a controlled environment
    test_clear_area(px, py, 3);

    // Find a passable direction to place the locked door
    short targetX = -1, targetY = -1;
    short moveDir = -1;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = px + nbDirs[dir][0];
        short ny = py + nbDirs[dir][1];
        if (nx > 0 && nx < DCOLS - 1 && ny > 0 && ny < DROWS - 1) {
            targetX = nx;
            targetY = ny;
            moveDir = dir;
            break;
        }
    }
    ASSERT(moveDir >= 0);

    // Place a LOCKED_DOOR on the DUNGEON layer at the adjacent cell
    // LOCKED_DOOR has T_OBSTRUCTS_EVERYTHING which includes T_OBSTRUCTS_PASSABILITY
    test_set_terrain(targetX, targetY, DUNGEON, LOCKED_DOOR);

    // Verify the cell has the passability-blocking flag
    ASSERT(test_cell_has_terrain_flag(targetX, targetY, T_OBSTRUCTS_PASSABILITY));

    // Try to move into the locked door — player should not pass through
    test_move(moveDir);

    // Player should still be at the original position
    ASSERT_EQ(player.loc.x, px);
    ASSERT_EQ(player.loc.y, py);

    test_teardown_game();
}

TEST(test_locked_door_obstructs_vision) {
    test_init_game(42);
    test_set_player_hp(500, 500);
    test_remove_all_monsters();

    short px = player.loc.x;
    short py = player.loc.y;
    test_clear_area(px, py, 3);

    // Place a locked door adjacent to the player
    short nx = px + 1;
    short ny = py;
    if (nx < DCOLS - 1) {
        test_set_terrain(nx, ny, DUNGEON, LOCKED_DOOR);

        // LOCKED_DOOR has T_OBSTRUCTS_EVERYTHING which includes T_OBSTRUCTS_VISION
        ASSERT(test_cell_has_terrain_flag(nx, ny, T_OBSTRUCTS_VISION));
    }

    test_teardown_game();
}

TEST(test_wooden_door_opens_on_step) {
    test_init_game(42);
    test_set_player_hp(500, 500);
    test_remove_all_monsters();

    short px = player.loc.x;
    short py = player.loc.y;
    test_clear_area(px, py, 3);

    // Find a valid adjacent cell for the door
    short targetX = -1, targetY = -1;
    short moveDir = -1;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = px + nbDirs[dir][0];
        short ny = py + nbDirs[dir][1];
        if (nx > 0 && nx < DCOLS - 1 && ny > 0 && ny < DROWS - 1) {
            targetX = nx;
            targetY = ny;
            moveDir = dir;
            break;
        }
    }
    ASSERT(moveDir >= 0);

    // Place a regular wooden DOOR — it has TM_PROMOTES_ON_STEP so the player
    // should be able to walk through it (it promotes to OPEN_DOOR).
    // A wooden DOOR does NOT have T_OBSTRUCTS_PASSABILITY.
    test_set_terrain(targetX, targetY, DUNGEON, DOOR);

    // Verify the wooden door does not block passability
    ASSERT(!test_cell_has_terrain_flag(targetX, targetY, T_OBSTRUCTS_PASSABILITY));

    // Move into the door — player should pass through
    test_move(moveDir);

    ASSERT_EQ(player.loc.x, targetX);
    ASSERT_EQ(player.loc.y, targetY);

    test_teardown_game();
}

TEST(test_open_iron_door_allows_passage) {
    test_init_game(42);
    test_set_player_hp(500, 500);
    test_remove_all_monsters();

    short px = player.loc.x;
    short py = player.loc.y;
    test_clear_area(px, py, 3);

    short targetX = -1, targetY = -1;
    short moveDir = -1;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = px + nbDirs[dir][0];
        short ny = py + nbDirs[dir][1];
        if (nx > 0 && nx < DCOLS - 1 && ny > 0 && ny < DROWS - 1) {
            targetX = nx;
            targetY = ny;
            moveDir = dir;
            break;
        }
    }
    ASSERT(moveDir >= 0);

    // An OPEN_IRON_DOOR_INERT does not obstruct passability
    test_set_terrain(targetX, targetY, DUNGEON, OPEN_IRON_DOOR_INERT);

    ASSERT(!test_cell_has_terrain_flag(targetX, targetY, T_OBSTRUCTS_PASSABILITY));

    // Move into the open iron door — player should pass through
    test_move(moveDir);

    ASSERT_EQ(player.loc.x, targetX);
    ASSERT_EQ(player.loc.y, targetY);

    test_teardown_game();
}

TEST(test_multiple_keys_in_pack) {
    test_init_game(42);

    // Give three different keys
    item *key1 = test_give_item(KEY, KEY_DOOR, 0);
    item *key2 = test_give_item(KEY, KEY_CAGE, 0);
    item *key3 = test_give_item(KEY, KEY_PORTAL, 0);

    ASSERT(key1 != NULL);
    ASSERT(key2 != NULL);
    ASSERT(key3 != NULL);

    // Count keys in pack
    int keyCount = 0;
    for (item *it = packItems->nextItem; it != NULL; it = it->nextItem) {
        if (it->category == KEY) {
            keyCount++;
        }
    }
    ASSERT_GE(keyCount, 3);

    test_teardown_game();
}

TEST(test_secret_door_blocks_movement) {
    test_init_game(42);
    test_set_player_hp(500, 500);
    test_remove_all_monsters();

    short px = player.loc.x;
    short py = player.loc.y;
    test_clear_area(px, py, 3);

    short targetX = -1, targetY = -1;
    short moveDir = -1;
    for (short dir = 0; dir < DIRECTION_COUNT; dir++) {
        short nx = px + nbDirs[dir][0];
        short ny = py + nbDirs[dir][1];
        if (nx > 0 && nx < DCOLS - 1 && ny > 0 && ny < DROWS - 1) {
            targetX = nx;
            targetY = ny;
            moveDir = dir;
            break;
        }
    }
    ASSERT(moveDir >= 0);

    // SECRET_DOOR has T_OBSTRUCTS_EVERYTHING — it looks like a wall
    test_set_terrain(targetX, targetY, DUNGEON, SECRET_DOOR);

    ASSERT(test_cell_has_terrain_flag(targetX, targetY, T_OBSTRUCTS_PASSABILITY));

    // Player should not be able to walk through a secret door
    test_move(moveDir);

    ASSERT_EQ(player.loc.x, px);
    ASSERT_EQ(player.loc.y, py);

    test_teardown_game();
}

SUITE(keys) {
    RUN_TEST(test_key_item_creation);
    RUN_TEST(test_key_cage_creation);
    RUN_TEST(test_key_portal_creation);
    RUN_TEST(test_key_item_in_pack);
    RUN_TEST(test_key_is_identified_by_default);
    RUN_TEST(test_locked_door_terrain_blocks_movement);
    RUN_TEST(test_locked_door_obstructs_vision);
    RUN_TEST(test_wooden_door_opens_on_step);
    RUN_TEST(test_open_iron_door_allows_passage);
    RUN_TEST(test_multiple_keys_in_pack);
    RUN_TEST(test_secret_door_blocks_movement);
}
