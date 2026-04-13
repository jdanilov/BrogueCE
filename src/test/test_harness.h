// test_harness.h — Minimal C test harness for BrogueCE
#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Test framework macros ----

#define TEST(name) static void name(void)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        test_fail(__FILE__, __LINE__, #cond); \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (_a != _b) { \
        test_fail_eq(__FILE__, __LINE__, #a, #b, _a, _b); \
        return; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (_a == _b) { \
        test_fail_ne(__FILE__, __LINE__, #a, #b, _a); \
        return; \
    } \
} while(0)

#define ASSERT_GT(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (!(_a > _b)) { \
        test_fail_cmp(__FILE__, __LINE__, #a, ">", #b, _a, _b); \
        return; \
    } \
} while(0)

#define ASSERT_GE(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (!(_a >= _b)) { \
        test_fail_cmp(__FILE__, __LINE__, #a, ">=", #b, _a, _b); \
        return; \
    } \
} while(0)

#define ASSERT_LT(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (!(_a < _b)) { \
        test_fail_cmp(__FILE__, __LINE__, #a, "<", #b, _a, _b); \
        return; \
    } \
} while(0)

#define ASSERT_LE(a, b) do { \
    long _a = (long)(a), _b = (long)(b); \
    if (!(_a <= _b)) { \
        test_fail_cmp(__FILE__, __LINE__, #a, "<=", #b, _a, _b); \
        return; \
    } \
} while(0)

#define RUN_TEST(name) run_test(#name, name)

// ---- Test framework state ----

typedef struct {
    int passed;
    int failed;
    int current_failed;
} test_state;

extern test_state _test_state;

static inline void test_fail(const char *file, int line, const char *cond) {
    printf("  FAIL %s:%d: %s\n", file, line, cond);
    _test_state.current_failed = 1;
}

static inline void test_fail_eq(const char *file, int line,
    const char *a_str, const char *b_str, long a, long b) {
    printf("  FAIL %s:%d: %s == %s (got %ld vs %ld)\n", file, line, a_str, b_str, a, b);
    _test_state.current_failed = 1;
}

static inline void test_fail_ne(const char *file, int line,
    const char *a_str, const char *b_str, long val) {
    printf("  FAIL %s:%d: %s != %s (both are %ld)\n", file, line, a_str, b_str, val);
    _test_state.current_failed = 1;
}

static inline void test_fail_cmp(const char *file, int line,
    const char *a_str, const char *op, const char *b_str, long a, long b) {
    printf("  FAIL %s:%d: %s %s %s (got %ld vs %ld)\n", file, line, a_str, op, b_str, a, b);
    _test_state.current_failed = 1;
}

static inline void run_test(const char *name, void (*fn)(void)) {
    _test_state.current_failed = 0;
    fn();
    if (_test_state.current_failed) {
        _test_state.failed++;
        printf("  FAIL  %s\n", name);
    } else {
        _test_state.passed++;
        printf("  OK    %s\n", name);
    }
}

// ---- Test suite registration ----

typedef void (*test_suite_fn)(void);

#define SUITE(name) \
    void suite_##name(void); \
    void suite_##name(void)
#define RUN_SUITE(name) run_suite(#name, suite_##name)

static inline void run_suite(const char *name, test_suite_fn fn) {
    printf("\n=== %s ===\n", name);
    fn();
}

// ---- Game helpers ----

// Initialize a game with a given seed. Sets up variant, calls initializeRogue + startLevel.
void test_init_game(uint64_t seed);

// Tear down the current game, freeing allocated resources.
void test_teardown_game(void);

// Send a keystroke event.
void test_send_key(signed long key, boolean ctrl, boolean shift);

// Move the player in a direction (UP, DOWN, LEFT, RIGHT, UPLEFT, etc.)
void test_move(short direction);

// Rest for one turn.
void test_rest(void);

// Rest for N turns.
void test_rest_turns(int n);

// Get the creature at a given position, or NULL.
creature *test_monster_at(short x, short y);

// Count living monsters on the current level.
int test_count_monsters(void);

// Check if a cell has a specific terrain flag on any layer.
boolean test_cell_has_terrain_flag(short x, short y, unsigned long flag);

// Place a monster directly at a position. Returns the creature.
creature *test_place_monster(short monsterID, short x, short y);

// Re-seed the RNG to a known state. Call after test_init_game + arena setup
// to decouple the test's gameplay RNG from dungeon generation.
// This makes tests immune to changes in item/monster tables.
void test_reseed(uint64_t seed);

// Initialize a blank arena (no dungeon generation). Creates a walled rectangle
// of open floor with the player at center. The RNG is seeded to `seed` and is
// completely independent of item/monster tables. Use this instead of
// test_init_game for tests that build their own controlled scenario.
void test_init_arena(uint64_t seed);

// Teleport the player to (x, y). Clears HAS_PLAYER from old cell, sets it on new cell.
void test_teleport_player(short x, short y);

// Create an item and add it to the player's pack. Returns the item.
item *test_give_item(unsigned short category, short kind, short enchant);

// Set player HP directly.
void test_set_player_hp(short hp, short maxHp);

// Apply a status effect to the player.
void test_set_player_status(short statusType, short value);

// Find an open floor cell near (x, y). Returns true and sets *out if found.
boolean test_find_open_floor(short nearX, short nearY, pos *out);

// ---- Arena / setup helpers ----

// Clear a rectangular area to open floor, removing all terrain, monsters, and items.
// Replaces DUNGEON layer with FLOOR, clears LIQUID/GAS/SURFACE to NOTHING.
// Kills any monsters in the area and clears cell flags.
void test_clear_area(short centerX, short centerY, short radius);

// Kill and remove all monsters on the current level.
void test_remove_all_monsters(void);

// Set terrain on a specific layer at (x, y).
void test_set_terrain(short x, short y, enum dungeonLayers layer, enum tileType terrain);

// Run N environment update ticks (fire spread, gas dissipation, tile promotions).
// Does NOT process monster or player turns.
void test_advance_environment(int ticks);

#endif // TEST_HARNESS_H
