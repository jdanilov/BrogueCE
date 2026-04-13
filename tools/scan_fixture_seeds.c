// scan_fixture_seeds.c — Generic fixture seed scanner.
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
// Detection: return true if this level contains the fixture
// Strategy: find any cell matching feature[0].terrain on feature[0].layer,
//           then check a radius-2 neighborhood for feature[1].terrain on
//           feature[1].layer (if feature[1] exists with terrain != 0).
// ---------------------------------------------------------------------------

static boolean level_has_fixture(int mt) {
    const blueprint *bp = &blueprintCatalog[mt];
    if (bp->featureCount == 0) return false;

    // Find first feature with a terrain tile
    int f0 = -1;
    for (int i = 0; i < bp->featureCount; i++) {
        if (bp->feature[i].terrain != 0) { f0 = i; break; }
    }
    if (f0 < 0) return false;

    enum tileType t0    = bp->feature[f0].terrain;
    enum dungeonLayers l0 = bp->feature[f0].layer;

    // Find second feature with a terrain tile (for confirmation)
    int f1 = -1;
    for (int i = f0 + 1; i < bp->featureCount; i++) {
        if (bp->feature[i].terrain != 0) { f1 = i; break; }
    }

    enum tileType t1    = (f1 >= 0) ? bp->feature[f1].terrain : 0;
    enum dungeonLayers l1 = (f1 >= 0) ? bp->feature[f1].layer  : 0;

    for (int x = 1; x < DCOLS - 1; x++) {
        for (int y = 1; y < DROWS - 1; y++) {
            if (pmap[x][y].layers[l0] != t0) continue;

            // If no second confirmation feature, presence of t0 is enough
            if (t1 == 0) return true;

            // Check radius 2 for t1/l1
            for (int dx = -2; dx <= 2; dx++) {
                for (int dy = -2; dy <= 2; dy++) {
                    int nx = x + dx, ny = y + dy;
                    if (nx < 0 || nx >= DCOLS || ny < 0 || ny >= DROWS) continue;
                    if (pmap[nx][ny].layers[l1] == t1) return true;
                }
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Print a 13x9 window centered on the first matching t0 cell
// ---------------------------------------------------------------------------

static void print_fixture_map(int mt) {
    const blueprint *bp = &blueprintCatalog[mt];
    if (bp->featureCount == 0) return;

    int f0 = -1;
    for (int i = 0; i < bp->featureCount; i++) {
        if (bp->feature[i].terrain != 0) { f0 = i; break; }
    }
    if (f0 < 0) return;

    enum tileType t0      = bp->feature[f0].terrain;
    enum dungeonLayers l0 = bp->feature[f0].layer;

    int cx = -1, cy = -1;
    for (int x = 1; x < DCOLS - 1 && cx < 0; x++) {
        for (int y = 1; y < DROWS - 1 && cx < 0; y++) {
            if (pmap[x][y].layers[l0] == t0) { cx = x; cy = y; }
        }
    }
    if (cx < 0) return;

    for (int y = cy - 4; y <= cy + 4; y++) {
        for (int x = cx - 6; x <= cx + 6; x++) {
            if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) printf(" ");
            else printf("%c", cell_char(x, y));
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
    int catalog_size = 0;
    // Count blueprints by walking until depthRange[0]==0 && name==NULL
    // Actually just trust the user's index; print name as confirmation.
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
        if (level_has_fixture(mt)) {
            print_fixture_map(mt);
        } else {
            printf("(fixture not found on D1 for this seed)\n");
        }
        printf("\nLegend: S statue  ~ water/lava  ' shallow/embers  : marble/web  , bones/rubble  ^ foliage  \" grass  . floor  # wall\n");
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
        boolean found = level_has_fixture(mt);
        test_teardown_game();
        if (found) {
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
