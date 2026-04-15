// scan_fixture_seeds.c — Generic fixture seed scanner.
//
// Uses rogue.placedMachines[] metadata recorded by buildAMachine() to find
// fixtures by blueprint index — no heuristic tile scanning needed.
//
// Build:
//   cd /opt/ed/BrogueCE
//   GAME_SRCS=$(find src/brogue src/variants -name '*.c') && \
//   cc -DDATADIR=. -DBROGUE_SDL -DBROGUE_EXTRA_VERSION='""' \
//      -Isrc/brogue -Isrc/platform -Isrc/variants -Isrc/test \
//      -std=c99 -O2 \
//      tools/scan_fixture_seeds.c $GAME_SRCS \
//      src/platform/null-platform.c src/platform/platformdependent.c \
//      src/test/test_harness.c \
//      -o bin/scan-fixture 2>&1 | grep error
//
// Run (find first matching seed):
//   ./bin/scan-fixture <machine_type_index> [max_seed]
//
// Run (display specific seed):
//   ./bin/scan-fixture <machine_type_index> -seed <seed>
//
// Example:
//   ./bin/scan-fixture 73          # scan seeds 1-200 for MT_FIXTURE_FOUNTAIN
//   ./bin/scan-fixture 73 -seed 13 # print seed 13

#include "test_harness.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// ASCII rendering
// ---------------------------------------------------------------------------

static char cell_char(int x, int y) {
    enum tileType dun  = pmap[x][y].layers[DUNGEON];
    enum tileType liq  = pmap[x][y].layers[LIQUID];
    enum tileType surf = pmap[x][y].layers[SURFACE];

    // DUNGEON layer — draw priority 0 wins
    if (dun == STEAM_VENT)         return '=';  // G_VENT — periodic steam
    if (dun == STATUE_INERT)       return 'S';  // G_STATUE = eszett in real term
    if (dun == CRYSTAL_WALL)       return '#';
    if (dun == ALTAR_INERT)        return '|';
    if (dun == OBSIDIAN)           return '.';  // same char as floor (dark bg)
    if (dun == MARBLE_FLOOR)       return ':';  // G_CARPET — shown as : here for clarity
    if (dun == CARPET)             return ':';

    // LIQUID layer — priority ~40-55
    if (liq == DEEP_WATER)         return '~';
    if (liq == LAVA)               return '~';  // red bg in real term
    if (liq == CHASM)              return '*';  // U_FOUR_DOTS in real term
    if (liq == SHALLOW_WATER)      return '\''; // char 0 (floor dot) + blue bg
    if (liq == MUD)                return '~';  // G_BOG — same char as water!

    // SURFACE layer — priority 45-85
    if (surf == FOLIAGE)           return '^';
    if (surf == FUNGUS_FOREST)     return '^';
    if (surf == SPIDERWEB)         return ':';
    if (surf == GRASS)             return '"';
    if (surf == DEAD_GRASS)        return '"';
    if (surf == LUMINESCENT_FUNGUS) return '"';
    if (surf == HAY)               return '"';
    if (surf == EMBERS)            return '\''; // G_ASHES
    if (surf == JUNK)              return ',';  // G_BONES
    if (surf == RED_BLOOD)         return '.';  // G_FLOOR_ALT
    if (surf == ASH)               return '\''; // G_ASHES — same as embers
    if (surf == BONES)             return ',';  // G_BONES
    if (surf == RUBBLE)            return ',';  // G_RUBBLE

    // DUNGEON layer fallback
    if (dun == FLOOR || dun == FLOOR_FLOODABLE) return '.';
    if (dun == WALL || dun == GRANITE)           return '#';

    return ' ';
}

// ---------------------------------------------------------------------------
// Detection: find the placed machine by blueprint index using metadata
// ---------------------------------------------------------------------------

