// test_room_designs.c — Tests for new room design types (8-17)

#include "test_harness.h"

// Count floor cells (value == 1) in a grid
static int countFloorCells(short **grid) {
    int count = 0;
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (grid[x][y] == 1) {
                count++;
            }
        }
    }
    return count;
}

// Flood fill from (startX, startY) on cells with value == 1, marking visited as 2.
// Returns number of cells reached. Uses the engine's floodFillGrid.
static int testFloodFill(short **grid, int startX, int startY) {
    if (startX < 0 || startX >= DCOLS || startY < 0 || startY >= DROWS) return 0;
    if (grid[startX][startY] != 1) return 0;
    return (int)floodFillGrid(grid, startX, startY, 1, 1, 2);
}

// Find first floor cell in grid
static boolean findFloorCell(short **grid, short *outX, short *outY) {
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            if (grid[x][y] == 1) {
                *outX = x;
                *outY = y;
                return true;
            }
        }
    }
    return false;
}

// Verify a room design: has floor cells within bounds, and all floor cells are contiguous.
// Uses a copy of the grid for flood fill so the original is untouched.
static void verifyRoomDesign(short **grid, int minCells, int maxCells, const char *roomName) {
    int totalFloor = countFloorCells(grid);

    // Check floor count is within expected bounds
    if (totalFloor < minCells) {
        printf("    %s: too few floor cells: %d (min %d)\n", roomName, totalFloor, minCells);
        ASSERT(totalFloor >= minCells);
        return;
    }
    if (totalFloor > maxCells) {
        printf("    %s: too many floor cells: %d (max %d)\n", roomName, totalFloor, maxCells);
        ASSERT(totalFloor <= maxCells);
        return;
    }

    // Check no floor cells are on the grid boundary (leave margin)
    for (int x = 0; x < DCOLS; x++) {
        if (grid[x][0] == 1 || grid[x][DROWS - 1] == 1) {
            printf("    %s: floor cell on vertical boundary at x=%d\n", roomName, x);
            ASSERT(false);
            return;
        }
    }
    for (int y = 0; y < DROWS; y++) {
        if (grid[0][y] == 1 || grid[DCOLS - 1][y] == 1) {
            printf("    %s: floor cell on horizontal boundary at y=%d\n", roomName, y);
            ASSERT(false);
            return;
        }
    }

    // Check contiguity via flood fill on a copy
    short **copy = allocGrid();
    copyGrid(copy, grid);
    short startX, startY;
    if (findFloorCell(copy, &startX, &startY)) {
        int reached = testFloodFill(copy, startX, startY);
        if (reached != totalFloor) {
            printf("    %s: not contiguous! flood reached %d of %d cells\n", roomName, reached, totalFloor);
        }
        ASSERT_EQ(reached, totalFloor);
    }
    freeGrid(copy);
}

// Helper: generate a room of a given type and verify it
static void testRoomType(int roomType, int minCells, int maxCells, const char *name) {
    test_init_arena(42 + roomType);

    short **grid = allocGrid();
    designRoomOfType(grid, roomType);
    verifyRoomDesign(grid, minCells, maxCells, name);
    freeGrid(grid);

    test_teardown_game();
}

// Helper: test a room type with multiple seeds to catch edge cases
static void testRoomTypeMultiSeeds(int roomType, int minCells, int maxCells, const char *name) {
    for (int seed = 100; seed < 110; seed++) {
        test_init_arena(seed);

        short **grid = allocGrid();
        designRoomOfType(grid, roomType);

        int floorCount = countFloorCells(grid);
        if (floorCount < minCells || floorCount > maxCells) {
            printf("    %s (seed %d): floor count %d not in [%d, %d]\n",
                   name, seed, floorCount, minCells, maxCells);
        }
        ASSERT_GE(floorCount, minCells);
        ASSERT_LE(floorCount, maxCells);

        // Check contiguity
        short **copy = allocGrid();
        copyGrid(copy, grid);
        short sx, sy;
        if (findFloorCell(copy, &sx, &sy)) {
            int reached = testFloodFill(copy, sx, sy);
            if (reached != floorCount) {
                printf("    %s (seed %d): not contiguous, reached %d of %d\n",
                       name, seed, reached, floorCount);
            }
            ASSERT_EQ(reached, floorCount);
        }
        freeGrid(copy);

        freeGrid(grid);
        test_teardown_game();
    }
}

