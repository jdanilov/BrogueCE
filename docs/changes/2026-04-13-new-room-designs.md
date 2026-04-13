# New Room Designs

Added 10 new room types to dungeon generation, more than doubling room variety from 8 to 18 types. The new rooms create more tactical terrain on early levels through sightline-breaking pillars, chokepoints, blind corners, and defensible positions.

## New room types

| # | Room | Description |
|---|------|-------------|
| 8 | **L-shaped** | Two rectangles meeting at a corner. 4 orientations. Arms 3-6 wide, 5-10 long. |
| 9 | **Pillared hall** | Rectangle (7-13 x 5-9) with interior pillar grid. Pillar margin 1-2 tiles from walls. |
| 10 | **Nested/donut** | Large rectangle (7-11 x 5-9) with hollow center (3-5 x 1-3) and one gap entry. |
| 11 | **Trefoil/clover** | Three overlapping circles (radius 2-3) in a triangle arrangement. |
| 12 | **Cathedral** | Cross-shaped pillared halls. Main: 9-15 x 5-7, cross: 5-7 x 7-11. Two variants: 2/3 have pillar rows only near top/bottom walls (open nave), 1/3 have pillars throughout. |
| 13 | **Alcove** | Rectangle (7-13 x 4-7) with 1-cell niches along top and bottom walls every 2 cells. |
| 14 | **Z-shaped** | Two rectangles (4-7 x 3-5) on opposite corners connected by a 2-3 wide corridor. |
| 15 | **T-shaped** | Bar (7-12 x 2-4) with perpendicular stem (3-5 x 3-6). 4 orientations. |
| 16 | **Diamond** | Manhattan-distance diamond shape, radius 3-5. |
| 17 | **Dumbbell** | Two circles (radius 2-4) connected by a narrow corridor (1-2 wide, 2-4 long). Horizontal or vertical. |

## Depth weighting

Five room types (L-shaped, Pillared, Alcove, Z-shaped, T-shaped) get a frequency boost at shallow depths that fades linearly toward the amulet level. This increases architectural variety on early floors where gameplay is currently repetitive.

## Files changed

- `src/brogue/Architect.c` — 10 new `designXxxRoom()` functions, switch cases, depth adjustment
- `src/brogue/Rogue.h` — `ROOM_TYPE_COUNT` 8 to 18, `designRoomOfType()` declaration
- `src/brogue/Globals.c` — Updated all 4 dungeon profiles with 18-element frequency arrays
- `src/test/test_room_designs.c` — 20 tests (2 per room type: single-seed + 10-seed sweep)
- `src/test/test_main.c` — Registered `suite_room_designs`
- `test/update_seed_catalogs.sh` — New convenience script to regenerate all seed catalogs
- `test/seed_catalogs/` — All 3 catalogs regenerated