static const placedMachineInfo *find_placed_machine(int mt) {
    for (int i = 0; i < rogue.placedMachineCount; i++) {
        if (rogue.placedMachines[i].blueprintIndex == mt) {
            return &rogue.placedMachines[i];
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// Print the fixture: find bounding box of feature tiles near origin + padding
// ---------------------------------------------------------------------------

static void print_fixture_map(const placedMachineInfo *info, int mt) {
    const blueprint *bp = &blueprintCatalog[mt];
    int ox = info->origin.x;
    int oy = info->origin.y;

    int minX, maxX, minY, maxY;

    if (bp->featureCount > 0) {
        // Use blueprint roomSize as search radius for feature tiles
        int radius = bp->roomSize[1];
        if (radius < 6) radius = 6;

        minX = ox; maxX = ox; minY = oy; maxY = oy;
        for (int x = ox - radius; x <= ox + radius; x++) {
            for (int y = oy - radius; y <= oy + radius; y++) {
                if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;
                for (int f = 0; f < bp->featureCount; f++) {
                    if (bp->feature[f].terrain != 0
                        && pmap[x][y].layers[bp->feature[f].layer] == bp->feature[f].terrain) {
                        if (x < minX) minX = x;
                        if (x > maxX) maxX = x;
                        if (y < minY) minY = y;
                        if (y > maxY) maxY = y;
                    }
                }
            }
        }
    } else {
        // Custom layout (no blueprint features): show fixed window around origin
        minX = ox - 3;
        maxX = ox + 3;
        minY = oy - 5;
        maxY = oy + 5;
    }

    // Add 1-cell padding for wall context
    minX = (minX > 0) ? minX - 1 : 0;
    minY = (minY > 0) ? minY - 1 : 0;
    maxX = (maxX < DCOLS - 1) ? maxX + 1 : DCOLS - 1;
    maxY = (maxY < DROWS - 1) ? maxY + 1 : DROWS - 1;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            printf("%c", cell_char(x, y));
        }
        printf("\n");
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <machine_type_index> [max_seed]\n", argv[0]);
        fprintf(stderr, "       %s <machine_type_index> -seed <seed>\n", argv[0]);
        return 1;
    }

    int mt = atoi(argv[1]);

    // Validate: init game briefly to check catalog bounds
    test_init_game(1);
    const blueprint *bp = &blueprintCatalog[mt];
    printf("Machine type %d: %s\n", mt, bp->name ? bp->name : "(unnamed)");
    printf("Depth range: %d–%d   Features: %d\n\n",
           bp->depthRange[0], bp->depthRange[1], bp->featureCount);
    test_teardown_game();

    // Mode: display a specific seed
    if (argc >= 4 && strcmp(argv[2], "-seed") == 0) {
        uint64_t seed = (uint64_t)atoll(argv[3]);
        printf("Seed %llu, D%d:\n\n", (unsigned long long)seed, bp->depthRange[0]);
        test_init_game(seed);
        const placedMachineInfo *info = find_placed_machine(mt);
        if (info) {
            printf("Origin: (%d, %d)  Machine #%d\n\n", info->origin.x, info->origin.y, info->machineNumber);
            print_fixture_map(info, mt);
        } else {
            printf("(fixture not found on D1 for this seed)\n");
        }
        printf("\nLegend: S statue  = vent  ~ water/lava  ' shallow/embers  : marble/web  , bones/rubble  ^ foliage  \" grass  . floor  # wall\n");
        test_teardown_game();
        return 0;
    }

    // Mode: scan seeds 1..max_seed
    uint64_t max_seed = (argc >= 3) ? (uint64_t)atoll(argv[2]) : 200;
    printf("Scanning seeds 1–%llu for machine type %d on D1...\n\n",
           (unsigned long long)max_seed, mt);

    uint64_t first_hit = 0;
    for (uint64_t seed = 1; seed <= max_seed; seed++) {
        test_init_game(seed);
        const placedMachineInfo *info = find_placed_machine(mt);
        test_teardown_game();
        if (info) {
            printf("Seed %llu D1\n", (unsigned long long)seed);
            if (first_hit == 0) first_hit = seed;
        }
    }

    if (first_hit > 0) {
        printf("\nFirst hit: seed %llu. Run:\n", (unsigned long long)first_hit);
        printf("  ./bin/scan-fixture %d -seed %llu\n", mt, (unsigned long long)first_hit);
    } else {
        printf("No seeds found in range 1–%llu.\n", (unsigned long long)max_seed);
        printf("Try: ./bin/scan-fixture %d 1000\n", mt);
    }

    return 0;
}