TEST(test_lshaped_room) {
    testRoomType(8, 15, 200, "L-shaped");
}

TEST(test_lshaped_room_multi) {
    testRoomTypeMultiSeeds(8, 15, 200, "L-shaped");
}

TEST(test_pillared_hall) {
    testRoomType(9, 10, 200, "Pillared hall");
}

TEST(test_pillared_hall_multi) {
    testRoomTypeMultiSeeds(9, 10, 200, "Pillared hall");
}

TEST(test_nested_room) {
    testRoomType(10, 15, 200, "Nested/donut");
}

TEST(test_nested_room_multi) {
    testRoomTypeMultiSeeds(10, 15, 200, "Nested/donut");
}

TEST(test_trefoil_room) {
    testRoomType(11, 10, 150, "Trefoil");
}

TEST(test_trefoil_room_multi) {
    testRoomTypeMultiSeeds(11, 10, 150, "Trefoil");
}

TEST(test_cathedral_room) {
    testRoomType(12, 15, 300, "Cathedral");
}

TEST(test_cathedral_room_multi) {
    testRoomTypeMultiSeeds(12, 15, 300, "Cathedral");
}

TEST(test_alcove_room) {
    testRoomType(13, 15, 200, "Alcove");
}

TEST(test_alcove_room_multi) {
    testRoomTypeMultiSeeds(13, 15, 200, "Alcove");
}

TEST(test_zshaped_room) {
    testRoomType(14, 15, 200, "Z-shaped");
}

TEST(test_zshaped_room_multi) {
    testRoomTypeMultiSeeds(14, 15, 200, "Z-shaped");
}

TEST(test_tshaped_room) {
    testRoomType(15, 15, 200, "T-shaped");
}

TEST(test_tshaped_room_multi) {
    testRoomTypeMultiSeeds(15, 15, 200, "T-shaped");
}

TEST(test_diamond_room) {
    testRoomType(16, 10, 100, "Diamond");
}

TEST(test_diamond_room_multi) {
    testRoomTypeMultiSeeds(16, 10, 100, "Diamond");
}

TEST(test_dumbbell_room) {
    testRoomType(17, 10, 200, "Dumbbell");
}

TEST(test_dumbbell_room_multi) {
    testRoomTypeMultiSeeds(17, 10, 200, "Dumbbell");
}

SUITE(room_designs) {
    RUN_TEST(test_lshaped_room);
    RUN_TEST(test_lshaped_room_multi);
    RUN_TEST(test_pillared_hall);
    RUN_TEST(test_pillared_hall_multi);
    RUN_TEST(test_nested_room);
    RUN_TEST(test_nested_room_multi);
    RUN_TEST(test_trefoil_room);
    RUN_TEST(test_trefoil_room_multi);
    RUN_TEST(test_cathedral_room);
    RUN_TEST(test_cathedral_room_multi);
    RUN_TEST(test_alcove_room);
    RUN_TEST(test_alcove_room_multi);
    RUN_TEST(test_zshaped_room);
    RUN_TEST(test_zshaped_room_multi);
    RUN_TEST(test_tshaped_room);
    RUN_TEST(test_tshaped_room_multi);
    RUN_TEST(test_diamond_room);
    RUN_TEST(test_diamond_room_multi);
    RUN_TEST(test_dumbbell_room);
    RUN_TEST(test_dumbbell_room_multi);
}
