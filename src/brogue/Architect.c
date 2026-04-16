/*
 *  Architect.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/10/09.
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

short topBlobMinX, topBlobMinY, blobWidth, blobHeight;

boolean cellHasTerrainFlag(pos loc, unsigned long flagMask) {
    brogueAssert(isPosInMap(loc));
    return ((flagMask) & terrainFlags(loc) ? true : false);
}

boolean cellHasTMFlag(pos loc, unsigned long flagMask) {
    return (flagMask & terrainMechFlags(loc)) ? true : false;
}

boolean cellHasTerrainType(pos p, enum tileType terrain) {
    return (
        pmapAt(p)->layers[DUNGEON] == terrain
        || pmapAt(p)->layers[LIQUID] == terrain
        || pmapAt(p)->layers[SURFACE] == terrain
        || pmapAt(p)->layers[GAS] == terrain
    ) ? true : false;
}

static inline boolean cellIsPassableOrDoor(short x, short y) {
    if (!cellHasTerrainFlag((pos){ x, y }, T_PATHING_BLOCKER)) {
        return true;
    }
    return (
        (cellHasTMFlag((pos){ x, y }, (TM_IS_SECRET | TM_PROMOTES_WITH_KEY | TM_CONNECTS_LEVEL)) && cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY))
    );
}

// BFS check: returns true if all passable-or-door cells form a single connected component.
// Used after custom fixture layouts to ensure blocking terrain didn't split the level.
static boolean levelIsFullyConnected(void) {
    char visited[DCOLS][DROWS];
    short queueX[DCOLS * DROWS], queueY[DCOLS * DROWS];
    short qHead = 0, qTail = 0;

    zeroOutGrid(visited);

    // Seed BFS from first passable cell.
    for (short i = 1; i < DCOLS - 1 && qTail == 0; i++) {
        for (short j = 1; j < DROWS - 1 && qTail == 0; j++) {
            if (cellIsPassableOrDoor(i, j)) {
                visited[i][j] = true;
                queueX[qTail] = i;
                queueY[qTail] = j;
                qTail++;
            }
        }
    }

    if (qTail == 0) return true; // no passable cells

    // BFS flood fill.
    while (qHead < qTail) {
        short x = queueX[qHead], y = queueY[qHead];
        qHead++;
        for (short dir = 0; dir < 4; dir++) {
            short nx = x + nbDirs[dir][0];
            short ny = y + nbDirs[dir][1];
            if (coordinatesAreInMap(nx, ny) && !visited[nx][ny] && cellIsPassableOrDoor(nx, ny)) {
                visited[nx][ny] = true;
                queueX[qTail] = nx;
                queueY[qTail] = ny;
                qTail++;
            }
        }
    }

    // If any passable cell was not reached, the level is disconnected.
    for (short i = 1; i < DCOLS - 1; i++) {
        for (short j = 1; j < DROWS - 1; j++) {
            if (cellIsPassableOrDoor(i, j) && !visited[i][j]) {
                return false;
            }
        }
    }
    return true;
}

static boolean checkLoopiness(short x, short y) {
    short newX, newY, dir, sdir;
    short numStrings, maxStringLength, currentStringLength;

    if (!(pmap[x][y].flags & IN_LOOP)) {
        return false;
    }

    // find an unloopy neighbor to start on
    for (sdir = 0; sdir < DIRECTION_COUNT; sdir++) {
        newX = x + cDirs[sdir][0];
        newY = y + cDirs[sdir][1];
        if (!coordinatesAreInMap(newX, newY)
            || !(pmap[newX][newY].flags & IN_LOOP)) {
            break;
        }
    }
    if (sdir == 8) { // no unloopy neighbors
        return false; // leave cell loopy
    }

    // starting on this unloopy neighbor, work clockwise and count up (a) the number of strings
    // of loopy neighbors, and (b) the length of the longest such string.
    numStrings = maxStringLength = currentStringLength = 0;
    boolean inString = false;
    for (dir = sdir; dir < sdir + 8; dir++) {
        newX = x + cDirs[dir % 8][0];
        newY = y + cDirs[dir % 8][1];
        if (coordinatesAreInMap(newX, newY) && (pmap[newX][newY].flags & IN_LOOP)) {
            currentStringLength++;
            if (!inString) {
                if (numStrings > 0) {
                    return false; // more than one string here; leave loopy
                }
                numStrings++;
                inString = true;
            }
        } else if (inString) {
            if (currentStringLength > maxStringLength) {
                maxStringLength = currentStringLength;
            }
            currentStringLength = 0;
            inString = false;
        }
    }
    if (inString && currentStringLength > maxStringLength) {
        maxStringLength = currentStringLength;
    }
    if (numStrings == 1 && maxStringLength <= 4) {
        pmap[x][y].flags &= ~IN_LOOP;

        for (dir = 0; dir < DIRECTION_COUNT; dir++) {
            newX = x + cDirs[dir][0];
            newY = y + cDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)) {
                checkLoopiness(newX, newY);
            }
        }
        return true;
    } else {
        return false;
    }
}

static void auditLoop(short x, short y, char grid[DCOLS][DROWS]) {
    short dir, newX, newY;
    if (coordinatesAreInMap(x, y)
        && !grid[x][y]
        && !(pmap[x][y].flags & IN_LOOP)) {

        grid[x][y] = true;
        for (dir = 0; dir < DIRECTION_COUNT; dir++) {
            newX = x + nbDirs[dir][0];
            newY = y + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)) {
                auditLoop(newX, newY, grid);
            }
        }
    }
}

// Assumes it is called with respect to a passable (startX, startY), and that the same is not already included in results.
// Returns 10000 if the area included an area machine.
static short floodFillCount(char results[DCOLS][DROWS], char passMap[DCOLS][DROWS], short startX, short startY) {
    short dir, newX, newY, count;

    count = (passMap[startX][startY] == 2 ? 5000 : 1);

    if (pmap[startX][startY].flags & IS_IN_AREA_MACHINE) {
        count = 10000;
    }

    results[startX][startY] = true;

    for(dir=0; dir<4; dir++) {
        newX = startX + nbDirs[dir][0];
        newY = startY + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && passMap[newX][newY]
            && !results[newX][newY]) {

            count += floodFillCount(results, passMap, newX, newY);
        }
    }
    return min(count, 10000);
}

// Rotates around the cell, counting up the number of distinct strings of passable neighbors in a single revolution.
//      Zero means there are no impassable tiles adjacent.
//      One means it is adjacent to a wall.
//      Two means it is in a hallway or something similar.
//      Three means it is the center of a T-intersection or something similar.
//      Four means it is in the intersection of two hallways.
//      Five or more means there is a bug.
short passableArcCount(short x, short y) {
    short arcCount, dir, oldX, oldY, newX, newY;

    brogueAssert(coordinatesAreInMap(x, y));

    arcCount = 0;
    for (dir = 0; dir < DIRECTION_COUNT; dir++) {
        oldX = x + cDirs[(dir + 7) % 8][0];
        oldY = y + cDirs[(dir + 7) % 8][1];
        newX = x + cDirs[dir][0];
        newY = y + cDirs[dir][1];
        // Counts every transition from passable to impassable or vice-versa on the way around the cell:
        if ((coordinatesAreInMap(newX, newY) && cellIsPassableOrDoor(newX, newY))
            != (coordinatesAreInMap(oldX, oldY) && cellIsPassableOrDoor(oldX, oldY))) {
            arcCount++;
        }
    }
    return arcCount / 2; // Since we added one when we entered a wall and another when we left.
}

// locates all loops and chokepoints
void analyzeMap(boolean calculateChokeMap) {
    short i, j, i2, j2, dir, newX, newY, oldX, oldY, passableArcCount, cellCount;
    char grid[DCOLS][DROWS], passMap[DCOLS][DROWS];
    boolean designationSurvives;

    // first find all of the loops
    rogue.staleLoopMap = false;

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)) {

                pmap[i][j].flags &= ~IN_LOOP;
                passMap[i][j] = false;
            } else {
                pmap[i][j].flags |= IN_LOOP;
                passMap[i][j] = true;
            }
        }
    }

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            checkLoopiness(i, j);
        }
    }

    // remove extraneous loop markings
    zeroOutGrid(grid);
    auditLoop(0, 0, grid);

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (pmap[i][j].flags & IN_LOOP) {
                designationSurvives = false;
                for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && !grid[newX][newY]
                        && !(pmap[newX][newY].flags & IN_LOOP)) {
                        designationSurvives = true;
                        break;
                    }
                }
                if (!designationSurvives) {
                    grid[i][j] = true;
                    pmap[i][j].flags &= ~IN_LOOP;
                }
            }
        }
    }

    // done finding loops; now flag chokepoints
    for(i=1; i<DCOLS-1; i++) {
        for(j=1; j<DROWS-1; j++) {
            pmap[i][j].flags &= ~IS_CHOKEPOINT;
            if (passMap[i][j] && !(pmap[i][j].flags & IN_LOOP)) {
                passableArcCount = 0;
                for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                    oldX = i + cDirs[(dir + 7) % 8][0];
                    oldY = j + cDirs[(dir + 7) % 8][1];
                    newX = i + cDirs[dir][0];
                    newY = j + cDirs[dir][1];
                    if ((coordinatesAreInMap(newX, newY) && passMap[newX][newY])
                        != (coordinatesAreInMap(oldX, oldY) && passMap[oldX][oldY])) {
                        if (++passableArcCount > 2) {
                            if (!passMap[i-1][j] && !passMap[i+1][j] || !passMap[i][j-1] && !passMap[i][j+1]) {
                                pmap[i][j].flags |= IS_CHOKEPOINT;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    if (calculateChokeMap) {

        // Done finding chokepoints; now create a chokepoint map.

        // The chokepoint map is a number for each passable tile. If the tile is a chokepoint,
        // then the number indicates the number of tiles that would be rendered unreachable if the
        // chokepoint were blocked. If the tile is not a chokepoint, then the number indicates
        // the number of tiles that would be rendered unreachable if the nearest exit chokepoint
        // were blocked.
        // The cost of all of this is one depth-first flood-fill per open point that is adjacent to a chokepoint.

        // Start by setting the chokepoint values really high, and roping off room machines.
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                chokeMap[i][j] = 30000;
                if (pmap[i][j].flags & IS_IN_ROOM_MACHINE) {
                    passMap[i][j] = false;
                }
            }
        }

        // Scan through and find a chokepoint next to an open point.

        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (passMap[i][j] && (pmap[i][j].flags & IS_CHOKEPOINT)) {
                    for (dir=0; dir<4; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && passMap[newX][newY]
                            && !(pmap[newX][newY].flags & IS_CHOKEPOINT)) {
                            // OK, (newX, newY) is an open point and (i, j) is a chokepoint.
                            // Pretend (i, j) is blocked by changing passMap, and run a flood-fill cell count starting on (newX, newY).
                            // Keep track of the flooded region in grid[][].
                            zeroOutGrid(grid);
                            passMap[i][j] = false;
                            cellCount = floodFillCount(grid, passMap, newX, newY);
                            passMap[i][j] = true;

                            // CellCount is the size of the region that would be obstructed if the chokepoint were blocked.
                            // CellCounts less than 4 are not useful, so we skip those cases.

                            if (cellCount >= 4) {
                                // Now, on the chokemap, all of those flooded cells should take the lesser of their current value or this resultant number.
                                for(i2=0; i2<DCOLS; i2++) {
                                    for(j2=0; j2<DROWS; j2++) {
                                        if (grid[i2][j2] && cellCount < chokeMap[i2][j2]) {
                                            chokeMap[i2][j2] = cellCount;
                                            pmap[i2][j2].flags &= ~IS_GATE_SITE;
                                        }
                                    }
                                }

                                // The chokepoint itself should also take the lesser of its current value or the flood count.
                                if (cellCount < chokeMap[i][j]) {
                                    chokeMap[i][j] = cellCount;
                                    pmap[i][j].flags |= IS_GATE_SITE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Add some loops to the otherwise simply connected network of rooms.
static void addLoops(short **grid, short minimumPathingDistance) {
    short newX, newY, oppX, oppY;
    short **pathMap, **costMap;
    short i, d, x, y, sCoord[DCOLS*DROWS];
    const short dirCoords[2][2] = {{1, 0}, {0, 1}};

    fillSequentialList(sCoord, DCOLS*DROWS);
    shuffleList(sCoord, DCOLS*DROWS);

    if (D_INSPECT_LEVELGEN) {
        colorOverDungeon(&darkGray);
        hiliteGrid(grid, &white, 100);
    }

    pathMap = allocGrid();
    costMap = allocGrid();
    copyGrid(costMap, grid);
    findReplaceGrid(costMap, 0, 0, PDS_OBSTRUCTION);
    findReplaceGrid(costMap, 1, 30000, 1);

    for (i = 0; i < DCOLS*DROWS; i++) {
        x = sCoord[i]/DROWS;
        y = sCoord[i] % DROWS;
        if (!grid[x][y]) {
            for (d=0; d <= 1; d++) { // Try a horizontal door, and then a vertical door.
                newX = x + dirCoords[d][0];
                oppX = x - dirCoords[d][0];
                newY = y + dirCoords[d][1];
                oppY = y - dirCoords[d][1];
                if (coordinatesAreInMap(newX, newY)
                    && coordinatesAreInMap(oppX, oppY)
                    && grid[newX][newY] == 1
                    && grid[oppX][oppY] == 1) { // If the tile being inspected has floor on both sides,

                    fillGrid(pathMap, 30000);
                    pathMap[newX][newY] = 0;
                    dijkstraScan(pathMap, costMap, false);
                    if (pathMap[oppX][oppY] > minimumPathingDistance) { // and if the pathing distance between the two flanking floor tiles exceeds minimumPathingDistance,
                        grid[x][y] = 2;             // then turn the tile into a doorway.
                        costMap[x][y] = 1;          // (Cost map also needs updating.)
                        if (D_INSPECT_LEVELGEN) {
                            pos p = { x, y };
                            plotCharWithColor(G_CLOSED_DOOR, mapToWindow(p), &black, &green);
                        }
                        break;
                    }
                }
            }
        }
    }
    if (D_INSPECT_LEVELGEN) {
        temporaryMessage("Added secondary connections:", REQUIRE_ACKNOWLEDGMENT);
    }
    freeGrid(pathMap);
    freeGrid(costMap);
}

// Assumes (startX, startY) is in the machine.
// Returns true if everything went well, and false if we ran into a machine component
// that was already there, as we don't want to build a machine around it.
static boolean addTileToMachineInteriorAndIterate(char interior[DCOLS][DROWS], short startX, short startY) {
    short dir, newX, newY;
    boolean goodSoFar = true;

    interior[startX][startY] = true;

    for (dir = 0; dir < 4 && goodSoFar; dir++) {
        newX = startX + nbDirs[dir][0];
        newY = startY + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)) {
            if ((pmap[newX][newY].flags & HAS_ITEM)
                || ((pmap[newX][newY].flags & IS_IN_MACHINE) && !(pmap[newX][newY].flags & IS_GATE_SITE))) {
                // Abort if there's an item in the room.
                // Items haven't been populated yet, so the only way this could happen is if another machine
                // previously placed an item here.
                // Also abort if we're touching another machine at any point other than a gate tile.
                return false;
            }
            if (!interior[newX][newY]
                && chokeMap[newX][newY] <= chokeMap[startX][startY] // don't have to worry about walls since they're all 30000
                && !(pmap[newX][newY].flags & IS_IN_MACHINE)) {
                //goodSoFar = goodSoFar && addTileToMachineInteriorAndIterate(interior, newX, newY);
                if (goodSoFar) {
                    goodSoFar = addTileToMachineInteriorAndIterate(interior, newX, newY);
                }
            }
        }
    }
    return goodSoFar;
}

static void copyMap(pcell from[DCOLS][DROWS], pcell to[DCOLS][DROWS]) {
    short i, j;

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            to[i][j] = from[i][j];
        }
    }
}

static boolean itemIsADuplicate(item *theItem, item **spawnedItems, short itemCount) {
    short i;
    if (theItem->category & (STAFF | WAND | POTION | SCROLL | RING | WEAPON | ARMOR | CHARM)) {
        for (i = 0; i < itemCount; i++) {
            if (spawnedItems[i]->category == theItem->category
                && spawnedItems[i]->kind == theItem->kind) {

                return true;
            }
        }
    }
    return false;
}

static boolean blueprintQualifies(short i, unsigned long requiredMachineFlags) {
    if (blueprintCatalog[i].depthRange[0] > rogue.depthLevel
        || blueprintCatalog[i].depthRange[1] < rogue.depthLevel
                // Must have the required flags:
        || (~(blueprintCatalog[i].flags) & requiredMachineFlags)
                // May NOT have BP_ADOPT_ITEM unless that flag is required:
        || (blueprintCatalog[i].flags & BP_ADOPT_ITEM & ~requiredMachineFlags)
                // May NOT have BP_VESTIBULE unless that flag is required:
        || (blueprintCatalog[i].flags & BP_VESTIBULE & ~requiredMachineFlags)) {

        return false;
    }
    return true;
}

static void abortItemsAndMonsters(item *spawnedItems[MACHINES_BUFFER_LENGTH], creature *spawnedMonsters[MACHINES_BUFFER_LENGTH]) {
    short i, j;

    for (i=0; i<MACHINES_BUFFER_LENGTH && spawnedItems[i]; i++) {
        removeItemFromChain(spawnedItems[i], floorItems);
        removeItemFromChain(spawnedItems[i], packItems); // just in case; can't imagine why this would arise.
        for (j=0; j<MACHINES_BUFFER_LENGTH && spawnedMonsters[j]; j++) {
            // Remove the item from spawned monsters, so it doesn't get double-freed when the creature is killed below.
            if (spawnedMonsters[j]->carriedItem == spawnedItems[i]) {
                spawnedMonsters[j]->carriedItem = NULL;
                break;
            }
        }
        deleteItem(spawnedItems[i]);
        spawnedItems[i] = NULL;
    }
    for (i=0; i<MACHINES_BUFFER_LENGTH && spawnedMonsters[i]; i++) {
        killCreature(spawnedMonsters[i], true);
        spawnedMonsters[i] = NULL;
    }
}

static boolean cellIsFeatureCandidate(short x, short y,
                               short originX, short originY,
                               short distanceBound[2],
                               char interior[DCOLS][DROWS],
                               char occupied[DCOLS][DROWS],
                               char viewMap[DCOLS][DROWS],
                               short **distanceMap,
                               short machineNumber,
                               unsigned long featureFlags,
                               unsigned long bpFlags) {
    short newX, newY, dir, distance;

    // No building in the hallway if it's prohibited.
    // This check comes before the origin check, so an area machine will fail altogether
    // if its origin is in a hallway and the feature that must be built there does not permit as much.
    if ((featureFlags & MF_NOT_IN_HALLWAY)
        && passableArcCount(x, y) > 1) {
        return false;
    }

    // No building along the perimeter of the level if it's prohibited.
    if ((featureFlags & MF_NOT_ON_LEVEL_PERIMETER)
        && (x == 0 || x == DCOLS - 1 || y == 0 || y == DROWS - 1)) {
        return false;
    }

    // The origin is a candidate if the feature is flagged to be built at the origin.
    // If it's a room, the origin (i.e. doorway) is otherwise NOT a candidate.
    if (featureFlags & MF_BUILD_AT_ORIGIN) {
        return ((x == originX && y == originY) ? true : false);
    } else if ((bpFlags & BP_ROOM) && x == originX && y == originY) {
        return false;
    }

    // No building in another feature's personal space!
    if (occupied[x][y]) {
        return false;
    }

    // Must be in the viewmap if the appropriate flag is set.
    if ((featureFlags & (MF_IN_VIEW_OF_ORIGIN | MF_IN_PASSABLE_VIEW_OF_ORIGIN))
        && !viewMap[x][y]) {
        return false;
    }

    // Do a distance check if the feature requests it.
    if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) { // Distance is calculated for walls too.
        distance = 10000;
        for (dir = 0; dir < 4; dir++) {
            newX = x + nbDirs[dir][0];
            newY = y + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)
                && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
                && distance > distanceMap[newX][newY] + 1) {

                distance = distanceMap[newX][newY] + 1;
            }
        }
    } else {
        distance = distanceMap[x][y];
    }

    if (distance > distanceBound[1]     // distance exceeds max
        || distance < distanceBound[0]) {   // distance falls short of min
        return false;
    }
    if (featureFlags & MF_BUILD_IN_WALLS) {             // If we're supposed to build in a wall...
        if (!interior[x][y]
            && (pmap[x][y].machineNumber == 0 || pmap[x][y].machineNumber == machineNumber)
            && cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) { // ...and this location is a wall that's not already machined...
            for (dir=0; dir<4; dir++) {
                newX = x + nbDirs[dir][0];
                newY = y + nbDirs[dir][1];
                if (coordinatesAreInMap(newX, newY)     // ...and it's next to an interior spot or permitted elsewhere and next to passable spot...
                    && ((interior[newX][newY] && !(newX==originX && newY==originY))
                        || ((featureFlags & MF_BUILD_ANYWHERE_ON_LEVEL)
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)
                            && pmap[newX][newY].machineNumber == 0))) {
                    return true;                        // ...then we're golden!
                }
            }
        }
        return false;                                   // Otherwise, no can do.
    } else if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) { // Can't build in a wall unless instructed to do so.
        return false;
    } else if (featureFlags & MF_BUILD_ANYWHERE_ON_LEVEL) {
        if ((featureFlags & MF_GENERATE_ITEM)
            && (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_ITEMS | T_PATHING_BLOCKER) || (pmap[x][y].flags & (IS_CHOKEPOINT | IN_LOOP | IS_IN_MACHINE)))) {
            return false;
        } else {
            return !(pmap[x][y].flags & IS_IN_MACHINE);
        }
    } else if (interior[x][y]) {
        return true;
    }
    return false;
}


static void addLocationToKey(item *theItem, short x, short y, boolean disposableHere) {
    short i;

    for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].loc.x || theItem->keyLoc[i].machine); i++);
    theItem->keyLoc[i].loc = (pos){ x, y };
    theItem->keyLoc[i].disposableHere = disposableHere;
}

static void addMachineNumberToKey(item *theItem, short machineNumber, boolean disposableHere) {
    short i;

    for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].loc.x || theItem->keyLoc[i].machine); i++);
    theItem->keyLoc[i].machine = machineNumber;
    theItem->keyLoc[i].disposableHere = disposableHere;
}

static void expandMachineInterior(char interior[DCOLS][DROWS], short minimumInteriorNeighbors) {
    boolean madeChange;
    short nbcount, newX, newY, i, j, layer;
    enum directions dir;

    do {
        madeChange = false;
        for(i=1; i<DCOLS-1; i++) {
            for(j=1; j < DROWS-1; j++) {
                if (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                    && pmap[i][j].machineNumber == 0) {

                    // Count up the number of interior open neighbors out of eight:
                    for (nbcount = dir = 0; dir < DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (interior[newX][newY]
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)) {
                            nbcount++;
                        }
                    }
                    if (nbcount >= minimumInteriorNeighbors) {
                        // Make sure zero exterior open/machine neighbors out of eight:
                        for (nbcount = dir = 0; dir < DIRECTION_COUNT; dir++) {
                            newX = i + nbDirs[dir][0];
                            newY = j + nbDirs[dir][1];
                            if (!interior[newX][newY]
                                && (!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY) || pmap[newX][newY].machineNumber != 0)) {
                                nbcount++;
                                break;
                            }
                        }
                        if (!nbcount) {
                            // Eliminate this obstruction; welcome its location into the machine.
                            madeChange = true;
                            interior[i][j] = true;
                            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                                if (tileCatalog[pmap[i][j].layers[layer]].flags & T_PATHING_BLOCKER) {
                                    pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                                }
                            }
                            for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                                newX = i + nbDirs[dir][0];
                                newY = j + nbDirs[dir][1];
                                if (pmap[newX][newY].layers[DUNGEON] == GRANITE) {
                                    pmap[newX][newY].layers[DUNGEON] = WALL;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (madeChange);

    // Clear doors and secret doors out of the interior of the machine.
    for(i=1; i<DCOLS-1; i++) {
        for(j=1; j < DROWS-1; j++) {
            if (interior[i][j]
                && (pmap[i][j].layers[DUNGEON] == DOOR || pmap[i][j].layers[DUNGEON] == SECRET_DOOR)) {

                pmap[i][j].layers[DUNGEON] = FLOOR;
            }
        }
    }
}

static boolean fillInteriorForVestibuleMachine(char interior[DCOLS][DROWS], short bp, short originX, short originY) {
    short **distanceMap, **costMap, qualifyingTileCount, totalFreq, sRows[DROWS], sCols[DCOLS], i, j, k;
    boolean success = true;

    zeroOutGrid(interior);

    distanceMap = allocGrid();
    fillGrid(distanceMap, 30000);
    distanceMap[originX][originY] = 0;

    costMap = allocGrid();
    populateGenericCostMap(costMap);
    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (costMap[i][j] == 1 && (pmap[i][j].flags & IS_IN_MACHINE)) { //pmap[i][j].machineNumber) {
                costMap[i][j] = PDS_FORBIDDEN;
            }
        }
    }
    costMap[originX][originY] = 1;
    dijkstraScan(distanceMap, costMap, false);
    freeGrid(costMap);

    qualifyingTileCount = 0; // Keeps track of how many interior cells we've added.
    totalFreq = rand_range(blueprintCatalog[bp].roomSize[0], blueprintCatalog[bp].roomSize[1]); // Keeps track of the goal size.

    fillSequentialList(sCols, DCOLS);
    shuffleList(sCols, DCOLS);
    fillSequentialList(sRows, DROWS);
    shuffleList(sRows, DROWS);

    for (k=0; k<1000 && qualifyingTileCount < totalFreq; k++) {
        for(i=0; i<DCOLS && qualifyingTileCount < totalFreq; i++) {
            for(j=0; j<DROWS && qualifyingTileCount < totalFreq; j++) {
                if (distanceMap[sCols[i]][sRows[j]] == k) {
                    interior[sCols[i]][sRows[j]] = true;
                    qualifyingTileCount++;

                    if (pmap[sCols[i]][sRows[j]].flags & HAS_ITEM) {
                        // Abort if we've engulfed another machine's item.
                        success = false;
                        qualifyingTileCount = totalFreq; // This is a hack to drop out of these three for-loops.
                    }
                }
            }
        }
    }

    // Now make sure the interior map satisfies the machine's qualifications.
    if ((blueprintCatalog[bp].flags & BP_TREAT_AS_BLOCKING)
        && levelIsDisconnectedWithBlockingMap(interior, false)) {
        success = false;
    } else if ((blueprintCatalog[bp].flags & BP_REQUIRE_BLOCKING)
               && levelIsDisconnectedWithBlockingMap(interior, true) < 100) {
        success = false;
    }
    freeGrid(distanceMap);
    return success;
}

static void redesignInterior(char interior[DCOLS][DROWS], short originX, short originY, short theProfileIndex) {
    short i, j, n, newX, newY;
    enum directions dir;
    pos orphanList[20];
    short orphanCount = 0;
    short **grid, **pathingGrid, **costGrid;
    grid = allocGrid();

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (interior[i][j]) {
                if (i == originX && j == originY) {
                    grid[i][j] = 1; // All rooms must grow from this space.
                } else {
                    grid[i][j] = 0; // Other interior squares are fair game for placing rooms.
                }
            } else if (cellIsPassableOrDoor(i, j)) {
                grid[i][j] = 1; // Treat existing level as already built (though shielded by a film of -1s).
                for (dir = 0; dir < 4; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && interior[newX][newY]
                        && (newX != originX || newY != originY)) {

                        orphanList[orphanCount] = (pos){ .x = newX, .y = newY };
                        orphanCount++;
                        grid[i][j] = -1; // Treat the orphaned door as off limits.

                        break;
                    }
                }
            } else {
                grid[i][j] = -1; // Exterior spaces are off limits.
            }
        }
    }
    attachRooms(grid, &dungeonProfileCatalog[theProfileIndex], 40, 40);

    // Connect to preexisting rooms that were orphaned (mostly preexisting machine rooms).
    if (orphanCount > 0) {
        pathingGrid = allocGrid();
        costGrid = allocGrid();
        for (n = 0; n < orphanCount; n++) {

            if (D_INSPECT_MACHINES) {
                dumpLevelToScreen();
                copyGrid(pathingGrid, grid);
                findReplaceGrid(pathingGrid, -1, -1, 0);
                hiliteGrid(pathingGrid, &green, 50);
                plotCharWithColor('X', mapToWindow(orphanList[n]), &black, &orange);
                temporaryMessage("Orphan detected:", REQUIRE_ACKNOWLEDGMENT);
            }

            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (interior[i][j]) {
                        if (grid[i][j] > 0) {
                            pathingGrid[i][j] = 0;
                            costGrid[i][j] = 1;
                        } else {
                            pathingGrid[i][j] = 30000;
                            costGrid[i][j] = 1;
                        }
                    } else {
                        pathingGrid[i][j] = 30000;
                        costGrid[i][j] = PDS_OBSTRUCTION;
                    }
                }
            }
            dijkstraScan(pathingGrid, costGrid, false);

            i = orphanList[n].x;
            j = orphanList[n].y;
            while (pathingGrid[i][j] > 0) {
                for (dir = 0; dir < 4; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];

                    if (coordinatesAreInMap(newX, newY)
                        && pathingGrid[newX][newY] < pathingGrid[i][j]) {

                        grid[i][j] = 1;
                        i = newX;
                        j = newY;
                        break;
                    }
                }
                brogueAssert(dir < 4);
                if (D_INSPECT_MACHINES) {
                    dumpLevelToScreen();
                    displayGrid(pathingGrid);
                    pos p = { i, j };
                    plotCharWithColor('X', mapToWindow(p), &black, &orange);
                    temporaryMessage("Orphan connecting:", REQUIRE_ACKNOWLEDGMENT);
                }
            }
        }
        freeGrid(pathingGrid);
        freeGrid(costGrid);
    }

    addLoops(grid, 10);
    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (interior[i][j]) {
                if (grid[i][j] >= 0) {
                    pmap[i][j].layers[SURFACE] = pmap[i][j].layers[GAS] = NOTHING;
                }
                if (grid[i][j] == 0) {
                    pmap[i][j].layers[DUNGEON] = GRANITE;
                    interior[i][j] = false;
                }
                if (grid[i][j] >= 1) {
                    pmap[i][j].layers[DUNGEON] = FLOOR;
                }
            }
        }
    }
    freeGrid(grid);
}

static void prepareInteriorWithMachineFlags(char interior[DCOLS][DROWS], short originX, short originY, unsigned long flags, short dungeonProfileIndex) {
    short i, j, newX, newY;
    enum dungeonLayers layer;
    enum directions dir;

    // If requested, clear and expand the room as far as possible until either it's convex or it bumps into surrounding rooms
    if (flags & BP_MAXIMIZE_INTERIOR) {
        expandMachineInterior(interior, 1);
    } else if (flags & BP_OPEN_INTERIOR) {
        expandMachineInterior(interior, 4);
    }

    // If requested, cleanse the interior -- no interesting terrain allowed.
    if (flags & BP_PURGE_INTERIOR) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]) {
                    for (layer=0; layer<NUMBER_TERRAIN_LAYERS; layer++) {
                        pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                    }
                }
            }
        }
    }

    // If requested, purge pathing blockers -- no traps allowed.
    if (flags & BP_PURGE_PATHING_BLOCKERS) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]) {
                    for (layer=0; layer<NUMBER_TERRAIN_LAYERS; layer++) {
                        if (tileCatalog[pmap[i][j].layers[layer]].flags & T_PATHING_BLOCKER) {
                            pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                        }
                    }
                }
            }
        }
    }

    // If requested, purge the liquid layer in the interior -- no liquids allowed.
    if (flags & BP_PURGE_LIQUIDS) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]) {
                    pmap[i][j].layers[LIQUID] = NOTHING;
                }
            }
        }
    }

    // Surround with walls if requested.
    if (flags & BP_SURROUND_WITH_WALLS) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j] && !(pmap[i][j].flags & IS_GATE_SITE)) {
                    for (dir=0; dir< DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && !interior[newX][newY]
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
                            && !(pmap[newX][newY].flags & IS_GATE_SITE)
                            && !pmap[newX][newY].machineNumber
                            && cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)) {
                            for (layer=0; layer<NUMBER_TERRAIN_LAYERS; layer++) {
                                pmap[newX][newY].layers[layer] = (layer == DUNGEON ? WALL : 0);
                            }
                        }
                    }
                }
            }
        }
    }

    // Completely clear the interior, fill with granite, and cut entirely new rooms into it from the gate site.
    // Then zero out any portion of the interior that is still wall.
    if (flags & BP_REDESIGN_INTERIOR) {
        redesignInterior(interior, originX, originY, dungeonProfileIndex);
    }

    // Reinforce surrounding tiles and interior tiles if requested to prevent tunneling in or through.
    if (flags & BP_IMPREGNABLE) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]
                    && !(pmap[i][j].flags & IS_GATE_SITE)) {

                    pmap[i][j].flags |= IMPREGNABLE;
                    for (dir=0; dir< DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && !interior[newX][newY]
                            && !(pmap[newX][newY].flags & IS_GATE_SITE)) {

                            pmap[newX][newY].flags |= IMPREGNABLE;
                        }
                    }
                }
            }
        }
    }
}

// --- Rotatable layout infrastructure ---
// Defines a tile as an (dx,dy) offset from an anchor point, with tile type and layer.
// Used by applyRotatableLayout() to place patterns in any of 4 cardinal orientations.
typedef struct {
    short dx, dy;
    enum tileType tile;
    enum dungeonLayers layer;
    boolean clearConflict; // if true, clear the "other" layer (SURFACE↔LIQUID)
} layoutTile;

// Rotation matrices: maps (dx,dy) → rotated (rx,ry).
// Index 0=North(identity), 1=East(90°CW), 2=South(180°), 3=West(270°CW).
static const short rotationMatrix[4][4] = {
    { 1,  0,  0,  1},  // N: rx= dx, ry= dy
    { 0, -1,  1,  0},  // E: rx=-dy, ry= dx
    {-1,  0,  0, -1},  // S: rx=-dx, ry=-dy
    { 0,  1, -1,  0},  // W: rx= dy, ry=-dx
};

// Try all 4 rotations of a tile pattern, pick the placement closest to origin.
// pattern: array of layoutTile offsets from anchor.
// centerDx/Dy: offset from anchor to the "center" point (for outCenter), in base orientation.
// Returns true if placed, false if no rotation fits the interior.
static boolean applyRotatableLayout(
    short originX, short originY,
    char interior[DCOLS][DROWS],
    const layoutTile *pattern, int patCount,
    short centerDx, short centerDy,
    pos *outCenter)
{
    short bestAX = 0, bestAY = 0, bestRot = -1;
    int bestDist = 10000;

    for (short rot = 0; rot < 4; rot++) {
        const short *r = rotationMatrix[rot];
        for (short ax = 2; ax < DCOLS - 2; ax++) {
            for (short ay = 2; ay < DROWS - 2; ay++) {
                boolean fits = true;
                for (int t = 0; t < patCount && fits; t++) {
                    short rx = ax + r[0] * pattern[t].dx + r[1] * pattern[t].dy;
                    short ry = ay + r[2] * pattern[t].dx + r[3] * pattern[t].dy;
                    if (rx < 1 || rx >= DCOLS - 1 || ry < 1 || ry >= DROWS - 1) { fits = false; break; }
                    if (!interior[rx][ry]) fits = false;
                }
                if (fits) {
                    short cx = ax + r[0] * centerDx + r[1] * centerDy;
                    short cy = ay + r[2] * centerDx + r[3] * centerDy;
                    int dist = (cx - originX) * (cx - originX) + (cy - originY) * (cy - originY);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestAX = ax;
                        bestAY = ay;
                        bestRot = rot;
                    }
                }
            }
        }
    }

    if (bestRot < 0) return false;

    const short *r = rotationMatrix[bestRot];
    for (int t = 0; t < patCount; t++) {
        short rx = bestAX + r[0] * pattern[t].dx + r[1] * pattern[t].dy;
        short ry = bestAY + r[2] * pattern[t].dx + r[3] * pattern[t].dy;
        pmap[rx][ry].layers[pattern[t].layer] = pattern[t].tile;
        if (pattern[t].clearConflict) {
            if (pattern[t].layer == LIQUID) pmap[rx][ry].layers[SURFACE] = NOTHING;
            else if (pattern[t].layer == SURFACE) pmap[rx][ry].layers[LIQUID] = NOTHING;
        }
    }

    if (outCenter) {
        outCenter->x = bestAX + r[0] * centerDx + r[1] * centerDy;
        outCenter->y = bestAY + r[2] * centerDx + r[3] * centerDy;
    }
    return true;
}

// Place a horizontal line of 3-5 shallow water tiles with rubble bookends and dead grass accent.
// Scans for the longest horizontal run of interior cells near the origin.
// Returns true if layout was placed (minimum 3 water tiles), false otherwise.
// Sets *outCenter to the center of the water line.
static boolean applyDrainageLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    short bestX = 0, bestY = 0, bestLen = 0;

    // Find longest horizontal run of interior cells near origin.
    for (short y = max(0, originY - 4); y <= min(DROWS - 1, originY + 4); y++) {
        for (short startX = 0; startX < DCOLS; startX++) {
            if (!interior[startX][y]) continue;
            short endX = startX;
            while (endX + 1 < DCOLS && interior[endX + 1][y]) {
                endX++;
            }
            short len = endX - startX + 1;
            if (len > bestLen) {
                bestLen = len;
                bestX = startX;
                bestY = y;
            }
            startX = endX;
        }
    }

    if (bestLen < 5) return false; // need at least rubble + 3 water + rubble

    // Clamp water length to 3-5 tiles, centered in the run.
    short waterLen = bestLen - 2; // reserve 1 cell each side for rubble
    if (waterLen > 5) waterLen = 5;
    if (waterLen < 3) waterLen = 3;

    short totalLen = waterLen + 2; // water + 2 rubble bookends
    short offsetX = bestX + (bestLen - totalLen) / 2;

    // Place rubble at left end
    pmap[offsetX][bestY].layers[SURFACE] = RUBBLE;
    // Place water line
    for (short i = 1; i <= waterLen; i++) {
        pmap[offsetX + i][bestY].layers[LIQUID] = SHALLOW_WATER;
        pmap[offsetX + i][bestY].layers[SURFACE] = NOTHING;
    }
    // Place rubble at right end
    pmap[offsetX + waterLen + 1][bestY].layers[SURFACE] = RUBBLE;

    // Place dead grass accent on an adjacent cell if interior allows
    short midX = offsetX + 1 + waterLen / 2;
    if (bestY + 1 < DROWS && interior[midX][bestY + 1]) {
        pmap[midX][bestY + 1].layers[SURFACE] = DEAD_GRASS;
    } else if (bestY - 1 >= 0 && interior[midX][bestY - 1]) {
        pmap[midX][bestY - 1].layers[SURFACE] = DEAD_GRASS;
    }

    if (outCenter) {
        outCenter->x = midX;
        outCenter->y = bestY;
    }
    return true;
}

// Place a puddle: 1 MUD tile at center, 1-3 SHALLOW_WATER on random adjacent cells,
// GRASS on remaining open neighbors, FOLIAGE on the outer ring. Asymmetric and organic.
// Returns true if layout was placed, false if insufficient space.
static boolean applyPuddleLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find an interior cell near the origin with at least 3 adjacent interior cells.
    short bestX = 0, bestY = 0, bestAdj = 0;

    for (short dy = -3; dy <= 3; dy++) {
        for (short dx = -3; dx <= 3; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;

            short adj = 0;
            if (interior[x-1][y]) adj++;
            if (interior[x+1][y]) adj++;
            if (interior[x][y-1]) adj++;
            if (interior[x][y+1]) adj++;

            if (adj > bestAdj) {
                bestAdj = adj;
                bestX = x;
                bestY = y;
            }
        }
    }

    if (bestAdj < 3) return false;

    // Place MUD at center
    pmap[bestX][bestY].layers[LIQUID] = MUD;
    pmap[bestX][bestY].layers[SURFACE] = NOTHING;

    // Collect adjacent interior cells and shuffle them
    short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    short adjCells[4][2];
    short adjCount = 0;
    for (short i = 0; i < 4; i++) {
        short nx = bestX + dirs[i][0];
        short ny = bestY + dirs[i][1];
        if (nx < 1 || nx >= DCOLS - 1 || ny < 1 || ny >= DROWS - 1) continue;
        if (!interior[nx][ny]) continue;
        adjCells[adjCount][0] = nx;
        adjCells[adjCount][1] = ny;
        adjCount++;
    }

    // Shuffle adjacent cells for asymmetry
    for (short i = adjCount - 1; i > 0; i--) {
        short j = rand_range(0, i);
        short tmpX = adjCells[i][0], tmpY = adjCells[i][1];
        adjCells[i][0] = adjCells[j][0]; adjCells[i][1] = adjCells[j][1];
        adjCells[j][0] = tmpX; adjCells[j][1] = tmpY;
    }

    // Place 1-3 SHALLOW_WATER on the first random adjacent cells
    short waterCount = rand_range(1, min(3, adjCount));
    for (short i = 0; i < waterCount; i++) {
        pmap[adjCells[i][0]][adjCells[i][1]].layers[LIQUID] = SHALLOW_WATER;
        pmap[adjCells[i][0]][adjCells[i][1]].layers[SURFACE] = NOTHING;
    }

    // Place GRASS on remaining adjacent cells
    for (short i = waterCount; i < adjCount; i++) {
        pmap[adjCells[i][0]][adjCells[i][1]].layers[SURFACE] = GRASS;
    }

    // Place FOLIAGE on the outer ring: cells adjacent to the water/mud cluster
    // that are interior but not already placed
    for (short i = 0; i < waterCount; i++) {
        short wx = adjCells[i][0];
        short wy = adjCells[i][1];
        for (short d = 0; d < 4; d++) {
            short fx = wx + dirs[d][0];
            short fy = wy + dirs[d][1];
            if (fx < 1 || fx >= DCOLS - 1 || fy < 1 || fy >= DROWS - 1) continue;
            if (!interior[fx][fy]) continue;
            if (fx == bestX && fy == bestY) continue; // skip MUD center
            // Skip cells already used as water or grass
            if (pmap[fx][fy].layers[LIQUID] == SHALLOW_WATER || pmap[fx][fy].layers[LIQUID] == MUD) continue;
            if (pmap[fx][fy].layers[SURFACE] == GRASS) continue;
            pmap[fx][fy].layers[SURFACE] = FOLIAGE;
        }
    }

    // Also place foliage around the grass cells
    for (short i = waterCount; i < adjCount; i++) {
        short gx = adjCells[i][0];
        short gy = adjCells[i][1];
        for (short d = 0; d < 4; d++) {
            short fx = gx + dirs[d][0];
            short fy = gy + dirs[d][1];
            if (fx < 1 || fx >= DCOLS - 1 || fy < 1 || fy >= DROWS - 1) continue;
            if (!interior[fx][fy]) continue;
            if (fx == bestX && fy == bestY) continue;
            if (pmap[fx][fy].layers[LIQUID] == SHALLOW_WATER || pmap[fx][fy].layers[LIQUID] == MUD) continue;
            if (pmap[fx][fy].layers[SURFACE] == GRASS) continue;
            pmap[fx][fy].layers[SURFACE] = FOLIAGE;
        }
    }

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }
    return true;
}

// Forge layout: anvil (STATUE_INERT) at head of a 2-tile vertical lava trough,
// flanked by obsidian work floor. Embers radiate outward, rubble at edges.
// Pattern (relative to anvil at 0,0):
//     embers rubble embers
//     embers anvil  embers
//     embers obsid  embers
//     embers lava   embers
//     embers obsid  embers
//     embers lava   embers
//     embers rubble embers
// Requires a 3-wide, 5-tall interior region near origin.
static boolean applyForgeLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // 3-wide, 5-tall forge: rubble-anvil-obsidian-lava-rubble with embers flanking.
    // Tries all 4 rotations via shared helper.
    static const layoutTile forgePat[] = {
        // Row 0: rubble center, embers flanking
        {-1, 0, EMBERS,       SURFACE, false},
        { 0, 0, RUBBLE,       SURFACE, false},
        { 1, 0, EMBERS,       SURFACE, false},
        // Row 1: anvil center, embers flanking
        {-1, 1, EMBERS,       SURFACE, false},
        { 0, 1, STATUE_INERT, DUNGEON, false},
        { 1, 1, EMBERS,       SURFACE, false},
        // Row 2: obsidian center, embers flanking
        {-1, 2, EMBERS,       SURFACE, false},
        { 0, 2, OBSIDIAN,     DUNGEON, false},
        { 1, 2, EMBERS,       SURFACE, false},
        // Row 3: lava center, embers flanking
        {-1, 3, EMBERS,       SURFACE, false},
        { 0, 3, LAVA,         LIQUID,  true},
        { 1, 3, EMBERS,       SURFACE, false},
        // Row 4: rubble center, embers flanking
        {-1, 4, EMBERS,       SURFACE, false},
        { 0, 4, RUBBLE,       SURFACE, false},
        { 1, 4, EMBERS,       SURFACE, false},
    };
    return applyRotatableLayout(originX, originY, interior,
                                forgePat, sizeof(forgePat) / sizeof(forgePat[0]),
                                0, 2, outCenter);
}

// Place an altar nook: processional carpet runner leading to an altar on a marble dais.
// Tries all 4 rotations via shared helper.
static boolean applyAltarNookLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    static const layoutTile altarPat[] = {
        // Row 0: dais
        { 0,  0, ALTAR_INERT,  DUNGEON, false},
        {-1,  0, MARBLE_FLOOR, DUNGEON, false},
        { 1,  0, MARBLE_FLOOR, DUNGEON, false},
        // Row 1: marble + carpet + marble
        {-1,  1, MARBLE_FLOOR, DUNGEON, false},
        { 0,  1, CARPET,       DUNGEON, false},
        { 1,  1, MARBLE_FLOOR, DUNGEON, false},
        // Rows 2-4: carpet runner
        { 0,  2, CARPET,       DUNGEON, false},
        { 0,  3, CARPET,       DUNGEON, false},
        { 0,  4, CARPET,       DUNGEON, false},
        // Row 5: embers flanking entrance
        {-1,  5, EMBERS,       SURFACE, false},
        { 0,  5, CARPET,       DUNGEON, false},
        { 1,  5, EMBERS,       SURFACE, false},
    };
    return applyRotatableLayout(originX, originY, interior,
                                altarPat, sizeof(altarPat) / sizeof(altarPat[0]),
                                0, 3, outCenter);
}

// Place spiderwebs on interior cells adjacent to walls, with bones nearby.
// Finds wall-adjacent interior cells near the origin and clusters webs there.
// Place a vine trellis: 2-8 foliage tiles hugging a wall in a contiguous line.
// Finds wall-adjacent interior cells near the origin and grows a line along the wall.
// Returns true if at least 2 tiles were placed.
static boolean applyVineTrellisLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Collect wall-adjacent interior floor cells near the origin.
    typedef struct { short x, y; } cell;
    cell wallAdj[40];
    short wallAdjCount = 0;

    for (short dy = -5; dy <= 5 && wallAdjCount < 40; dy++) {
        for (short dx = -5; dx <= 5 && wallAdjCount < 40; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            // Must be adjacent to at least one wall
            boolean adjWall = false;
            if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)) adjWall = true;
            if (cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)) adjWall = true;
            if (cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)) adjWall = true;
            if (cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) adjWall = true;

            if (adjWall) {
                wallAdj[wallAdjCount++] = (cell){x, y};
            }
        }
    }

    if (wallAdjCount < 2) return false;

    // For each wall-adjacent cell, try to grow the longest contiguous line
    // along the wall in cardinal directions.
    short bestLen = 0;
    cell bestLine[8];

    for (short i = 0; i < wallAdjCount; i++) {
        // Try horizontal and vertical directions
        short dirs[2][2] = {{1, 0}, {0, 1}};
        for (short d = 0; d < 2; d++) {
            cell line[8];
            short len = 0;
            line[len++] = wallAdj[i];

            // Grow in positive direction
            for (short step = 1; step <= 7 && len < 8; step++) {
                short nx = wallAdj[i].x + dirs[d][0] * step;
                short ny = wallAdj[i].y + dirs[d][1] * step;
                if (nx < 1 || nx >= DCOLS - 1 || ny < 1 || ny >= DROWS - 1) break;
                if (!interior[nx][ny]) break;
                if (pmap[nx][ny].layers[DUNGEON] != FLOOR) break;

                // Must also be wall-adjacent
                boolean adj = false;
                if (cellHasTerrainFlag((pos){nx-1, ny}, T_OBSTRUCTS_PASSABILITY)) adj = true;
                if (cellHasTerrainFlag((pos){nx+1, ny}, T_OBSTRUCTS_PASSABILITY)) adj = true;
                if (cellHasTerrainFlag((pos){nx, ny-1}, T_OBSTRUCTS_PASSABILITY)) adj = true;
                if (cellHasTerrainFlag((pos){nx, ny+1}, T_OBSTRUCTS_PASSABILITY)) adj = true;
                if (!adj) break;

                line[len++] = (cell){nx, ny};
            }

            if (len > bestLen) {
                bestLen = len;
                for (short k = 0; k < len; k++) bestLine[k] = line[k];
            }
        }
    }

    if (bestLen < 2) return false;

    // Place foliage on the line
    for (short i = 0; i < bestLen; i++) {
        pmap[bestLine[i].x][bestLine[i].y].layers[SURFACE] = FOLIAGE;
    }

    // Center on the middle of the line
    short mid = bestLen / 2;
    if (outCenter) {
        outCenter->x = bestLine[mid].x;
        outCenter->y = bestLine[mid].y;
    }
    return true;
}

// Returns true if layout was placed, false if insufficient wall-adjacent cells.
static boolean applyCobwebCornerLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Collect interior cells adjacent to at least one wall, near the origin.
    typedef struct { short x, y; short wallCount; } candidate;
    candidate candidates[20];
    short candidateCount = 0;

    for (short dy = -4; dy <= 4 && candidateCount < 20; dy++) {
        for (short dx = -4; dx <= 4 && candidateCount < 20; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            // Count adjacent walls (non-interior cells that are passability-blocking)
            short walls = 0;
            if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) walls++;

            if (walls > 0) {
                candidates[candidateCount++] = (candidate){x, y, walls};
            }
        }
    }

    if (candidateCount < 2) return false;

    // Sort by wall count descending (prefer corner cells with 2+ walls).
    for (short i = 0; i < candidateCount - 1; i++) {
        for (short j = i + 1; j < candidateCount; j++) {
            if (candidates[j].wallCount > candidates[i].wallCount) {
                candidate tmp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = tmp;
            }
        }
    }

    // Place 2-3 spiderwebs on the best wall-adjacent cells.
    short webCount = min(3, candidateCount);
    for (short i = 0; i < webCount; i++) {
        pmap[candidates[i].x][candidates[i].y].layers[SURFACE] = SPIDERWEB;
    }

    // Place 2-3 bones on nearby interior cells (wall-adjacent candidates or open floor).
    short bonesPlaced = 0;
    short bonesTarget = 2 + (candidateCount > 5 ? 1 : 0); // 2-3 bones

    // First, use remaining wall-adjacent candidates after the webs.
    for (short i = webCount; i < candidateCount && bonesPlaced < bonesTarget; i++) {
        pmap[candidates[i].x][candidates[i].y].layers[SURFACE] = BONES;
        bonesPlaced++;
    }

    // If still need more bones, scatter on open interior cells near the webs.
    for (short dy = -2; dy <= 2 && bonesPlaced < bonesTarget; dy++) {
        for (short dx = -2; dx <= 2 && bonesPlaced < bonesTarget; dx++) {
            short x = candidates[0].x + dx;
            short y = candidates[0].y + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[SURFACE] != NOTHING) continue;
            pmap[x][y].layers[SURFACE] = BONES;
            bonesPlaced++;
        }
    }

    if (outCenter) {
        outCenter->x = candidates[0].x;
        outCenter->y = candidates[0].y;
    }
    return true;
}

// Place a crumbled wall: STATUE_INERT column stub against a wall, with RUBBLE in an L-shape.
// Finds a wall-adjacent interior cell near the origin, places the column stub there,
// and scatters 2-3 rubble tiles adjacent to it (forming an L away from the wall).
// Returns true if layout was placed, false if no suitable wall-adjacent cell.
static boolean applyCrumbledWallLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Collect interior cells adjacent to at least one wall, near the origin.
    typedef struct { short x, y; short wallCount; } candidate;
    candidate candidates[20];
    short candidateCount = 0;

    for (short dy = -4; dy <= 4 && candidateCount < 20; dy++) {
        for (short dx = -4; dx <= 4 && candidateCount < 20; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            // Count adjacent walls
            short walls = 0;
            if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) walls++;

            if (walls >= 1) {
                candidates[candidateCount++] = (candidate){x, y, walls};
            }
        }
    }

    if (candidateCount < 1) return false;

    // Sort by wall count descending (prefer corner cells).
    for (short i = 0; i < candidateCount - 1; i++) {
        for (short j = i + 1; j < candidateCount; j++) {
            if (candidates[j].wallCount > candidates[i].wallCount) {
                candidate tmp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = tmp;
            }
        }
    }

    // Place column stub (STATUE_INERT) at the best wall-adjacent cell.
    // Check connectivity: placing a blocking tile must not disconnect the level.
    short colX = candidates[0].x, colY = candidates[0].y;

    // Check connectivity before placing the blocking statue.
    char blockingMap[DCOLS][DROWS];
    zeroOutGrid(blockingMap);
    blockingMap[colX][colY] = true;
    if (levelIsDisconnectedWithBlockingMap(blockingMap, false)) {
        return false;
    }
    pmap[colX][colY].layers[DUNGEON] = STATUE_INERT;

    // Place 2-3 rubble tiles on adjacent interior floor cells (not wall-side).
    short rubblePlaced = 0;
    short rubbleTarget = 2 + rand_range(0, 1); // 2-3 rubble
    // Prefer cells away from the wall (interior direction).
    for (short dy = -1; dy <= 1 && rubblePlaced < rubbleTarget; dy++) {
        for (short dx = -1; dx <= 1 && rubblePlaced < rubbleTarget; dx++) {
            if (dx == 0 && dy == 0) continue;
            short rx = colX + dx;
            short ry = colY + dy;
            if (rx < 1 || rx >= DCOLS - 1 || ry < 1 || ry >= DROWS - 1) continue;
            if (!interior[rx][ry]) continue;
            if (pmap[rx][ry].layers[DUNGEON] != FLOOR) continue;
            if (pmap[rx][ry].layers[SURFACE] != NOTHING) continue;
            // Skip cells that are also wall-adjacent on the same side (we want rubble spilling inward).
            pmap[rx][ry].layers[SURFACE] = RUBBLE;
            rubblePlaced++;
        }
    }

    if (rubblePlaced < 2) {
        // Not enough space for rubble; abort.
        pmap[colX][colY].layers[DUNGEON] = FLOOR;
        for (short dy = -1; dy <= 1; dy++) {
            for (short dx = -1; dx <= 1; dx++) {
                short rx = colX + dx;
                short ry = colY + dy;
                if (rx >= 0 && rx < DCOLS && ry >= 0 && ry < DROWS) {
                    if (pmap[rx][ry].layers[SURFACE] == RUBBLE) {
                        pmap[rx][ry].layers[SURFACE] = NOTHING;
                    }
                }
            }
        }
        return false;
    }

    if (outCenter) {
        outCenter->x = colX;
        outCenter->y = colY;
    }
    return true;
}

// Place a bird nest: statue (stone perch) at center, spiderweb + hay + bones on adjacent cells.
// Finds an interior cell near origin with at least 2 adjacent interior cells.
static boolean applyBirdNestLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    short bestX = 0, bestY = 0, bestAdj = 0;

    for (short dy = -3; dy <= 3; dy++) {
        for (short dx = -3; dx <= 3; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;

            short adj = 0;
            if (interior[x-1][y]) adj++;
            if (interior[x+1][y]) adj++;
            if (interior[x][y-1]) adj++;
            if (interior[x][y+1]) adj++;

            if (adj > bestAdj) {
                bestAdj = adj;
                bestX = x;
                bestY = y;
            }
        }
    }

    if (bestAdj < 2) return false;

    // Place statue (stone perch) at center
    pmap[bestX][bestY].layers[DUNGEON] = STATUE_INERT;
    pmap[bestX][bestY].layers[SURFACE] = NOTHING;

    // Place spiderweb, hay, and bones on adjacent interior cells
    short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    short placed = 0;
    for (short i = 0; i < 4 && placed < 3; i++) {
        short nx = bestX + dirs[i][0];
        short ny = bestY + dirs[i][1];
        if (nx < 0 || nx >= DCOLS || ny < 0 || ny >= DROWS) continue;
        if (!interior[nx][ny]) continue;

        if (placed == 0) {
            pmap[nx][ny].layers[SURFACE] = SPIDERWEB;
        } else if (placed == 1) {
            pmap[nx][ny].layers[SURFACE] = HAY;
        } else {
            pmap[nx][ny].layers[SURFACE] = BONES;
        }
        placed++;
    }

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }
    return true;
}

// Place a tight mossy alcove: shallow water at center, surrounded by foliage and grass.
// Finds a suitable interior cell near the origin and places a 3-5 tile cluster.
// Returns true if layout was placed, false if insufficient space.
static boolean applyMossyAlcoveLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find an interior cell near the origin that has at least 2 adjacent interior cells.
    short bestX = 0, bestY = 0, bestAdj = 0;

    for (short dy = -3; dy <= 3; dy++) {
        for (short dx = -3; dx <= 3; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;

            // Count adjacent interior cells
            short adj = 0;
            if (interior[x-1][y]) adj++;
            if (interior[x+1][y]) adj++;
            if (interior[x][y-1]) adj++;
            if (interior[x][y+1]) adj++;

            if (adj > bestAdj) {
                bestAdj = adj;
                bestX = x;
                bestY = y;
            }
        }
    }

    if (bestAdj < 2) return false;

    // Place shallow water at center
    pmap[bestX][bestY].layers[LIQUID] = SHALLOW_WATER;
    pmap[bestX][bestY].layers[SURFACE] = NOTHING;

    // Place foliage and grass on adjacent interior cells
    short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    short placed = 0;
    for (short i = 0; i < 4 && placed < 4; i++) {
        short nx = bestX + dirs[i][0];
        short ny = bestY + dirs[i][1];
        if (nx < 0 || nx >= DCOLS || ny < 0 || ny >= DROWS) continue;
        if (!interior[nx][ny]) continue;

        if (placed % 2 == 0) {
            pmap[nx][ny].layers[SURFACE] = FOLIAGE;
        } else {
            pmap[nx][ny].layers[SURFACE] = GRASS;
        }
        placed++;
    }

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }
    return true;
}

// Place a 3-wide alternating-row garden pattern (foliage rows / water rows).
// Extends vertically from the origin, using only cells inside the machine interior.
// Returns true if layout was placed, false if insufficient space.
// Sets *outCenter to the center of the placed pattern.
static boolean applyGardenLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find the longest 3-wide run of interior cells in any of 4 directions.
    // Directions: vertical (dx=0,dy=1), horizontal (dx=1,dy=0), and their reverses.
    // "3-wide" means the center cell and its two perpendicular neighbors are all interior.
    static const short dirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    short bestCX = 0, bestCY = 0, bestDir = -1, bestLen = 0;

    for (short d = 0; d < 4; d++) {
        short mainDx = dirs[d][0], mainDy = dirs[d][1];
        // Perpendicular direction: rotate 90°
        short perpDx = -mainDy, perpDy = mainDx;

        for (short cx = 2; cx < DCOLS - 2; cx++) {
            for (short cy = 2; cy < DROWS - 2; cy++) {
                // Scan along main direction from (cx, cy)
                short len = 0;
                while (len < 10) {
                    short x = cx + mainDx * len;
                    short y = cy + mainDy * len;
                    if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) break;
                    if (!interior[x][y]) break;
                    short px1 = x + perpDx, py1 = y + perpDy;
                    short px2 = x - perpDx, py2 = y - perpDy;
                    if (px1 < 1 || px1 >= DCOLS-1 || py1 < 1 || py1 >= DROWS-1) break;
                    if (px2 < 1 || px2 >= DCOLS-1 || py2 < 1 || py2 >= DROWS-1) break;
                    if (!interior[px1][py1] || !interior[px2][py2]) break;
                    len++;
                }
                if (len > bestLen) {
                    bestLen = len;
                    bestCX = cx;
                    bestCY = cy;
                    bestDir = d;
                }
            }
        }
    }

    // Clamp to 4-7 rows.
    if (bestLen > 7) {
        short excess = bestLen - 7;
        bestCX += dirs[bestDir][0] * (excess / 2);
        bestCY += dirs[bestDir][1] * (excess / 2);
        bestLen = 7;
    }
    if (bestLen < 4) return false;

    short mainDx = dirs[bestDir][0], mainDy = dirs[bestDir][1];
    short perpDx = -mainDy, perpDy = mainDx;

    if (outCenter) {
        outCenter->x = bestCX + mainDx * (bestLen / 2);
        outCenter->y = bestCY + mainDy * (bestLen / 2);
    }

    // Place alternating rows: even offset = foliage, odd offset = shallow water.
    for (short i = 0; i < bestLen; i++) {
        enum tileType terrain = (i % 2 == 0) ? FOLIAGE : SHALLOW_WATER;
        enum dungeonLayers layer = (i % 2 == 0) ? SURFACE : LIQUID;
        short x = bestCX + mainDx * i;
        short y = bestCY + mainDy * i;
        for (short s = -1; s <= 1; s++) {
            short px = x + perpDx * s;
            short py = y + perpDy * s;
            pmap[px][py].layers[layer] = terrain;
            if (layer == LIQUID) {
                pmap[px][py].layers[SURFACE] = NOTHING;
            } else {
                pmap[px][py].layers[LIQUID] = NOTHING;
            }
        }
    }
    return true;
}

// Count how many ring positions at a given center are inside the interior.
// Large ring: dist² in {5,8,9} = 16 possible positions (radius ~2.2-3.0).
// Small ring: dist² in {2,4} = 8 possible positions (radius ~1.4-2.0).
static void countRingPositions(short cx, short cy, char interior[DCOLS][DROWS],
                               short *largeRing, short *smallRing) {
    *largeRing = 0;
    *smallRing = 0;
    for (short ry = -3; ry <= 3; ry++) {
        for (short rx = -3; rx <= 3; rx++) {
            short d2 = rx * rx + ry * ry;
            short nx = cx + rx, ny = cy + ry;
            if (nx < 0 || nx >= DCOLS || ny < 0 || ny >= DROWS) continue;
            if (!interior[nx][ny]) continue;
            if (d2 == 5 || d2 == 8 || d2 == 9) (*largeRing)++;
            if (d2 == 2 || d2 == 4) (*smallRing)++;
        }
    }
}

// Place a glowing fairy ring of fungus around a grassy hollow.
// Tries large ring first (7x7, dist²{5,8,9} ring around dist²≤4 grass interior).
// Falls back to small ring (5x5, dist²{2,4} ring around dist²≤1 grass interior).
// Up to 25% of ring tiles may be missing due to wall clipping for natural irregularity.
static boolean applyMushroomCircleLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    short bestX = 0, bestY = 0;
    short bestLarge = 0, bestSmall = 0;

    for (short dy = -5; dy <= 5; dy++) {
        for (short dx = -5; dx <= 5; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 3 || x >= DCOLS - 3 || y < 3 || y >= DROWS - 3) continue;
            if (!interior[x][y]) continue;

            short largeRing, smallRing;
            countRingPositions(x, y, interior, &largeRing, &smallRing);

            // Prefer large ring; track best of each
            if (largeRing > bestLarge || (largeRing == bestLarge && smallRing > bestSmall)) {
                bestLarge = largeRing;
                bestSmall = smallRing;
                bestX = x;
                bestY = y;
            }
        }
    }

    // Large ring: 16 positions, need 75% = 12+
    // Small ring: 8 positions, need 75% = 6+
    boolean useLarge = (bestLarge >= 12);
    if (!useLarge && bestSmall < 6) return false;

    // If we're using the small ring, re-find the best center optimized for small ring
    if (!useLarge) {
        short bestSmallScore = 0;
        for (short dy = -5; dy <= 5; dy++) {
            for (short dx = -5; dx <= 5; dx++) {
                short x = originX + dx;
                short y = originY + dy;
                if (x < 2 || x >= DCOLS - 2 || y < 2 || y >= DROWS - 2) continue;
                if (!interior[x][y]) continue;

                short largeRing, smallRing;
                countRingPositions(x, y, interior, &largeRing, &smallRing);

                if (smallRing > bestSmallScore) {
                    bestSmallScore = smallRing;
                    bestX = x;
                    bestY = y;
                }
            }
        }
        if (bestSmallScore < 6) return false;
    }

    // Place tiles
    short ringCount = 0;
    short radius = useLarge ? 3 : 2;

    for (short dy = -radius; dy <= radius; dy++) {
        for (short dx = -radius; dx <= radius; dx++) {
            short x = bestX + dx;
            short y = bestY + dy;
            if (x < 0 || x >= DCOLS || y < 0 || y >= DROWS) continue;
            if (!interior[x][y]) continue;

            short dist2 = dx * dx + dy * dy;
            boolean isGrass, isRing;

            if (useLarge) {
                isGrass = (dist2 <= 4);
                isRing = (dist2 == 5 || dist2 == 8 || dist2 == 9);
            } else {
                isGrass = (dist2 <= 1);
                isRing = (dist2 == 2 || dist2 == 4);
            }

            if (isGrass) {
                pmap[x][y].layers[SURFACE] = GRASS;
                pmap[x][y].layers[LIQUID] = NOTHING;
            } else if (isRing) {
                if (ringCount % 3 == 0) {
                    pmap[x][y].layers[SURFACE] = LUMINESCENT_FUNGUS;
                } else {
                    pmap[x][y].layers[SURFACE] = FUNGUS_FOREST;
                }
                pmap[x][y].layers[LIQUID] = NOTHING;
                ringCount++;
            }
        }
    }

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }
    return true;
}

// Place a crystal outcrop: 2 crystal walls side-by-side against a wall, with luminescent fungus around them.
// Finds wall-adjacent interior floor cells and places crystals + fungus in a coherent cluster.
static boolean applyCrystalOutcropLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Collect interior cells adjacent to at least one wall, near the origin.
    typedef struct { short x, y; short wallCount; } candidate;
    candidate candidates[30];
    short candidateCount = 0;

    for (short dy = -5; dy <= 5 && candidateCount < 30; dy++) {
        for (short dx = -5; dx <= 5 && candidateCount < 30; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            short walls = 0;
            if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) walls++;

            if (walls >= 1) {
                candidates[candidateCount++] = (candidate){x, y, walls};
            }
        }
    }

    if (candidateCount < 2) return false;

    // Sort by wall count descending (prefer cells with more wall contact).
    for (short i = 0; i < candidateCount - 1; i++) {
        for (short j = i + 1; j < candidateCount; j++) {
            if (candidates[j].wallCount > candidates[i].wallCount) {
                candidate tmp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = tmp;
            }
        }
    }

    // Find a pair of adjacent wall-adjacent candidates for the two crystal walls.
    short c1x = -1, c1y = -1, c2x = -1, c2y = -1;
    for (short i = 0; i < candidateCount && c1x < 0; i++) {
        for (short j = i + 1; j < candidateCount && c1x < 0; j++) {
            short dx = candidates[i].x - candidates[j].x;
            short dy = candidates[i].y - candidates[j].y;
            if (abs(dx) + abs(dy) == 1) { // cardinally adjacent
                c1x = candidates[i].x; c1y = candidates[i].y;
                c2x = candidates[j].x; c2y = candidates[j].y;
            }
        }
    }

    if (c1x < 0) return false; // no adjacent pair found

    // Check connectivity: placing two blocking tiles must not disconnect the level.
    char blockingMap[DCOLS][DROWS];
    zeroOutGrid(blockingMap);
    blockingMap[c1x][c1y] = true;
    blockingMap[c2x][c2y] = true;
    if (levelIsDisconnectedWithBlockingMap(blockingMap, false)) {
        return false;
    }

    // Place the two crystal walls.
    pmap[c1x][c1y].layers[DUNGEON] = CRYSTAL_WALL;
    pmap[c2x][c2y].layers[DUNGEON] = CRYSTAL_WALL;

    // Place luminescent fungus on adjacent interior floor cells.
    short fungusPlaced = 0;
    short fungusTarget = 3 + rand_range(0, 2); // 3-5 fungus
    for (short dy = -1; dy <= 1 && fungusPlaced < fungusTarget; dy++) {
        for (short dx = -1; dx <= 1 && fungusPlaced < fungusTarget; dx++) {
            if (dx == 0 && dy == 0) continue;
            // Check around both crystal positions.
            for (int c = 0; c < 2; c++) {
                short fx = (c == 0 ? c1x : c2x) + dx;
                short fy = (c == 0 ? c1y : c2y) + dy;
                if (fx < 1 || fx >= DCOLS - 1 || fy < 1 || fy >= DROWS - 1) continue;
                if (!interior[fx][fy]) continue;
                if (pmap[fx][fy].layers[DUNGEON] != FLOOR) continue;
                if (pmap[fx][fy].layers[SURFACE] != NOTHING) continue;
                pmap[fx][fy].layers[SURFACE] = LUMINESCENT_FUNGUS;
                fungusPlaced++;
                if (fungusPlaced >= fungusTarget) break;
            }
        }
    }

    // Center on the midpoint of the two crystals.
    if (outCenter) {
        outCenter->x = (c1x + c2x) / 2;
        outCenter->y = (c1y + c2y) / 2;
    }
    return true;
}

// Place a weapon rack against a wall: STATUE_INERT (rack) with 1-2 JUNK tiles flanking it.
// ~30% chance to spawn a WEAPON item adjacent to the rack.
static boolean applyWeaponRackLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find wall-adjacent interior floor cells near origin.
    typedef struct { short x, y; short wallCount; } candidate;
    candidate candidates[20];
    short candidateCount = 0;

    for (short dy = -4; dy <= 4 && candidateCount < 20; dy++) {
        for (short dx = -4; dx <= 4 && candidateCount < 20; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            short walls = 0;
            if (cellHasTerrainFlag((pos){x-1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x+1, y}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y-1}, T_OBSTRUCTS_PASSABILITY)) walls++;
            if (cellHasTerrainFlag((pos){x, y+1}, T_OBSTRUCTS_PASSABILITY)) walls++;

            if (walls >= 1) {
                candidates[candidateCount++] = (candidate){x, y, walls};
            }
        }
    }

    if (candidateCount < 1) return false;

    // Sort by wall count descending (prefer corner cells).
    for (short i = 0; i < candidateCount - 1; i++) {
        for (short j = i + 1; j < candidateCount; j++) {
            if (candidates[j].wallCount > candidates[i].wallCount) {
                candidate tmp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = tmp;
            }
        }
    }

    short rackX = candidates[0].x, rackY = candidates[0].y;

    // Check connectivity before placing the blocking statue.
    char blockingMap[DCOLS][DROWS];
    zeroOutGrid(blockingMap);
    blockingMap[rackX][rackY] = true;
    if (levelIsDisconnectedWithBlockingMap(blockingMap, false)) {
        return false;
    }
    pmap[rackX][rackY].layers[DUNGEON] = STATUE_INERT;

    // Place 1-2 junk tiles on adjacent interior floor cells (prefer non-wall side).
    short junkPlaced = 0;
    short junkTarget = 1 + rand_range(0, 1); // 1-2 junk
    for (short dy = -1; dy <= 1 && junkPlaced < junkTarget; dy++) {
        for (short dx = -1; dx <= 1 && junkPlaced < junkTarget; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (dx != 0 && dy != 0) continue; // cardinal only
            short jx = rackX + dx;
            short jy = rackY + dy;
            if (jx < 1 || jx >= DCOLS - 1 || jy < 1 || jy >= DROWS - 1) continue;
            if (!interior[jx][jy]) continue;
            if (pmap[jx][jy].layers[DUNGEON] != FLOOR) continue;
            if (pmap[jx][jy].layers[SURFACE] != NOTHING) continue;
            // Skip cells that are wall-adjacent (prefer spilling inward).
            if (cellHasTerrainFlag((pos){jx, jy}, T_OBSTRUCTS_PASSABILITY)) continue;
            pmap[jx][jy].layers[SURFACE] = JUNK;
            junkPlaced++;
        }
    }

    if (junkPlaced < 1) {
        // Not enough space; abort.
        pmap[rackX][rackY].layers[DUNGEON] = FLOOR;
        return false;
    }

    if (outCenter) {
        *outCenter = (pos){rackX, rackY};
    }

    // ~60% chance to spawn a WEAPON item nearby.
    if (rand_percent(60)) {
        short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
        for (short i = 0; i < 4; i++) {
            short ix = rackX + dirs[i][0];
            short iy = rackY + dirs[i][1];
            if (ix < 1 || ix >= DCOLS - 1 || iy < 1 || iy >= DROWS - 1) continue;
            if (!interior[ix][iy]) continue;
            if (pmap[ix][iy].layers[DUNGEON] != FLOOR) continue;
            if (pmap[ix][iy].layers[SURFACE] == NOTHING || pmap[ix][iy].layers[SURFACE] == JUNK) {
                item *loot = generateItem(WEAPON, -1);
                placeItemAt(loot, (pos){ix, iy});
                break;
            }
        }
    }

    return true;
}

// Place a toppled bookcase in a wall nook (floor cell with 3 cardinal walls).
// STATUE_INERT for the bookcase frame, JUNK on the open side for spilled books.
// ~40% chance of a SCROLL item on the junk tile.
static boolean applyToppledBookcaseLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find interior floor cells with exactly 3 cardinal walls (a nook/alcove).
    typedef struct { short x, y; } candidate;
    candidate candidates[40];
    short candidateCount = 0;

    for (short dy = -5; dy <= 5 && candidateCount < 40; dy++) {
        for (short dx = -5; dx <= 5 && candidateCount < 40; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            short walls = 0;
            short openX = 0, openY = 0;
            short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
            for (short d = 0; d < 4; d++) {
                short nx = x + dirs[d][0];
                short ny = y + dirs[d][1];
                if (cellHasTerrainFlag((pos){nx, ny}, T_OBSTRUCTS_PASSABILITY)) {
                    walls++;
                } else {
                    openX = nx;
                    openY = ny;
                }
            }

            if (walls == 3) {
                // Verify the open neighbor is interior floor for junk placement.
                if (openX >= 1 && openX < DCOLS - 1 && openY >= 1 && openY < DROWS - 1
                    && interior[openX][openY]
                    && pmap[openX][openY].layers[DUNGEON] == FLOOR) {
                    candidates[candidateCount++] = (candidate){x, y};
                }
            }
        }
    }

    if (candidateCount < 1) return false;

    // Pick the candidate closest to origin.
    short bestIdx = 0;
    short bestDist = 10000;
    for (short i = 0; i < candidateCount; i++) {
        short dist = (candidates[i].x - originX) * (candidates[i].x - originX)
                   + (candidates[i].y - originY) * (candidates[i].y - originY);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i;
        }
    }

    short nookX = candidates[bestIdx].x;
    short nookY = candidates[bestIdx].y;

    // Find the open cardinal neighbor again.
    short junkX = 0, junkY = 0;
    short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    for (short d = 0; d < 4; d++) {
        short nx = nookX + dirs[d][0];
        short ny = nookY + dirs[d][1];
        if (!cellHasTerrainFlag((pos){nx, ny}, T_OBSTRUCTS_PASSABILITY)) {
            junkX = nx;
            junkY = ny;
            break;
        }
    }

    // Connectivity check — the bookcase blocks pathing.
    char blockingMap[DCOLS][DROWS];
    zeroOutGrid(blockingMap);
    blockingMap[nookX][nookY] = true;
    if (levelIsDisconnectedWithBlockingMap(blockingMap, false)) {
        return false;
    }

    pmap[nookX][nookY].layers[DUNGEON] = STATUE_INERT;
    pmap[junkX][junkY].layers[SURFACE] = JUNK;

    if (outCenter) {
        *outCenter = (pos){nookX, nookY};
    }

    // ~80% chance to spawn a SCROLL on the junk tile.
    if (rand_percent(80)) {
        item *loot = generateItem(SCROLL, -1);
        placeItemAt(loot, (pos){junkX, junkY});
    }

    return true;
}

// Place an abandoned camp: bedroll (HAY), fire ring (RUBBLE+EMBERS), marker post (STATUE_INERT),
// with scattered bones and junk. ~40% chance of FOOD or POTION loot.
// Tries all 4 rotations via shared helper.
static boolean applyAbandonedCampLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    static const layoutTile campPat[] = {
        // Row 0: bedroll
        { 0, 0, HAY,          SURFACE, false},
        { 1, 0, HAY,          SURFACE, false},
        // Row 1: meal scraps
        { 0, 1, BONES,        SURFACE, false},
        // Row 2: fire ring (rubble flanking embers)
        {-1, 2, RUBBLE,       SURFACE, false},
        { 0, 2, EMBERS,       SURFACE, false},
        { 1, 2, RUBBLE,       SURFACE, false},
        // Row 3: fire ring base + junk
        { 0, 3, RUBBLE,       SURFACE, false},
        { 1, 3, JUNK,         SURFACE, false},
        // Row 4: marker post
        { 0, 4, STATUE_INERT, DUNGEON, false},
    };
    if (!applyRotatableLayout(originX, originY, interior,
                              campPat, sizeof(campPat) / sizeof(campPat[0]),
                              0, 2, outCenter)) {
        return false;
    }

    // ~40% chance to spawn a FOOD or POTION item near the marker post.
    if (outCenter && rand_percent(40)) {
        // Find an open floor cell adjacent to the effective center for the item.
        short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
        for (short i = 0; i < 4; i++) {
            short ix = outCenter->x + dirs[i][0];
            short iy = outCenter->y + dirs[i][1];
            if (ix < 1 || ix >= DCOLS - 1 || iy < 1 || iy >= DROWS - 1) continue;
            if (!interior[ix][iy]) continue;
            if (pmap[ix][iy].layers[DUNGEON] != FLOOR) continue;
            if (pmap[ix][iy].layers[SURFACE] == NOTHING) {
                unsigned short category = rand_percent(50) ? FOOD : POTION;
                item *loot = generateItem(category, -1);
                placeItemAt(loot, (pos){ix, iy});
                break;
            }
        }
    }

    return true;
}

// Place a bone throne: STATUE_INERT throne on marble dais with carpet runner.
// Bones and blood are randomly scattered on nearby interior cells for organic variety.
// ~30% chance of RING or GOLD loot near the throne.
// Tries all 4 rotations via shared helper.
static boolean applyBoneThroneLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Fixed structure: throne on marble dais with carpet runner
    static const layoutTile thronePat[] = {
        // Row 0: throne on marble dais
        { 0, 0, STATUE_INERT, DUNGEON, false},
        {-1, 0, MARBLE_FLOOR, DUNGEON, false},
        { 1, 0, MARBLE_FLOOR, DUNGEON, false},
        // Row 1: marble flanking carpet
        {-1, 1, MARBLE_FLOOR, DUNGEON, false},
        { 0, 1, CARPET,       DUNGEON, false},
        { 1, 1, MARBLE_FLOOR, DUNGEON, false},
        // Rows 2-3: carpet runner
        { 0, 2, CARPET,       DUNGEON, false},
        { 0, 3, CARPET,       DUNGEON, false},
    };
    if (!applyRotatableLayout(originX, originY, interior,
                              thronePat, sizeof(thronePat) / sizeof(thronePat[0]),
                              0, 1, outCenter)) {
        return false;
    }

    // Scatter bones and blood randomly on nearby interior floor cells.
    // Collect eligible cells within 3 steps of the effective center.
    if (!outCenter) return true;
    short cx = outCenter->x;
    short cy = outCenter->y;

    typedef struct { short x, y; } cell;
    cell candidates[60];
    short candCount = 0;

    for (short dy = -3; dy <= 3 && candCount < 60; dy++) {
        for (short dx = -3; dx <= 3 && candCount < 60; dx++) {
            short x = cx + dx;
            short y = cy + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            // Skip cells already used by the fixed pattern
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;
            candidates[candCount++] = (cell){x, y};
        }
    }

    // Shuffle candidates
    for (short i = candCount - 1; i > 0; i--) {
        short j = rand_range(0, i);
        cell tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
    }

    // Place 3-5 bones and 3-5 blood stains from the shuffled list
    short bonesTarget = rand_range(3, 5);
    short bloodTarget = rand_range(3, 5);
    short bonesPlaced = 0, bloodPlaced = 0;

    for (short i = 0; i < candCount && (bonesPlaced < bonesTarget || bloodPlaced < bloodTarget); i++) {
        if (bonesPlaced < bonesTarget) {
            pmap[candidates[i].x][candidates[i].y].layers[SURFACE] = BONES;
            bonesPlaced++;
        } else if (bloodPlaced < bloodTarget) {
            pmap[candidates[i].x][candidates[i].y].layers[SURFACE] = RED_BLOOD;
            bloodPlaced++;
        }
    }

    // ~30% chance to spawn a RING or GOLD item near the throne.
    if (rand_percent(30)) {
        short dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
        for (short i = 0; i < 4; i++) {
            short ix = cx + dirs[i][0];
            short iy = cy + dirs[i][1];
            if (ix < 1 || ix >= DCOLS - 1 || iy < 1 || iy >= DROWS - 1) continue;
            if (!interior[ix][iy]) continue;
            if (pmap[ix][iy].layers[DUNGEON] != FLOOR && pmap[ix][iy].layers[DUNGEON] != CARPET) continue;
            unsigned short category = rand_percent(50) ? RING : GOLD;
            item *loot = generateItem(category, -1);
            placeItemAt(loot, (pos){ix, iy});
            break;
        }
    }

    return true;
}

// Place a blood pool: altar rising from a grand irregular lake of dried blood.
// Altar at center, large blood spread around it, bones scattered irregularly.
static boolean applyBloodPoolLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find best center with ample open interior (score = interior cells in 7x7 area)
    short bestX = 0, bestY = 0, bestScore = 0;
    for (short dy = -6; dy <= 6; dy++) {
        for (short dx = -6; dx <= 6; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 4 || x >= DCOLS - 4 || y < 3 || y >= DROWS - 3) continue;
            if (!interior[x][y]) continue;
            short score = 0;
            for (short ry = -3; ry <= 3; ry++) {
                for (short rx = -3; rx <= 3; rx++) {
                    short cx = x + rx, cy = y + ry;
                    if (cx >= 0 && cx < DCOLS && cy >= 0 && cy < DROWS && interior[cx][cy]) {
                        score++;
                    }
                }
            }
            if (score > bestScore) {
                bestScore = score;
                bestX = x;
                bestY = y;
            }
        }
    }
    // Need at least a 5x5 open area to look grand
    if (bestScore < 20) return false;

    // Place altar at center
    pmap[bestX][bestY].layers[DUNGEON] = ALTAR_INERT;

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }

    // Collect interior cells within radius 4 of the altar, excluding the altar itself
    typedef struct { short x, y; } cell;
    cell candidates[120];
    short candCount = 0;

    for (short dy = -4; dy <= 4 && candCount < 120; dy++) {
        for (short dx = -4; dx <= 4 && candCount < 120; dx++) {
            if (dx == 0 && dy == 0) continue;
            short x = bestX + dx;
            short y = bestY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;
            candidates[candCount++] = (cell){x, y};
        }
    }

    // Paint blood on cells close to the altar (Chebyshev distance <= 3),
    // with decreasing probability further out for irregular edges
    short bloodCount = 0;
    for (short i = 0; i < candCount; i++) {
        short dx = candidates[i].x - bestX;
        short dy = candidates[i].y - bestY;
        short dist = max(abs(dx), abs(dy));
        short chance;
        if (dist <= 1) chance = 95;
        else if (dist == 2) chance = 80;
        else if (dist == 3) chance = 55;
        else chance = 25;
        if (rand_percent(chance)) {
            pmap[candidates[i].x][candidates[i].y].layers[SURFACE] = RED_BLOOD;
            bloodCount++;
        }
    }

    // Need a meaningful amount of blood
    if (bloodCount < 8) return false;

    // Shuffle candidates for bone placement
    for (short i = candCount - 1; i > 0; i--) {
        short j = rand_range(0, i);
        cell tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
    }

    // Scatter 3-6 bones irregularly among the blood-stained cells
    short bonesTarget = rand_range(3, 6);
    short bonesPlaced = 0;
    for (short i = 0; i < candCount && bonesPlaced < bonesTarget; i++) {
        if (pmap[candidates[i].x][candidates[i].y].layers[SURFACE] == RED_BLOOD) {
            pmap[candidates[i].x][candidates[i].y].layers[SURFACE] = BONES;
            bonesPlaced++;
        }
    }

    return true;
}

// Place an obsidian formation: concentric thermal rings — obsidian core,
// embers ring, ash perimeter. Cooled lava remnants from an ancient flow.
static boolean applyObsidianFormationLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find best center with ample open interior (score = interior cells in 7x7 area)
    short bestX = 0, bestY = 0, bestScore = 0;
    for (short dy = -6; dy <= 6; dy++) {
        for (short dx = -6; dx <= 6; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 3 || x >= DCOLS - 3 || y < 3 || y >= DROWS - 3) continue;
            if (!interior[x][y]) continue;
            short score = 0;
            for (short ry = -3; ry <= 3; ry++) {
                for (short rx = -3; rx <= 3; rx++) {
                    short nx = x + rx, ny = y + ry;
                    if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS && interior[nx][ny]) {
                        score++;
                    }
                }
            }
            if (score > bestScore) {
                bestScore = score;
                bestX = x;
                bestY = y;
            }
        }
    }
    // Need at least a 5x5 open area
    if (bestScore < 20) return false;

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }

    // Paint concentric rings based on Chebyshev distance from center.
    // Distance 0-1: obsidian core, 2: embers ring, 3: ash perimeter.
    // Use randomized thresholds for irregular edges.
    for (short dy = -4; dy <= 4; dy++) {
        for (short dx = -4; dx <= 4; dx++) {
            short x = bestX + dx;
            short y = bestY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            short dist = max(abs(dx), abs(dy));
            if (dist <= 1) {
                // Core: always obsidian
                pmap[x][y].layers[DUNGEON] = OBSIDIAN;
            } else if (dist == 2) {
                // Inner ring: obsidian or embers
                if (rand_percent(40)) {
                    pmap[x][y].layers[DUNGEON] = OBSIDIAN;
                } else {
                    pmap[x][y].layers[SURFACE] = EMBERS;
                }
            } else if (dist == 3) {
                // Outer ring: embers or ash with falloff
                if (rand_percent(70)) {
                    if (rand_percent(35)) {
                        pmap[x][y].layers[SURFACE] = EMBERS;
                    } else {
                        pmap[x][y].layers[SURFACE] = ASH;
                    }
                }
            } else if (dist == 4) {
                // Fringe: sparse ash
                if (rand_percent(30)) {
                    pmap[x][y].layers[SURFACE] = ASH;
                }
            }
        }
    }

    return true;
}

// Place an ember pit (pyre): burnt stake at center surrounded by concentric rings
// of embers, ash, dead grass fringe, with bones scattered in the hot zone.
static boolean applyEmberPitLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find best center with ample open interior (score = interior cells in 7x7 area)
    short bestX = 0, bestY = 0, bestScore = 0;
    for (short dy = -6; dy <= 6; dy++) {
        for (short dx = -6; dx <= 6; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 3 || x >= DCOLS - 3 || y < 3 || y >= DROWS - 3) continue;
            if (!interior[x][y]) continue;
            short score = 0;
            for (short ry = -3; ry <= 3; ry++) {
                for (short rx = -3; rx <= 3; rx++) {
                    short nx = x + rx, ny = y + ry;
                    if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS && interior[nx][ny]) {
                        score++;
                    }
                }
            }
            if (score > bestScore) {
                bestScore = score;
                bestX = x;
                bestY = y;
            }
        }
    }
    // Need at least a 5x5 open area
    if (bestScore < 20) return false;

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }

    // Place the burnt stake at center
    pmap[bestX][bestY].layers[DUNGEON] = STATUE_INERT;

    // Paint concentric rings based on Chebyshev distance from center.
    // Distance 1: embers (hot zone), 2: ash, 3: dead grass fringe.
    // Scatter bones in the embers/ash zone.
    short bonesPlaced = 0;
    short bonesTarget = 2 + rand_range(0, 2); // 2-3 bones

    for (short dy = -4; dy <= 4; dy++) {
        for (short dx = -4; dx <= 4; dx++) {
            if (dx == 0 && dy == 0) continue; // skip center (stake)
            short x = bestX + dx;
            short y = bestY + dy;
            if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1) continue;
            if (!interior[x][y]) continue;
            if (pmap[x][y].layers[DUNGEON] != FLOOR) continue;

            short dist = max(abs(dx), abs(dy));
            if (dist == 1) {
                // Inner ring: embers, occasionally bones
                if (bonesPlaced < bonesTarget && rand_percent(25)) {
                    pmap[x][y].layers[SURFACE] = BONES;
                    bonesPlaced++;
                } else {
                    pmap[x][y].layers[SURFACE] = EMBERS;
                }
            } else if (dist == 2) {
                // Middle ring: ash, occasionally embers or bones
                if (rand_percent(75)) {
                    if (bonesPlaced < bonesTarget && rand_percent(20)) {
                        pmap[x][y].layers[SURFACE] = BONES;
                        bonesPlaced++;
                    } else if (rand_percent(30)) {
                        pmap[x][y].layers[SURFACE] = EMBERS;
                    } else {
                        pmap[x][y].layers[SURFACE] = ASH;
                    }
                }
            } else if (dist == 3) {
                // Outer ring: dead grass with ash falloff
                if (rand_percent(60)) {
                    if (rand_percent(25)) {
                        pmap[x][y].layers[SURFACE] = ASH;
                    } else {
                        pmap[x][y].layers[SURFACE] = DEAD_GRASS;
                    }
                }
            } else if (dist == 4) {
                // Fringe: sparse dead grass
                if (rand_percent(30)) {
                    pmap[x][y].layers[SURFACE] = DEAD_GRASS;
                }
            }
        }
    }

    return true;
}

// Place a lichen garden: 2-3 shallow water pools connected by luminescent fungus,
// surrounded by rings of fungus forest and dead grass fringe.
// Uses Chebyshev distance from water cells to paint concentric growth rings.
static boolean applyLichenGardenLayout(short originX, short originY, char interior[DCOLS][DROWS], pos *outCenter) {
    // Find best center with ample open interior (score = interior cells in 9x7 area)
    short bestX = 0, bestY = 0, bestScore = 0;
    for (short dy = -6; dy <= 6; dy++) {
        for (short dx = -6; dx <= 6; dx++) {
            short x = originX + dx;
            short y = originY + dy;
            if (x < 4 || x >= DCOLS - 4 || y < 3 || y >= DROWS - 3) continue;
            if (!interior[x][y]) continue;
            short score = 0;
            for (short ry = -3; ry <= 3; ry++) {
                for (short rx = -4; rx <= 4; rx++) {
                    short nx = x + rx, ny = y + ry;
                    if (nx >= 0 && nx < DCOLS && ny >= 0 && ny < DROWS && interior[nx][ny]) {
                        score++;
                    }
                }
            }
            if (score > bestScore) {
                bestScore = score;
                bestX = x;
                bestY = y;
            }
        }
    }
    if (bestScore < 15) return false;

    // Track water cells with a grid
    char waterGrid[DCOLS][DROWS];
    zeroOutGrid(waterGrid);

    short waterCells[20][2];
    short waterCount = 0;

    // Pool 1: at center + 1 adjacent tile
    waterGrid[bestX][bestY] = true;
    waterCells[waterCount][0] = bestX;
    waterCells[waterCount][1] = bestY;
    waterCount++;

    short card[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (int i = 3; i > 0; i--) {
        int j = rand_range(0, i);
        short t0 = card[i][0], t1 = card[i][1];
        card[i][0] = card[j][0]; card[i][1] = card[j][1];
        card[j][0] = t0; card[j][1] = t1;
    }
    for (int d = 0; d < 4 && waterCount < 2; d++) {
        short wx = bestX + card[d][0], wy = bestY + card[d][1];
        if (wx >= 1 && wx < DCOLS-1 && wy >= 1 && wy < DROWS-1
            && interior[wx][wy] && !waterGrid[wx][wy]) {
            waterGrid[wx][wy] = true;
            waterCells[waterCount][0] = wx;
            waterCells[waterCount][1] = wy;
            waterCount++;
        }
    }

    // Satellite pool offsets (shuffled for variety)
    short satOffsets[8][2] = {
        {3, 0}, {-3, 0}, {0, 2}, {0, -2},
        {2, 2}, {2, -2}, {-2, 2}, {-2, -2}
    };
    for (int i = 7; i > 0; i--) {
        int j = rand_range(0, i);
        short t0 = satOffsets[i][0], t1 = satOffsets[i][1];
        satOffsets[i][0] = satOffsets[j][0]; satOffsets[i][1] = satOffsets[j][1];
        satOffsets[j][0] = t0; satOffsets[j][1] = t1;
    }

    short numSatellites = rand_range(1, 2);
    short satPlaced = 0;
    for (int s = 0; s < 8 && satPlaced < numSatellites; s++) {
        short px = bestX + satOffsets[s][0];
        short py = bestY + satOffsets[s][1];
        if (px < 1 || px >= DCOLS-1 || py < 1 || py >= DROWS-1) continue;
        if (!interior[px][py] || waterGrid[px][py]) continue;

        waterGrid[px][py] = true;
        waterCells[waterCount][0] = px;
        waterCells[waterCount][1] = py;
        waterCount++;

        // Add 1-2 adjacent cells to this satellite pool
        short adj[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (int i = 3; i > 0; i--) {
            int j = rand_range(0, i);
            short t0 = adj[i][0], t1 = adj[i][1];
            adj[i][0] = adj[j][0]; adj[i][1] = adj[j][1];
            adj[j][0] = t0; adj[j][1] = t1;
        }
        short extra = rand_range(1, 2);
        for (int d = 0; d < 4 && extra > 0 && waterCount < 20; d++) {
            short wx = px + adj[d][0], wy = py + adj[d][1];
            if (wx < 1 || wx >= DCOLS-1 || wy < 1 || wy >= DROWS-1) continue;
            if (!interior[wx][wy] || waterGrid[wx][wy]) continue;
            waterGrid[wx][wy] = true;
            waterCells[waterCount][0] = wx;
            waterCells[waterCount][1] = wy;
            waterCount++;
            extra--;
        }
        satPlaced++;
    }

    if (waterCount < 3) return false;

    // Place water on the map
    for (int i = 0; i < waterCount; i++) {
        pmap[waterCells[i][0]][waterCells[i][1]].layers[LIQUID] = SHALLOW_WATER;
        pmap[waterCells[i][0]][waterCells[i][1]].layers[SURFACE] = NOTHING;
    }

    // Paint concentric rings by Chebyshev distance to nearest water cell:
    //   dist 1: LUMINESCENT_FUNGUS (glows!)
    //   dist 2: FUNGUS_FOREST
    //   dist 3: DEAD_GRASS (60% chance, for organic fringe)
    short totalPlaced = waterCount;
    short minY = (bestY - 7 > 1) ? bestY - 7 : 1;
    short maxY = (bestY + 7 < DROWS - 2) ? bestY + 7 : DROWS - 2;
    short minX = (bestX - 8 > 1) ? bestX - 8 : 1;
    short maxX = (bestX + 8 < DCOLS - 2) ? bestX + 8 : DCOLS - 2;

    for (short y = minY; y <= maxY; y++) {
        for (short x = minX; x <= maxX; x++) {
            if (!interior[x][y] || waterGrid[x][y]) continue;

            // Find min Chebyshev distance to any water cell
            short minDist = 999;
            for (int w = 0; w < waterCount; w++) {
                short adx = abs(x - waterCells[w][0]);
                short ady = abs(y - waterCells[w][1]);
                short d = (adx > ady) ? adx : ady;
                if (d < minDist) minDist = d;
            }

            if (minDist == 1) {
                pmap[x][y].layers[SURFACE] = LUMINESCENT_FUNGUS;
                pmap[x][y].layers[LIQUID] = NOTHING;
                totalPlaced++;
            } else if (minDist == 2) {
                pmap[x][y].layers[SURFACE] = FUNGUS_FOREST;
                pmap[x][y].layers[LIQUID] = NOTHING;
                totalPlaced++;
            } else if (minDist == 3 && rand_percent(60)) {
                pmap[x][y].layers[SURFACE] = DEAD_GRASS;
                totalPlaced++;
            }
        }
    }

    if (totalPlaced < 15) return false;

    if (outCenter) {
        outCenter->x = bestX;
        outCenter->y = bestY;
    }
    return true;
}

typedef struct machineData {
    // Our boolean grids:
    char interior[DCOLS][DROWS];    // This is the master grid for the machine. All area inside the machine are set to true.
    char occupied[DCOLS][DROWS];    // This keeps track of what is within the personal space of a previously built feature in the same machine.
    char candidates[DCOLS][DROWS];  // This is calculated at the start of each feature, and is true where that feature is eligible for building.
    char blockingMap[DCOLS][DROWS]; // Used during terrain/DF placement in features that are flagged not to tolerate blocking, to see if they block.
    char viewMap[DCOLS][DROWS];     // Used for features with MF_IN_VIEW_OF_ORIGIN, to calculate which cells are in view of the origin.

    pcell levelBackup[DCOLS][DROWS];

    item *spawnedItems[MACHINES_BUFFER_LENGTH];
    item *spawnedItemsSub[MACHINES_BUFFER_LENGTH];
    creature *spawnedMonsters[MACHINES_BUFFER_LENGTH];
    creature *spawnedMonstersSub[MACHINES_BUFFER_LENGTH];

    pos gateCandidates[50];
    short distances[100];
    short sRows[DROWS];
    short sCols[DCOLS];
} machineData;

// Returns true if the machine got built; false if it was aborted.
// If empty array parentSpawnedItems or parentSpawnedMonsters is given, will pass those back for deletion if necessary.
boolean buildAMachine(enum machineTypes bp,
                      short originX, short originY,
                      unsigned long requiredMachineFlags,
                      item *adoptiveItem,
                      item *parentSpawnedItems[MACHINES_BUFFER_LENGTH],
                      creature *parentSpawnedMonsters[MACHINES_BUFFER_LENGTH]) {

    short totalFreq, instance, instanceCount = 0,
        itemCount, monsterCount, qualifyingTileCount,
        **distanceMap = NULL,
        personalSpace, locationFailsafe,
        machineNumber;

    const unsigned long alternativeFlags[2] = {MF_ALTERNATIVE, MF_ALTERNATIVE_2};

    boolean DFSucceeded, terrainSucceeded, generateEverywhere,
        tryAgain, success = false, skipFeature[20];

    creature *monst = NULL, *torchBearer = NULL, *leader = NULL;

    item *theItem = NULL, *torch = NULL;

    const machineFeature *feature;

    machineData *p = malloc(sizeof(machineData));

    memset(p, 0, sizeof(machineData));

    const boolean chooseBP = (((signed short) bp) <= 0 ? true : false);

    const boolean chooseLocation = (originX <= 0 || originY <= 0 ? true : false);

    int failsafe = 10;
    do {
        tryAgain = false;
        if (--failsafe <= 0) {
            if (distanceMap) {
                freeGrid(distanceMap);
            }
            if (D_MESSAGE_MACHINE_GENERATION) {
                if (chooseBP || chooseLocation) {
                    printf("\nDepth %i: Failed to build a machine; gave up after 10 unsuccessful attempts to find a suitable blueprint and/or location.",
                           rogue.depthLevel);
                } else {
                    printf("\nDepth %i: Failed to build a machine; requested blueprint %i:%s and location did not work.",
                           rogue.depthLevel, bp, blueprintCatalog[bp].name);
                }
            }
            free(p);
            return false;
        }

        if (chooseBP) { // If no blueprint is given, then pick one:

            // First, choose the blueprint. We choose from among blueprints
            // that have the required blueprint flags and that satisfy the depth requirements.
            totalFreq = 0;
            for (int i=1; i<gameConst->numberBlueprints; i++) {
                if (blueprintQualifies(i, requiredMachineFlags)) {
                    totalFreq += blueprintCatalog[i].frequency;
                }
            }

            if (!totalFreq) { // If no suitable blueprints are in the library, fail.
                if (distanceMap) {
                    freeGrid(distanceMap);
                }
                if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to build a machine because no suitable blueprints were available.",
                             rogue.depthLevel);
                free(p);
                return false;
            }

            // Pick from among the suitable blueprints.
            int randIndex = rand_range(1, totalFreq);
            for (int i=1; i<gameConst->numberBlueprints; i++) {
                if (blueprintQualifies(i, requiredMachineFlags)) {
                    if (randIndex <= blueprintCatalog[i].frequency) {
                        bp = i;
                        break;
                    } else {
                        randIndex -= blueprintCatalog[i].frequency;
                    }
                }
            }

            // If we don't have a blueprint yet, something went wrong.
            brogueAssert(bp>0);
        }

        // Find a location and map out the machine interior.
        if (blueprintCatalog[bp].flags & BP_ROOM) {
            // If it's a room machine, count up the gates of appropriate
            // choke size and remember where they are. The origin of the room will be the gate location.
            zeroOutGrid(p->interior);

            if (chooseLocation) {
                analyzeMap(true); // Make sure the chokeMap is up to date.
                totalFreq = 0;
                for(int i=0; i<DCOLS; i++) {
                    for(int j=0; j<DROWS && totalFreq < 50; j++) {
                        if ((pmap[i][j].flags & IS_GATE_SITE)
                            && !(pmap[i][j].flags & IS_IN_MACHINE)
                            && chokeMap[i][j] >= blueprintCatalog[bp].roomSize[0]
                            && chokeMap[i][j] <= blueprintCatalog[bp].roomSize[1]) {

                            //DEBUG printf("\nDepth %i: Gate site qualified with interior size of %i.", rogue.depthLevel, chokeMap[i][j]);
                            p->gateCandidates[totalFreq] = (pos){ .x = i, .y = j };
                            totalFreq++;
                        }
                    }
                }

                if (totalFreq) {
                    // Choose the gate.
                    const int randIndex = rand_range(0, totalFreq - 1);
                    originX = p->gateCandidates[randIndex].x;
                    originY = p->gateCandidates[randIndex].y;
                } else {
                    // If no suitable sites, abort.
                    if (distanceMap) {
                        freeGrid(distanceMap);
                    }
                    if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to build a machine; there was no eligible door candidate for the chosen room machine from blueprint %i:%s.",
                                 rogue.depthLevel,
                                 bp,
                                 blueprintCatalog[bp].name);
                    free(p);
                    return false;
                }
            }

            // Now map out the interior into interior[][].
            // Start at the gate location and do a depth-first floodfill to grab all adjoining tiles with the
            // same or lower choke value, ignoring any tiles that are already part of a machine.
            // If we get false from this, try again. If we've tried too many times already, abort.
            tryAgain = !addTileToMachineInteriorAndIterate(p->interior, originX, originY);
        } else if (blueprintCatalog[bp].flags & BP_VESTIBULE) {
            if (chooseLocation) {
                // Door machines must have locations passed in. We can't pick one ourselves.
                if (distanceMap) {
                    freeGrid(distanceMap);
                }
                if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: ERROR: Attempted to build a door machine from blueprint %i:%s without a location being provided.",
                             rogue.depthLevel,
                             bp,
                             blueprintCatalog[bp].name);
                free(p);
                return false;
            }
            if (!fillInteriorForVestibuleMachine(p->interior, bp, originX, originY)) {
                if (distanceMap) {
                    freeGrid(distanceMap);
                }
                if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to build a door machine from blueprint %i:%s; not enough room.",
                             rogue.depthLevel,
                             bp,
                             blueprintCatalog[bp].name);
                free(p);
                return false;
            }
        } else {
            // Find a location and map out the interior for a non-room machine.
            // The strategy here is simply to pick a random location on the map,
            // expand it along a pathing map by one space in all directions until the size reaches
            // the chosen size, and then make sure the resulting space qualifies.
            // If not, try again. If we've tried too many times already, abort.

            locationFailsafe = 10;
            do {
                zeroOutGrid(p->interior);
                tryAgain = false;

                if (chooseLocation) {
                    // Pick a random origin location.
                    pos originLoc = { originX, originY };
                    randomMatchingLocation(&originLoc, FLOOR, NOTHING, -1);
                    originX = originLoc.x;
                    originY = originLoc.y;
                }

                if (!distanceMap) {
                    distanceMap = allocGrid();
                }
                fillGrid(distanceMap, 0);
                calculateDistances(distanceMap, originX, originY, T_PATHING_BLOCKER, NULL, true, false);
                qualifyingTileCount = 0; // Keeps track of how many interior cells we've added.
                totalFreq = rand_range(blueprintCatalog[bp].roomSize[0], blueprintCatalog[bp].roomSize[1]); // Keeps track of the goal size.

                fillSequentialList(p->sCols, DCOLS);
                shuffleList(p->sCols, DCOLS);
                fillSequentialList(p->sRows, DROWS);
                shuffleList(p->sRows, DROWS);

                for (int k=0; k<1000 && qualifyingTileCount < totalFreq; k++) {
                    for(int i=0; i<DCOLS && qualifyingTileCount < totalFreq; i++) {
                        for(int j=0; j<DROWS && qualifyingTileCount < totalFreq; j++) {
                            if (distanceMap[p->sCols[i]][p->sRows[j]] == k) {
                                p->interior[p->sCols[i]][p->sRows[j]] = true;
                                qualifyingTileCount++;

                                if (pmap[p->sCols[i]][p->sRows[j]].flags & (HAS_ITEM | HAS_MONSTER | IS_IN_MACHINE)) {
                                    // Abort if we've entered another machine or engulfed another machine's item or monster.
                                    tryAgain = true;
                                    qualifyingTileCount = totalFreq; // This is a hack to drop out of these three for-loops.
                                }
                            }
                        }
                    }
                }

                // Now make sure the interior map satisfies the machine's qualifications.
                if ((blueprintCatalog[bp].flags & BP_TREAT_AS_BLOCKING)
                    && levelIsDisconnectedWithBlockingMap(p->interior, false)) {
                    tryAgain = true;
                } else if ((blueprintCatalog[bp].flags & BP_REQUIRE_BLOCKING)
                           && levelIsDisconnectedWithBlockingMap(p->interior, true) < 100) {
                    tryAgain = true; // BP_REQUIRE_BLOCKING needs some work to make sure the disconnect is interesting.
                }
                // If locationFailsafe runs out, tryAgain will still be true, and we'll try a different machine.
                // If we're not choosing the blueprint, then don't bother with the locationFailsafe; just use the higher-level failsafe.
            } while (chooseBP && tryAgain && --locationFailsafe);
        }

        // If something went wrong, but we haven't been charged with choosing blueprint OR location,
        // then there is nothing to try again, so just fail.
        if (tryAgain && !chooseBP && !chooseLocation) {
            if (distanceMap) {
                freeGrid(distanceMap);
            }
            free(p);
            return false;
        }

        // Now loop if necessary.
    } while (tryAgain);

    // This is the point of no return. Back up the level so it can be restored if we have to abort this machine after this point.
    copyMap(pmap, p->levelBackup);

    // Perform any transformations to the interior indicated by the blueprint flags, including expanding the interior if requested.
    prepareInteriorWithMachineFlags(p->interior, originX, originY, blueprintCatalog[bp].flags, blueprintCatalog[bp].dungeonProfileType);

    // If necessary, label the interior as IS_IN_AREA_MACHINE or IS_IN_ROOM_MACHINE and mark down the number.
    machineNumber = ++rogue.machineNumber; // Reserve this machine number, starting with 1.
    for(int i=0; i<DCOLS; i++) {
        for(int j=0; j<DROWS; j++) {
            if (p->interior[i][j]) {
                pmap[i][j].flags |= ((blueprintCatalog[bp].flags & BP_ROOM) ? IS_IN_ROOM_MACHINE : IS_IN_AREA_MACHINE);
                pmap[i][j].machineNumber = machineNumber;
                // also clear any secret doors, since they screw up distance mapping and aren't fun inside machines
                if (pmap[i][j].layers[DUNGEON] == SECRET_DOOR) {
                    pmap[i][j].layers[DUNGEON] = DOOR;
                }
                // Clear wired tiles in case we stole them from another machine.
                for (int layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                    if (tileCatalog[pmap[i][j].layers[layer]].mechFlags & (TM_IS_WIRED | TM_IS_CIRCUIT_BREAKER)) {
                        pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                    }
                }
            }
        }
    }

//  DEBUG printf("\n\nWorking on blueprint %i, with origin at (%i, %i). Here's the initial interior map:", bp, originX, originY);
//  DEBUG logBuffer(interior);

    // Calculate the distance map (so that features that want to be close to or far from the origin can be placed accordingly)
    // and figure out the 33rd and 67th percentiles for features that want to be near or far from the origin.
    if (!distanceMap) {
        distanceMap = allocGrid();
    }
    fillGrid(distanceMap, 0);
    calculateDistances(distanceMap, originX, originY, T_PATHING_BLOCKER, NULL, true, true);
    qualifyingTileCount = 0;
    for (int i=0; i<100; i++) {
        p->distances[i] = 0;
    }
    for(int i=0; i<DCOLS; i++) {
        for(int j=0; j<DROWS; j++) {
            if (p->interior[i][j]
                && distanceMap[i][j] < 100) {
                p->distances[distanceMap[i][j]]++; // create a histogram of distances -- poor man's sort function
                qualifyingTileCount++;
            }
        }
    }
    int distance25 = (int) (qualifyingTileCount / 4);
    int distance75 = (int) (3 * qualifyingTileCount / 4);
    for (int i=0; i<100; i++) {
        if (distance25 <= p->distances[i]) {
            distance25 = i;
            break;
        } else {
            distance25 -= p->distances[i];
        }
    }
    for (int i=0; i<100; i++) {
        if (distance75 <= p->distances[i]) {
            distance75 = i;
            break;
        } else {
            distance75 -= p->distances[i];
        }
    }
    //DEBUG printf("\nDistances calculated: 33rd percentile of distance is %i, and 67th is %i.", distance25, distance75);

    // Now decide which features will be skipped -- of the features marked MF_ALTERNATIVE, skip all but one, chosen randomly.
    // Then repeat and do the same with respect to MF_ALTERNATIVE_2, to provide up to two independent sets of alternative features per machine.

    for (int i=0; i<blueprintCatalog[bp].featureCount; i++) {
        skipFeature[i] = false;
    }
    for (int j = 0; j <= 1; j++) {
        totalFreq = 0;
        for (int i=0; i<blueprintCatalog[bp].featureCount; i++) {
            if (blueprintCatalog[bp].feature[i].flags & alternativeFlags[j]) {
                skipFeature[i] = true;
                totalFreq++;
            }
        }
        if (totalFreq > 0) {
            int randIndex = rand_range(1, totalFreq);
            for (int i=0; i<blueprintCatalog[bp].featureCount; i++) {
                if (blueprintCatalog[bp].feature[i].flags & alternativeFlags[j]) {
                    if (randIndex == 1) {
                        skipFeature[i] = false; // This is the alternative that gets built. The rest do not.
                        break;
                    } else {
                        randIndex--;
                    }
                }
            }
        }
    }

    // Keep track of all monsters and items that we spawn -- if we abort, we have to go back and delete them all.
    itemCount = monsterCount = 0;

    // Zero out occupied[][], and use it to keep track of the personal space around each feature that gets placed.
    zeroOutGrid(p->occupied);

    // Now tick through the features and build them.
    for (int feat = 0; feat < blueprintCatalog[bp].featureCount; feat++) {

        if (skipFeature[feat]) {
            continue; // Skip the alternative features that were not selected for building.
        }

        feature = &(blueprintCatalog[bp].feature[feat]);

        // Figure out the distance bounds.
        short distanceBound[2] = { 0, 10000 };
        if (feature->flags & MF_NEAR_ORIGIN) {
            distanceBound[1] = distance25;
        }
        if (feature->flags & MF_FAR_FROM_ORIGIN) {
            distanceBound[0] = distance75;
        }

        if (feature->flags & (MF_IN_VIEW_OF_ORIGIN | MF_IN_PASSABLE_VIEW_OF_ORIGIN)) {
            zeroOutGrid(p->viewMap);
            if (feature->flags & MF_IN_PASSABLE_VIEW_OF_ORIGIN) {
                getFOVMask(p->viewMap, originX, originY, max(DCOLS, DROWS) * FP_FACTOR, T_PATHING_BLOCKER, 0, false);
            } else {
                getFOVMask(p->viewMap, originX, originY, max(DCOLS, DROWS) * FP_FACTOR, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION), 0, false);
            }
            p->viewMap[originX][originY] = true;

            if (D_INSPECT_MACHINES) {
                dumpLevelToScreen();
                hiliteCharGrid(p->viewMap, &omniscienceColor, 75);
                temporaryMessage("Showing visibility.", REQUIRE_ACKNOWLEDGMENT);
            }
        }

        do { // If the MF_REPEAT_UNTIL_NO_PROGRESS flag is set, repeat until we fail to build the required number of instances.

            // Make a master map of candidate locations for this feature.
            qualifyingTileCount = 0;
            for(int i=0; i<DCOLS; i++) {
                for(int j=0; j<DROWS; j++) {
                    if (cellIsFeatureCandidate(i, j,
                                               originX, originY,
                                               distanceBound,
                                               p->interior, p->occupied, p->viewMap, distanceMap,
                                               machineNumber, feature->flags, blueprintCatalog[bp].flags)) {
                        qualifyingTileCount++;
                        p->candidates[i][j] = true;
                    } else {
                        p->candidates[i][j] = false;
                    }
                }
            }

            if (D_INSPECT_MACHINES) {
                dumpLevelToScreen();
                hiliteCharGrid(p->occupied, &red, 75);
                hiliteCharGrid(p->candidates, &green, 75);
                hiliteCharGrid(p->interior, &blue, 75);
                temporaryMessage("Indicating: Occupied (red); Candidates (green); Interior (blue).", REQUIRE_ACKNOWLEDGMENT);
            }

            if (feature->flags & MF_EVERYWHERE & ~MF_BUILD_AT_ORIGIN) {
                // Generate everywhere that qualifies -- instead of randomly picking tiles, keep spawning until we run out of eligible tiles.
                generateEverywhere = true;
            } else {
                // build as many instances as required
                generateEverywhere = false;
                instanceCount = rand_range(feature->instanceCountRange[0], feature->instanceCountRange[1]);
            }

            // Cache the personal space constant.
            personalSpace = feature->personalSpace;

            for (instance = 0; (generateEverywhere || instance < instanceCount) && qualifyingTileCount > 0;) {

                // Find a location for the feature.
                int featX;
                int featY;
                if (feature->flags & MF_BUILD_AT_ORIGIN) {
                    // Does the feature want to be at the origin? If so, put it there. (Just an optimization.)
                    featX = originX;
                    featY = originY;
                } else {
                    // Pick our candidate location randomly, and also strike it from
                    // the candidates map so that subsequent instances of this same feature can't choose it.
                    featX = -1;
                    featY = -1;
                    int randIndex = rand_range(1, qualifyingTileCount);
                    for(int i=0; i<DCOLS && featX < 0; i++) {
                        for(int j=0; j<DROWS && featX < 0; j++) {
                            if (p->candidates[i][j]) {
                                if (randIndex == 1) {
                                    // This is the place!
                                    featX = i;
                                    featY = j;
                                    i = DCOLS;  // break out of the loops
                                    j = DROWS;
                                } else {
                                    randIndex--;
                                }
                            }
                        }
                    }
                }
                // Don't waste time trying the same place again whether or not this attempt succeeds.
                p->candidates[featX][featY] = false;
                qualifyingTileCount--;

                DFSucceeded = terrainSucceeded = true;

                // Try to build the DF first, if any, since we don't want it to be disrupted by subsequently placed terrain.
                if (feature->featureDF) {
                    DFSucceeded = spawnDungeonFeature(featX, featY, &dungeonFeatureCatalog[feature->featureDF], false,
                                                      !(feature->flags & MF_PERMIT_BLOCKING));
                }

                // Now try to place the terrain tile, if any.
                if (DFSucceeded && feature->terrain) {
                    // Must we check for blocking?
                    if (!(feature->flags & MF_PERMIT_BLOCKING)
                        && ((tileCatalog[feature->terrain].flags & T_PATHING_BLOCKER) || (feature->flags & MF_TREAT_AS_BLOCKING))) {
                        // Yes, check for blocking.

                        zeroOutGrid(p->blockingMap);
                        p->blockingMap[featX][featY] = true;
                        terrainSucceeded = !levelIsDisconnectedWithBlockingMap(p->blockingMap, false);
                    }
                    if (terrainSucceeded) {
                        pmap[featX][featY].layers[feature->layer] = feature->terrain;
                    }
                }

                // OK, if placement was successful, clear some personal space around the feature so subsequent features can't be generated too close.
                // Personal space of 0 means nothing gets cleared, 1 means that only the tile itself gets cleared, and 2 means the 3x3 grid centered on it.

                if (DFSucceeded && terrainSucceeded) {
                    for (int i = featX - personalSpace + 1;
                         i <= featX + personalSpace - 1;
                         i++) {
                        for (int j = featY - personalSpace + 1;
                             j <= featY + personalSpace - 1;
                             j++) {
                            if (coordinatesAreInMap(i, j)) {
                                if (p->candidates[i][j]) {
                                    brogueAssert(!p->occupied[i][j] || (i == originX && j == originY)); // Candidates[][] should never be true where occupied[][] is true.
                                    p->candidates[i][j] = false;
                                    qualifyingTileCount--;
                                }
                                p->occupied[i][j] = true;
                            }
                        }
                    }
                    instance++; // we've placed an instance
                    //DEBUG printf("\nPlaced instance #%i of feature %i at (%i, %i).", instance, feat, featX, featY);
                }

                if (DFSucceeded && terrainSucceeded) { // Proceed only if the terrain stuff for this instance succeeded.

                    theItem = NULL;

                    // Mark the feature location as part of the machine, in case it is not already inside of it.
                    pmap[featX][featY].flags |= ((blueprintCatalog[bp].flags & BP_ROOM) ? IS_IN_ROOM_MACHINE : IS_IN_AREA_MACHINE);
                    pmap[featX][featY].machineNumber = machineNumber;

                    // Mark the feature location as impregnable if requested.
                    if (feature->flags & MF_IMPREGNABLE) {
                        pmap[featX][featY].flags |= IMPREGNABLE;
                    }

                    // Generate an item as necessary.
                    if ((feature->flags & MF_GENERATE_ITEM)
                        || (adoptiveItem && (feature->flags & MF_ADOPT_ITEM) && (blueprintCatalog[bp].flags & BP_ADOPT_ITEM))) {
                        // Are we adopting an item instead of generating one?
                        if (adoptiveItem && (feature->flags & MF_ADOPT_ITEM) && (blueprintCatalog[bp].flags & BP_ADOPT_ITEM)) {
                            theItem = adoptiveItem;
                            adoptiveItem = NULL; // can be adopted only once
                        } else {
                            // Have to create an item ourselves.
                            theItem = generateItem(feature->itemCategory, feature->itemKind);
                            failsafe = 1000;
                            while ((theItem->flags & ITEM_CURSED)
                                   || ((feature->flags & MF_REQUIRE_GOOD_RUNIC) && (!(theItem->flags & ITEM_RUNIC))) // runic if requested
                                   || ((feature->flags & MF_NO_THROWING_WEAPONS) && theItem->category == WEAPON && theItem->quantity > 1) // no throwing weapons if prohibited
                                   || ((feature->flags & MF_REQUIRE_HEAVY_WEAPON) && (!itemIsHeavyWeapon(theItem) || !itemIsPositivelyEnchanted(theItem))) // must be a positively enchanted heavy weapon
                                   || itemIsADuplicate(theItem, p->spawnedItems, itemCount)) { // don't want to duplicates of rings, staffs, etc.
                                deleteItem(theItem);
                                theItem = generateItem(feature->itemCategory, feature->itemKind);
                                if (failsafe <= 0) {
                                    break;
                                }
                                failsafe--;
                            }
                            p->spawnedItems[itemCount] = theItem; // Keep a list of generated items so that we can delete them all if construction fails.
                            itemCount++;
                        }
                        theItem->flags |= feature->itemFlags;

                        addLocationToKey(theItem, featX, featY, (feature->flags & MF_KEY_DISPOSABLE) ? true : false);
                        theItem->originDepth = rogue.depthLevel;
                        if (feature->flags & MF_SKELETON_KEY) {
                            addMachineNumberToKey(theItem, machineNumber, (feature->flags & MF_KEY_DISPOSABLE) ? true : false);
                        }
                        if (!(feature->flags & MF_OUTSOURCE_ITEM_TO_MACHINE)
                            && !(feature->flags & MF_MONSTER_TAKE_ITEM)) {
                            // Place the item at the feature location.
                            placeItemAt(theItem, (pos){ featX, featY });
                        }
                    }

                    if (feature->flags & (MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_VESTIBULE)) {
                        // Put this item up for adoption, or generate a door guard machine.
                        // Try to create a sub-machine that qualifies.
                        // If we fail 10 times, abort the entire machine (including any sub-machines already built).
                        // Also, if we build a sub-machine, and it succeeds, but this (its parent machine) fails,
                        // we pass the monsters and items that it spawned back to the parent,
                        // so that if the parent fails, they can all be freed.
                        int i;
                        for (i=10; i > 0; i--) {
                            // First make sure our adopted item, if any, is not on the floor or in the pack already.
                            // Otherwise, a previous attempt to place it may have put it on the floor in a different
                            // machine, only to have that machine fail and be deleted, leaving the item remaining on
                            // the floor where placed.
                            if ((feature->flags & MF_OUTSOURCE_ITEM_TO_MACHINE) && theItem) {
                                removeItemFromChain(theItem, floorItems);
                                removeItemFromChain(theItem, packItems);
                                theItem->nextItem = NULL;
                                success = buildAMachine(-1, -1, -1, BP_ADOPT_ITEM, theItem, p->spawnedItemsSub, p->spawnedMonstersSub);
                            } else if (feature->flags & MF_BUILD_VESTIBULE) {
                                success = buildAMachine(-1, featX, featY, BP_VESTIBULE, NULL, p->spawnedItemsSub, p->spawnedMonstersSub);
                            }

                            // Now put the item up for adoption.
                            if (success) {
                                // Success! Now we have to add that machine's items and monsters to our own list, so they
                                // all get deleted if this machine or its parent fails.
                                for (int j=0; j<MACHINES_BUFFER_LENGTH && p->spawnedItemsSub[j]; j++) {
                                    p->spawnedItems[itemCount] = p->spawnedItemsSub[j];
                                    itemCount++;
                                    p->spawnedItemsSub[j] = NULL;
                                }
                                for (int j=0; j<MACHINES_BUFFER_LENGTH && p->spawnedMonstersSub[j]; j++) {
                                    p->spawnedMonsters[monsterCount] = p->spawnedMonstersSub[j];
                                    monsterCount++;
                                    p->spawnedMonstersSub[j] = NULL;
                                }
                                break;
                            }
                        }

                        if (!i) {
                            if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to place blueprint %i:%s because it requires an adoptive machine and we couldn't place one.", rogue.depthLevel, bp, blueprintCatalog[bp].name);
                            // failure! abort!
                            copyMap(p->levelBackup, pmap);
                            abortItemsAndMonsters(p->spawnedItems, p->spawnedMonsters);
                            freeGrid(distanceMap);
                            free(p);
                            return false;
                        }
                        theItem = NULL;
                    }

                    // Generate a horde as necessary.
                    if ((feature->flags & MF_GENERATE_HORDE)
                        || feature->monsterID) {

                        if (feature->flags & MF_GENERATE_HORDE) {
                            monst = spawnHorde(0,
                                               (pos){ featX, featY },
                                               ((HORDE_IS_SUMMONED | HORDE_LEADER_CAPTIVE) & ~(feature->hordeFlags)),
                                               feature->hordeFlags);
                            if (monst) {
                                monst->bookkeepingFlags |= MB_JUST_SUMMONED;
                            }
                        }

                        if (feature->monsterID) {
                            monst = monsterAtLoc((pos){ featX, featY });
                            if (monst) {
                                killCreature(monst, true); // If there's already a monster here, quietly bury the body.
                            }
                            monst = generateMonster(feature->monsterID, true, true);
                            if (monst) {
                                monst->loc = (pos){ .x = featX, .y = featY };
                                pmapAt(monst->loc)->flags |= HAS_MONSTER;
                                monst->bookkeepingFlags |= MB_JUST_SUMMONED;
                            }
                        }

                        if (monst) {
                            if (!leader) {
                                leader = monst;
                            }

                            // Give our item to the monster leader if appropriate.
                            // Actually just remember that we have to give it to this monster; the actual
                            // hand-off happens after we're sure that the machine will succeed.
                            if (theItem && (feature->flags & MF_MONSTER_TAKE_ITEM)) {
                                torchBearer = monst;
                                torch = theItem;
                            }
                        }

                        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                            creature *monst = nextCreature(&it);
                            if (monst->bookkeepingFlags & MB_JUST_SUMMONED) {

                                // All monsters spawned by a machine are tribemates.
                                // Assign leader/follower roles if they are not yet assigned.
                                if (!(monst->bookkeepingFlags & (MB_LEADER | MB_FOLLOWER))) {
                                    if (leader && leader != monst) {
                                        monst->leader = leader;
                                        monst->bookkeepingFlags &= ~MB_LEADER;
                                        monst->bookkeepingFlags |= MB_FOLLOWER;
                                        leader->bookkeepingFlags |= MB_LEADER;
                                    } else {
                                        leader = monst;
                                    }
                                }

                                monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
                                p->spawnedMonsters[monsterCount] = monst;
                                monsterCount++;
                                if (feature->flags & MF_MONSTER_SLEEPING) {
                                    monst->creatureState = MONSTER_SLEEPING;
                                }
                                if (feature->flags & MF_MONSTER_FLEEING) {
                                    monst->creatureState = MONSTER_FLEEING;
                                    monst->creatureMode = MODE_PERM_FLEEING;
                                }
                                if (feature->flags & MF_MONSTERS_DORMANT) {
                                    toggleMonsterDormancy(monst);
                                    if (!(feature->flags & MF_MONSTER_SLEEPING) && monst->creatureState != MONSTER_ALLY) {
                                        monst->creatureState = MONSTER_TRACKING_SCENT;
                                    }
                                }
                                monst->machineHome = machineNumber; // Monster remembers the machine that spawned it.
                            }
                        }
                    }
                }
                theItem = NULL;

                // Finished with this instance!
            }
        } while ((feature->flags & MF_REPEAT_UNTIL_NO_PROGRESS) && instance >= feature->minimumInstanceCount);

        //DEBUG printf("\nFinished feature %i. Here's the candidates map:", feat);
        //DEBUG logBuffer(candidates);

        if (instance < feature->minimumInstanceCount && !(feature->flags & MF_REPEAT_UNTIL_NO_PROGRESS)) {
            // failure! abort!

            if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to place blueprint %i:%s because of feature %i; needed %i instances but got only %i.",
                         rogue.depthLevel, bp, blueprintCatalog[bp].name, feat, feature->minimumInstanceCount, instance);

            // Restore the map to how it was before we touched it.
            copyMap(p->levelBackup, pmap);
            abortItemsAndMonsters(p->spawnedItems, p->spawnedMonsters);
            freeGrid(distanceMap);
            free(p);
            return false;
        }
    }

    // Clear out the interior flag for all non-wired cells, if requested.
    if (blueprintCatalog[bp].flags & BP_NO_INTERIOR_FLAG) {
        for(int i=0; i<DCOLS; i++) {
            for(int j=0; j<DROWS; j++) {
                if (pmap[i][j].machineNumber == machineNumber
                    && !cellHasTMFlag((pos){ i, j }, (TM_IS_WIRED | TM_IS_CIRCUIT_BREAKER))) {

                    pmap[i][j].flags &= ~IS_IN_MACHINE;
                    pmap[i][j].machineNumber = 0;
                }
            }
        }
    }

    if (torchBearer && torch) {
        if (torchBearer->carriedItem) {
            deleteItem(torchBearer->carriedItem);
        }
        removeItemFromChain(torch, floorItems);
        torchBearer->carriedItem = torch;
    }

    freeGrid(distanceMap);
    if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Built a machine from blueprint %i:%s with an origin at (%i, %i).", rogue.depthLevel, bp, blueprintCatalog[bp].name, originX, originY);

    //Pass created items and monsters to parent where they will be deleted on failure to place parent machine
    if (parentSpawnedItems) {
        for (int i=0; i<itemCount; i++) {
            parentSpawnedItems[i] = p->spawnedItems[i];
        }
    }
    if (parentSpawnedMonsters) {
        for (int i=0; i<monsterCount; i++) {
            parentSpawnedMonsters[i] = p->spawnedMonsters[i];
        }
    }

    // Custom tile layout for fixtures that need precise patterns.
    pos effectiveOrigin = (pos){ originX, originY };
    if (bp == MT_FIXTURE_DRAINAGE_CHANNEL) {
        if (!applyDrainageLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_MOSSY_ALCOVE) {
        if (!applyMossyAlcoveLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_COBWEB_CORNER) {
        if (!applyCobwebCornerLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_CRUMBLED_WALL) {
        if (!applyCrumbledWallLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_GARDEN_PATCH) {
        if (!applyGardenLayout(originX, originY, p->interior, &effectiveOrigin)) {
            // Not enough space for the garden pattern; abort and restore.
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_MUSHROOM_CIRCLE) {
        if (!applyMushroomCircleLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_BIRD_NEST) {
        if (!applyBirdNestLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_VINE_TRELLIS) {
        if (!applyVineTrellisLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_PUDDLE) {
        if (!applyPuddleLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_FORGE) {
        if (!applyForgeLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_ALTAR_NOOK) {
        if (!applyAltarNookLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_CRYSTAL_OUTCROP) {
        if (!applyCrystalOutcropLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_ABANDONED_CAMP) {
        if (!applyAbandonedCampLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_WEAPON_RACK) {
        if (!applyWeaponRackLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_LICHEN_GARDEN) {
        if (!applyLichenGardenLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_TOPPLED_BOOKCASE) {
        if (!applyToppledBookcaseLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_BONE_THRONE) {
        if (!applyBoneThroneLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_BLOOD_POOL) {
        if (!applyBloodPoolLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_OBSIDIAN_FORMATION) {
        if (!applyObsidianFormationLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }
    if (bp == MT_FIXTURE_EMBER_PIT) {
        if (!applyEmberPitLayout(originX, originY, p->interior, &effectiveOrigin)) {
            copyMap(p->levelBackup, pmap);
            rogue.machineNumber--;
            free(p);
            return false;
        }
    }

    // Universal connectivity check: ensure no machine placement disconnected the level.
    // Blocking terrain (statues, crystal walls, lava) placed by layout functions or
    // standard features can create impassable chokepoints. Abort the machine if
    // passable areas are split. This is a safety net that covers both custom fixture
    // layouts and standard feature placement.
    if (!levelIsFullyConnected()) {
        if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Aborting blueprint %i:%s because custom layout disconnected the level.",
                     rogue.depthLevel, bp, blueprintCatalog[bp].name);
        copyMap(p->levelBackup, pmap);
        rogue.machineNumber--;
        free(p);
        return false;
    }

    // Record this machine placement for external tools and queries.
    if (rogue.placedMachineCount < MAX_PLACED_MACHINES) {
        placedMachineInfo *info = &rogue.placedMachines[rogue.placedMachineCount++];
        info->blueprintIndex = bp;
        info->machineNumber = machineNumber;
        info->origin = effectiveOrigin;
    }

    free(p);
    return true;
}

// add machines to the dungeon.
static void addMachines(void) {
    short machineCount, failsafe;
    short randomMachineFactor;

    analyzeMap(true);

    // For bullet brogue, add a guaranteed weapon vault on l1
    if (gameVariant == VARIANT_BULLET_BROGUE && rogue.depthLevel == 1) {
        for (failsafe = 50; failsafe; failsafe--) {
            if (buildAMachine(MT_REWARD_HEAVY_OR_RUNIC_WEAPON, -1, -1, 0, NULL, NULL, NULL)) {
                break;
            }
        }
    }

    // Add the amulet holder if it's depth 26:
    if (rogue.depthLevel == gameConst->amuletLevel) {
        for (failsafe = 50; failsafe; failsafe--) {
            if (buildAMachine(MT_AMULET_AREA, -1, -1, 0, NULL, NULL, NULL)) {
                break;
            }
        }
    }

    // Add reward rooms, if any:
    machineCount = 0;
    while (rogue.depthLevel <= gameConst->deepestLevelForMachines
        && (rogue.rewardRoomsGenerated + machineCount) * gameConst->machinesPerLevelSuppressionMultiplier + gameConst->machinesPerLevelSuppressionOffset < rogue.depthLevel * gameConst->machinesPerLevelIncreaseFactor) {
        // try to build at least one every four levels on average
        machineCount++;
    }
    randomMachineFactor = (rogue.depthLevel <= gameConst->maxLevelForBonusMachines && (rogue.rewardRoomsGenerated + machineCount) == 0 ? 40 : 15);
    while (rand_percent(max(randomMachineFactor, 15 * gameConst->machinesPerLevelIncreaseFactor)) && machineCount < 100) {
        randomMachineFactor = 15;
        machineCount++;
    }

    for (failsafe = 50; machineCount && failsafe; failsafe--) {
        if (buildAMachine(-1, -1, -1, BP_REWARD, NULL, NULL, NULL)) {
            machineCount--;
            rogue.rewardRoomsGenerated++;
        }
    }
}

// Add terrain, DFs and flavor machines. Includes traps, torches, funguses, flavor machines, etc.
// If buildAreaMachines is true, build ONLY the autogenerators that include machines.
// If false, build all EXCEPT the autogenerators that include machines.
static void runAutogenerators(boolean buildAreaMachines) {
    short AG, count, i;
    const autoGenerator *gen;
    char grid[DCOLS][DROWS];

    // Cycle through the autoGenerators.
    for (AG=1; AG<gameConst->numberAutogenerators; AG++) {

        // Shortcut:
        gen = &(autoGeneratorCatalog[AG]);

        if (gen->machine > 0 == buildAreaMachines) {

            // Enforce depth constraints.
            if (rogue.depthLevel < gen->minDepth || rogue.depthLevel > gen->maxDepth) {
                continue;
            }

            // Decide how many of this AG to build.
            count = min((gen->minNumberIntercept + rogue.depthLevel * gen->minNumberSlope) / 100, gen->maxNumber);
            while (rand_percent(gen->frequency) && count < gen->maxNumber) {
                count++;
            }

            // Build that many instances.
            for (i = 0; i < count; i++) {

                // Find a location for DFs and terrain generations.
                //if (randomMatchingLocation(&x, &y, gen->requiredDungeonFoundationType, NOTHING, -1)) {
                //if (randomMatchingLocation(&x, &y, -1, -1, gen->requiredDungeonFoundationType)) {
                pos foundationLoc;
                if (randomMatchingLocation(&foundationLoc, gen->requiredDungeonFoundationType, gen->requiredLiquidFoundationType, -1)) {
                    // Spawn the DF.
                    if (gen->DFType) {
                        spawnDungeonFeature(foundationLoc.x, foundationLoc.y, &(dungeonFeatureCatalog[gen->DFType]), false, true);

                        if (D_INSPECT_LEVELGEN) {
                            dumpLevelToScreen();
                            hiliteCell(foundationLoc.x, foundationLoc.y, &yellow, 50, true);
                            temporaryMessage("Dungeon feature added.", REQUIRE_ACKNOWLEDGMENT);
                        }
                    }

                    // Spawn the terrain if it's got the priority to spawn there and won't disrupt connectivity.
                    if (gen->terrain
                        && tileCatalog[pmapAt(foundationLoc)->layers[gen->layer]].drawPriority >= tileCatalog[gen->terrain].drawPriority) {

                        // Check connectivity.
                        zeroOutGrid(grid);
                        grid[foundationLoc.x][foundationLoc.y] = true;
                        if (!(tileCatalog[gen->terrain].flags & T_PATHING_BLOCKER)
                            || !levelIsDisconnectedWithBlockingMap(grid, false)) {

                            // Build!
                            pmapAt(foundationLoc)->layers[gen->layer] = gen->terrain;

                            if (D_INSPECT_LEVELGEN) {
                                dumpLevelToScreen();
                                hiliteCell(foundationLoc.x, foundationLoc.y, &yellow, 50, true);
                                temporaryMessage("Terrain added.", REQUIRE_ACKNOWLEDGMENT);
                            }
                        }
                    }
                }

                // Attempt to build the machine if requested.
                // Machines will find their own locations, so it will not be at the same place as terrain and DF.
                if (gen->machine > 0) {
                    buildAMachine(gen->machine, -1, -1, 0, NULL, NULL, NULL);
                }
            }
        }
    }
}

// Knock down the boundaries between similar lakes where possible.
static void cleanUpLakeBoundaries(void) {
    short i, j, x, y, failsafe, layer;
    boolean reverse, madeChange;
    unsigned long subjectFlags;

    reverse = true;

    failsafe = 100;
    do {
        madeChange = false;
        reverse = !reverse;
        failsafe--;

        for (i = (reverse ? DCOLS - 2 : 1);
             (reverse ? i > 0 : i < DCOLS - 1);
             (reverse ? i-- : i++)) {

            for (j = (reverse ? DROWS - 2 : 1);
                 (reverse ? j > 0 : j < DROWS - 1);
                 (reverse ? j-- : j++)) {

                //assert(i >= 1 && i <= DCOLS - 2 && j >= 1 && j <= DROWS - 2);

                //if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                if (cellHasTerrainFlag((pos){ i, j }, T_LAKE_PATHING_BLOCKER | T_OBSTRUCTS_PASSABILITY)
                    && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)
                    && !(pmap[i][j].flags & IMPREGNABLE)) {

                    subjectFlags = terrainFlags((pos){ i, j }) & (T_LAKE_PATHING_BLOCKER | T_OBSTRUCTS_PASSABILITY);

                    x = y = 0;
                    if ((terrainFlags((pos){ i - 1, j }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)
                        && !cellHasTMFlag((pos){ i - 1, j }, TM_IS_SECRET)
                        && !cellHasTMFlag((pos){ i + 1, j }, TM_IS_SECRET)
                        && (terrainFlags((pos){ i - 1, j }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags) == (terrainFlags((pos){ i + 1, j }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)) {
                        x = i + 1;
                        y = j;
                    } else if ((terrainFlags((pos){ i, j - 1 }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)
                               && !cellHasTMFlag((pos){ i, j - 1 }, TM_IS_SECRET)
                               && !cellHasTMFlag((pos){ i, j + 1 }, TM_IS_SECRET)
                               && (terrainFlags((pos){ i, j - 1 }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags) == (terrainFlags((pos){ i, j + 1 }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)) {
                        x = i;
                        y = j + 1;
                    }
                    if (x) {
                        madeChange = true;
                        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                            pmap[i][j].layers[layer] = pmap[x][y].layers[layer];
                        }
                        //pmap[i][j].layers[DUNGEON] = CRYSTAL_WALL;
                    }
                }
            }
        }
    } while (madeChange && failsafe > 0);
}

static void removeDiagonalOpenings(void) {
    short i, j, k, x1, y1, x2, layer;
    boolean diagonalCornerRemoved;

    do {
        diagonalCornerRemoved = false;
        for (i=0; i<DCOLS-1; i++) {
            for (j=0; j<DROWS-1; j++) {
                for (k=0; k<=1; k++) {
                    if (!(tileCatalog[pmap[i + k][j].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)
                        && (tileCatalog[pmap[i + (1-k)][j].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)
                        && (tileCatalog[pmap[i + (1-k)][j].layers[DUNGEON]].flags & T_OBSTRUCTS_DIAGONAL_MOVEMENT)
                        && (tileCatalog[pmap[i + k][j+1].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)
                        && (tileCatalog[pmap[i + k][j+1].layers[DUNGEON]].flags & T_OBSTRUCTS_DIAGONAL_MOVEMENT)
                        && !(tileCatalog[pmap[i + (1-k)][j+1].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)) {

                        if (rand_percent(50)) {
                            x1 = i + (1-k);
                            x2 = i + k;
                            y1 = j;
                        } else {
                            x1 = i + k;
                            x2 = i + (1-k);
                            y1 = j + 1;
                        }
                        if (!(pmap[x1][y1].flags & HAS_MONSTER) && pmap[x1][y1].machineNumber == 0) {
                            diagonalCornerRemoved = true;
                            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                                pmap[x1][y1].layers[layer] = pmap[x2][y1].layers[layer];
                            }
                        }
                    }
                }
            }
        }
    } while (diagonalCornerRemoved == true);
}

static void insertRoomAt(short **dungeonMap, short **roomMap, const short roomToDungeonX, const short roomToDungeonY, const short xRoom, const short yRoom) {
    short newX, newY;
    enum directions dir;

    brogueAssert(coordinatesAreInMap(xRoom + roomToDungeonX, yRoom + roomToDungeonY));

    dungeonMap[xRoom + roomToDungeonX][yRoom + roomToDungeonY] = 1;
    for (dir = 0; dir < 4; dir++) {
        newX = xRoom + nbDirs[dir][0];
        newY = yRoom + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && roomMap[newX][newY]
            && coordinatesAreInMap(newX + roomToDungeonX, newY + roomToDungeonY)
            && dungeonMap[newX + roomToDungeonX][newY + roomToDungeonY] == 0) {

            insertRoomAt(dungeonMap, roomMap, roomToDungeonX, roomToDungeonY, newX, newY);
        }
    }
}

static void designCavern(short **grid, short minWidth, short maxWidth, short minHeight, short maxHeight) {
    short destX, destY;
    short caveX, caveY, caveWidth, caveHeight;
    short fillX = 0, fillY = 0;
    boolean foundFillPoint = false;
    short **blobGrid;
    blobGrid = allocGrid();

    fillGrid(grid, 0);
    createBlobOnGrid(blobGrid,
                     &caveX, &caveY, &caveWidth, &caveHeight,
                     5, minWidth, minHeight, maxWidth, maxHeight, 55, "ffffffttt", "ffffttttt");

//    colorOverDungeon(&darkGray);
//    hiliteGrid(blobGrid, &tanColor, 80);
//    temporaryMessage("Here's the cave:", REQUIRE_ACKNOWLEDGMENT);

    // Position the new cave in the middle of the grid...
    destX = (DCOLS - caveWidth) / 2;
    destY = (DROWS - caveHeight) / 2;
    // ...pick a floodfill insertion point...
    for (fillX = 0; fillX < DCOLS && !foundFillPoint; fillX++) {
        for (fillY = 0; fillY < DROWS && !foundFillPoint; fillY++) {
            if (blobGrid[fillX][fillY]) {
                foundFillPoint = true;
            }
        }
    }
    // ...and copy it to the master grid.
    insertRoomAt(grid, blobGrid, destX - caveX, destY - caveY, fillX, fillY);
    freeGrid(blobGrid);
}

// This is a special room that appears at the entrance to the dungeon on depth 1.
static void designEntranceRoom(short **grid) {
    short roomWidth, roomHeight, roomWidth2, roomHeight2, roomX, roomY, roomX2, roomY2;

    fillGrid(grid, 0);

    roomWidth = 8;
    roomHeight = 10;
    roomWidth2 = 20;
    roomHeight2 = 5;
    roomX = DCOLS/2 - roomWidth/2 - 1;
    roomY = DROWS - roomHeight - 2;
    roomX2 = DCOLS/2 - roomWidth2/2 - 1;
    roomY2 = DROWS - roomHeight2 - 2;

    drawRectangleOnGrid(grid, roomX, roomY, roomWidth, roomHeight, 1);
    drawRectangleOnGrid(grid, roomX2, roomY2, roomWidth2, roomHeight2, 1);
}

static void designCrossRoom(short **grid) {
    short roomWidth, roomHeight, roomWidth2, roomHeight2, roomX, roomY, roomX2, roomY2;

    fillGrid(grid, 0);

    roomWidth = rand_range(3, 12);
    roomX = rand_range(max(0, DCOLS/2 - (roomWidth - 1)), min(DCOLS, DCOLS/2));
    roomWidth2 = rand_range(4, 20);
    roomX2 = (roomX + (roomWidth / 2) + rand_range(0, 2) + rand_range(0, 2) - 3) - (roomWidth2 / 2);

    roomHeight = rand_range(3, 7);
    roomY = (DROWS/2 - roomHeight);

    roomHeight2 = rand_range(2, 5);
    roomY2 = (DROWS/2 - roomHeight2 - (rand_range(0, 2) + rand_range(0, 1)));

    drawRectangleOnGrid(grid, roomX - 5, roomY + 5, roomWidth, roomHeight, 1);
    drawRectangleOnGrid(grid, roomX2 - 5, roomY2 + 5, roomWidth2, roomHeight2, 1);
}

static void designSymmetricalCrossRoom(short **grid) {
    short majorWidth, majorHeight, minorWidth, minorHeight;

    fillGrid(grid, 0);

    majorWidth = rand_range(4, 8);
    majorHeight = rand_range(4, 5);

    minorWidth = rand_range(3, 4);
    if (majorHeight % 2 == 0) {
        minorWidth -= 1;
    }
    minorHeight = 3;//rand_range(2, 3);
    if (majorWidth % 2 == 0) {
        minorHeight -= 1;
    }

    drawRectangleOnGrid(grid, (DCOLS - majorWidth)/2, (DROWS - minorHeight)/2, majorWidth, minorHeight, 1);
    drawRectangleOnGrid(grid, (DCOLS - minorWidth)/2, (DROWS - majorHeight)/2, minorWidth, majorHeight, 1);
}

static void designSmallRoom(short **grid) {
    short width, height;

    fillGrid(grid, 0);
    width = rand_range(3, 6);
    height = rand_range(2, 4);
    drawRectangleOnGrid(grid, (DCOLS - width) / 2, (DROWS - height) / 2, width, height, 1);
}

static void designCircularRoom(short **grid) {
    short radius;

    if (rand_percent(5)) {
        radius = rand_range(4, 10);
    } else {
        radius = rand_range(2, 4);
    }

    fillGrid(grid, 0);
    drawCircleOnGrid(grid, DCOLS/2, DROWS/2, radius, 1);

    if (radius > 6
        && rand_percent(50)) {
        drawCircleOnGrid(grid, DCOLS/2, DROWS/2, rand_range(3, radius - 3), 0);
    }
}

static void designChunkyRoom(short **grid) {
    short i, x, y;
    short minX, maxX, minY, maxY;
    short chunkCount = rand_range(2, 8);

    fillGrid(grid, 0);
    drawCircleOnGrid(grid, DCOLS/2, DROWS/2, 2, 1);
    minX = DCOLS/2 - 3;
    maxX = DCOLS/2 + 3;
    minY = DROWS/2 - 3;
    maxY = DROWS/2 + 3;

    for (i=0; i<chunkCount;) {
        x = rand_range(minX, maxX);
        y = rand_range(minY, maxY);
        if (grid[x][y]) {
//            colorOverDungeon(&darkGray);
//            hiliteGrid(grid, &white, 100);

            drawCircleOnGrid(grid, x, y, 2, 1);
            i++;
            minX = max(1, min(x - 3, minX));
            maxX = min(DCOLS - 2, max(x + 3, maxX));
            minY = max(1, min(y - 3, minY));
            maxY = min(DROWS - 2, max(y + 3, maxY));

//            hiliteGrid(grid, &green, 50);
//            temporaryMessage("Added a chunk:", REQUIRE_ACKNOWLEDGMENT);
        }
    }
}

// ---- New room designs (types 8-17) ----

// 8. L-shaped room: two rectangles meeting at a corner
static void designLShapedRoom(short **grid) {
    short armWidth1, armLength1, armWidth2, armLength2;
    short cx, cy;
    short orientation; // 0=TL, 1=TR, 2=BL, 3=BR (which corner the L bends at)

    fillGrid(grid, 0);

    armWidth1 = rand_range(3, 6);
    armLength1 = rand_range(5, 10);
    armWidth2 = rand_range(3, 6);
    armLength2 = rand_range(5, 10);
    orientation = rand_range(0, 3);

    cx = DCOLS / 2;
    cy = DROWS / 2;

    switch (orientation) {
        case 0: // Vertical arm goes up, horizontal arm goes right
            drawRectangleOnGrid(grid, cx - armWidth1 / 2, cy - armLength1 + armWidth2, armWidth1, armLength1, 1);
            drawRectangleOnGrid(grid, cx - armWidth1 / 2, cy, armLength2, armWidth2, 1);
            break;
        case 1: // Vertical arm goes up, horizontal arm goes left
            drawRectangleOnGrid(grid, cx - armWidth1 / 2, cy - armLength1 + armWidth2, armWidth1, armLength1, 1);
            drawRectangleOnGrid(grid, cx - armWidth1 / 2 - armLength2 + armWidth1, cy, armLength2, armWidth2, 1);
            break;
        case 2: // Vertical arm goes down, horizontal arm goes right
            drawRectangleOnGrid(grid, cx - armWidth1 / 2, cy, armWidth1, armLength1, 1);
            drawRectangleOnGrid(grid, cx - armWidth1 / 2, cy, armLength2, armWidth2, 1);
            break;
        case 3: // Vertical arm goes down, horizontal arm goes left
            drawRectangleOnGrid(grid, cx - armWidth1 / 2, cy, armWidth1, armLength1, 1);
            drawRectangleOnGrid(grid, cx - armWidth1 / 2 - armLength2 + armWidth1, cy, armLength2, armWidth2, 1);
            break;
    }
}

// 9. Pillared hall: rectangle with interior columns
static void designPillaredHall(short **grid) {
    short width, height, x, y, px, py;
    short pillarMargin;

    fillGrid(grid, 0);

    // Prefer odd dimensions for symmetry
    width = rand_range(7, 13);
    if (width % 2 == 0) width++;
    if (width > 13) width = 13;
    height = rand_range(5, 9);
    if (height % 2 == 0) height++;
    if (height > 9) height = 9;

    x = (DCOLS - width) / 2;
    y = (DROWS - height) / 2;
    drawRectangleOnGrid(grid, x, y, width, height, 1);

    // Place pillars inside
    pillarMargin = rand_range(1, 2);
    for (px = x + pillarMargin; px < x + width - pillarMargin; px += 2) {
        for (py = y + pillarMargin; py < y + height - pillarMargin; py += 2) {
            grid[px][py] = 0;
        }
    }
}

// 10. Nested/donut room: large rectangle with hollow center, one gap
static void designNestedRoom(short **grid) {
    short outerW, outerH, innerW, innerH;
    short ox, oy, ix, iy;
    short gapSide, gapPos;

    fillGrid(grid, 0);

    outerW = rand_range(7, 11);
    outerH = rand_range(5, 9);
    innerW = rand_range(3, min(5, outerW - 4));
    innerH = rand_range(1, min(3, outerH - 4));

    ox = (DCOLS - outerW) / 2;
    oy = (DROWS - outerH) / 2;
    drawRectangleOnGrid(grid, ox, oy, outerW, outerH, 1);

    // Hollow out the center
    ix = ox + (outerW - innerW) / 2;
    iy = oy + (outerH - innerH) / 2;
    drawRectangleOnGrid(grid, ix, iy, innerW, innerH, 0);

    // Add a 1-cell gap on one random side of the inner wall
    gapSide = rand_range(0, 3);
    switch (gapSide) {
        case 0: // top
            gapPos = ix + rand_range(0, innerW - 1);
            if (iy > 0) grid[gapPos][iy - 1] = 1;
            break;
        case 1: // bottom
            gapPos = ix + rand_range(0, innerW - 1);
            if (iy + innerH < DROWS) grid[gapPos][iy + innerH] = 1;
            break;
        case 2: // left
            gapPos = iy + rand_range(0, innerH - 1);
            if (ix > 0) grid[ix - 1][gapPos] = 1;
            break;
        case 3: // right
            gapPos = iy + rand_range(0, innerH - 1);
            if (ix + innerW < DCOLS) grid[ix + innerW][gapPos] = 1;
            break;
    }
}

// 11. Trefoil/clover room: three overlapping circles in a triangle
static void designTrefoilRoom(short **grid) {
    short radius, cx, cy;
    short offset;

    fillGrid(grid, 0);

    radius = rand_range(2, 3);
    cx = DCOLS / 2;
    cy = DROWS / 2;
    offset = radius;

    // Three circles arranged in a triangle
    drawCircleOnGrid(grid, cx, cy - offset, radius, 1);        // top
    drawCircleOnGrid(grid, cx - offset, cy + offset / 2, radius, 1); // bottom-left
    drawCircleOnGrid(grid, cx + offset, cy + offset / 2, radius, 1); // bottom-right
}

// 12. Cathedral room: two overlapping pillared rectangles in a cross
// 2/3 have pillar rows only near the top and bottom walls (open nave).
// 1/3 have pillar rows throughout (dense columns).
static void designCathedralRoom(short **grid) {
    short mainW, mainH, crossW, crossH;
    short mx, my, cx2, cy2;
    short pillarMargin, px, py;
    boolean fullPillars;
    short minY, maxY, topPillarLimit, botPillarLimit;

    fillGrid(grid, 0);

    mainW = rand_range(9, 15);
    mainH = rand_range(5, 7);
    crossW = rand_range(5, 7);
    crossH = rand_range(7, 11);

    mx = (DCOLS - mainW) / 2;
    my = (DROWS - mainH) / 2;
    cx2 = (DCOLS - crossW) / 2;
    cy2 = (DROWS - crossH) / 2;

    drawRectangleOnGrid(grid, mx, my, mainW, mainH, 1);
    drawRectangleOnGrid(grid, cx2, cy2, crossW, crossH, 1);

    // Place pillars in the combined floor area
    pillarMargin = rand_range(1, 2);
    fullPillars = rand_percent(33);

    minY = min(my, cy2) + pillarMargin;
    maxY = max(my + mainH, cy2 + crossH) - pillarMargin;

    for (px = min(mx, cx2) + pillarMargin; px < max(mx + mainW, cx2 + crossW) - pillarMargin; px += 2) {
        for (py = minY; py < maxY; py += 2) {
            if (grid[px][py]) {
                if (fullPillars) {
                    grid[px][py] = 0;
                } else {
                    // Side pillars only: place in the top and bottom pillar rows
                    topPillarLimit = minY + 1;  // only the first pillar row
                    botPillarLimit = maxY - 2;  // only the last pillar row
                    if (py <= topPillarLimit || py >= botPillarLimit) {
                        grid[px][py] = 0;
                    }
                }
            }
        }
    }
}

// 13. Alcove room: rectangle with protruding niches along long walls
static void designAlcoveRoom(short **grid) {
    short width, height, x, y, i;

    fillGrid(grid, 0);

    width = rand_range(7, 13);
    if (width % 2 == 0) width++; // Odd for symmetry
    if (width > 13) width = 13;
    height = rand_range(4, 7);

    x = (DCOLS - width) / 2;
    y = (DROWS - height) / 2;

    // Draw main rectangle
    drawRectangleOnGrid(grid, x, y, width, height, 1);

    // Add niches (1-cell indentations into the wall) along top and bottom
    // Start 2 cells from each side, then every 2 cells
    for (i = x + 2; i < x + width - 2; i += 2) {
        // Top niche (one cell above the room)
        if (y - 1 >= 0) {
            grid[i][y - 1] = 1;
        }
        // Bottom niche (one cell below the room)
        if (y + height < DROWS) {
            grid[i][y + height] = 1;
        }
    }
}

// 14. Z-shaped room: two rectangles connected by a diagonal offset corridor
static void designZShapedRoom(short **grid) {
    short armW, armH, connectorW;
    short x1, y1, x2, y2;
    short cx, cy, i;
    boolean flipH;

    fillGrid(grid, 0);

    armW = rand_range(4, 7);
    armH = rand_range(3, 5);
    connectorW = rand_range(2, 3);

    cx = DCOLS / 2;
    cy = DROWS / 2;
    flipH = rand_percent(50);

    if (flipH) {
        // Top-right to bottom-left
        x1 = cx;
        y1 = cy - armH - 1;
        x2 = cx - armW;
        y2 = cy + 1;
    } else {
        // Top-left to bottom-right
        x1 = cx - armW;
        y1 = cy - armH - 1;
        x2 = cx;
        y2 = cy + 1;
    }

    drawRectangleOnGrid(grid, x1, y1, armW, armH, 1);
    drawRectangleOnGrid(grid, x2, y2, armW, armH, 1);

    // Draw connector between the two arms
    for (i = 0; i < connectorW; i++) {
        grid[cx - connectorW / 2 + i][cy - 1] = 1;
        grid[cx - connectorW / 2 + i][cy] = 1;
        grid[cx - connectorW / 2 + i][cy + 1] = 1;
    }
}

// 15. T-shaped room: rectangle with perpendicular stem
static void designTShapedRoom(short **grid) {
    short barW, barH, stemW, stemH;
    short bx, by, sx, sy;
    short orientation; // 0=stem down, 1=stem up, 2=stem left, 3=stem right

    fillGrid(grid, 0);

    orientation = rand_range(0, 3);

    if (orientation <= 1) {
        // Horizontal bar, vertical stem
        barW = rand_range(7, 12);
        barH = rand_range(2, 4);
        stemW = rand_range(3, 5);
        stemH = rand_range(3, 6);

        bx = (DCOLS - barW) / 2;
        sx = (DCOLS - stemW) / 2;

        if (orientation == 0) { // stem goes down
            by = (DROWS - barH - stemH) / 2;
            sy = by + barH;
        } else { // stem goes up
            sy = (DROWS - barH - stemH) / 2;
            by = sy + stemH;
        }

        drawRectangleOnGrid(grid, bx, by, barW, barH, 1);
        drawRectangleOnGrid(grid, sx, sy, stemW, stemH, 1);
    } else {
        // Vertical bar, horizontal stem
        barH = rand_range(7, 12);
        barW = rand_range(2, 4);
        stemH = rand_range(3, 5);
        stemW = rand_range(3, 6);

        by = (DROWS - barH) / 2;
        sy = (DROWS - stemH) / 2;

        if (orientation == 2) { // stem goes left
            bx = (DCOLS - barW - stemW) / 2 + stemW;
            sx = bx - stemW;
        } else { // stem goes right
            bx = (DCOLS - barW - stemW) / 2;
            sx = bx + barW;
        }

        drawRectangleOnGrid(grid, bx, by, barW, barH, 1);
        drawRectangleOnGrid(grid, sx, sy, stemW, stemH, 1);
    }
}

// 16. Diamond room: rotated square using manhattan distance
static void designDiamondRoom(short **grid) {
    short radius, cx, cy, x, y;

    fillGrid(grid, 0);

    radius = rand_range(3, 5);
    cx = DCOLS / 2;
    cy = DROWS / 2;

    for (x = cx - radius; x <= cx + radius; x++) {
        for (y = cy - radius; y <= cy + radius; y++) {
            if (x >= 0 && x < DCOLS && y >= 0 && y < DROWS) {
                if (abs(x - cx) + abs(y - cy) <= radius) {
                    grid[x][y] = 1;
                }
            }
        }
    }
}

// 17. Dumbbell room: two circles connected by a narrow corridor
static void designDumbbellRoom(short **grid) {
    short radius, corridorW, corridorLen;
    short cx, cy;
    short x, y;
    boolean horizontal;

    fillGrid(grid, 0);

    radius = rand_range(2, 4);
    corridorW = rand_range(1, 2);
    corridorLen = rand_range(2, 4);
    horizontal = rand_percent(50);

    cx = DCOLS / 2;
    cy = DROWS / 2;

    if (horizontal) {
        short offset = radius + corridorLen / 2;
        // Left circle
        drawCircleOnGrid(grid, cx - offset, cy, radius, 1);
        // Right circle
        drawCircleOnGrid(grid, cx + offset, cy, radius, 1);
        // Corridor
        for (x = cx - offset + radius; x <= cx + offset - radius; x++) {
            for (y = cy - corridorW / 2; y <= cy + corridorW / 2; y++) {
                if (x >= 0 && x < DCOLS && y >= 0 && y < DROWS) {
                    grid[x][y] = 1;
                }
            }
        }
    } else {
        short offset = radius + corridorLen / 2;
        // Top circle
        drawCircleOnGrid(grid, cx, cy - offset, radius, 1);
        // Bottom circle
        drawCircleOnGrid(grid, cx, cy + offset, radius, 1);
        // Corridor
        for (y = cy - offset + radius; y <= cy + offset - radius; y++) {
            for (x = cx - corridorW / 2; x <= cx + corridorW / 2; x++) {
                if (x >= 0 && x < DCOLS && y >= 0 && y < DROWS) {
                    grid[x][y] = 1;
                }
            }
        }
    }
}

// If the indicated tile is a wall on the room stored in grid, and it could be the site of
// a door out of that room, then return the outbound direction that the door faces.
// Otherwise, return NO_DIRECTION.
static enum directions directionOfDoorSite(short **grid, short x, short y) {
    enum directions dir, solutionDir;
    short newX, newY, oppX, oppY;

    if (grid[x][y]) { // Already occupied
        return NO_DIRECTION;
    }

    solutionDir = NO_DIRECTION;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        oppX = x - nbDirs[dir][0];
        oppY = y - nbDirs[dir][1];
        if (coordinatesAreInMap(oppX, oppY)
            && coordinatesAreInMap(newX, newY)
            && grid[oppX][oppY] == 1) {

            // This grid cell would be a valid tile on which to place a door that, facing outward, points dir.
            if (solutionDir != NO_DIRECTION) {
                // Already claimed by another direction; no doors here!
                return NO_DIRECTION;
            }
            solutionDir = dir;
        }
    }
    return solutionDir;
}

static void chooseRandomDoorSites(short **roomMap, pos doorSites[4]) {
    short i, j, k, newX, newY;
    enum directions dir;
    short **grid;
    boolean doorSiteFailed;

    grid = allocGrid();
    copyGrid(grid, roomMap);

//    colorOverDungeon(&darkGray);
//    hiliteGrid(grid, &blue, 100);
//    temporaryMessage("Generating this room:", REQUIRE_ACKNOWLEDGMENT);
//    const char dirChars[] = "^v<>";

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (!grid[i][j]) {
                dir = directionOfDoorSite(roomMap, i, j);
                if (dir != NO_DIRECTION) {
                    // Trace a ray 10 spaces outward from the door site to make sure it doesn't intersect the room.
                    // If it does, it's not a valid door site.
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    doorSiteFailed = false;
                    for (k=0; k<10 && coordinatesAreInMap(newX, newY) && !doorSiteFailed; k++) {
                        if (grid[newX][newY]) {
                            doorSiteFailed = true;
                        }
                        newX += nbDirs[dir][0];
                        newY += nbDirs[dir][1];
                    }
                    if (!doorSiteFailed) {
//                        plotCharWithColor(dirChars[dir], mapToWindow((pos){ i, j }), &black, &green);
                        grid[i][j] = dir + 2; // So as not to conflict with 0 or 1, which are used to indicate exterior/interior.
                    }
                }
            }
        }
    }

//    temporaryMessage("Door candidates:", REQUIRE_ACKNOWLEDGMENT);

    // Pick four doors, one in each direction, and store them in doorSites[dir].
    for (dir=0; dir<4; dir++) {
        randomLocationInGrid(grid, &(doorSites[dir].x), &(doorSites[dir].y), dir + 2);
    }

    freeGrid(grid);
}

static void attachHallwayTo(short **grid, pos doorSites[4]) {
    short i, x, y, newX, newY, dirs[4];
    short length;
    enum directions dir, dir2;
    boolean allowObliqueHallwayExit;

    // Pick a direction.
    fillSequentialList(dirs, 4);
    shuffleList(dirs, 4);
    for (i=0; i<4; i++) {
        dir = dirs[i];
        if (doorSites[dir].x != -1
            && doorSites[dir].y != -1
            && coordinatesAreInMap(doorSites[dir].x + nbDirs[dir][0] * HORIZONTAL_CORRIDOR_MAX_LENGTH,
                                   doorSites[dir].y + nbDirs[dir][1] * VERTICAL_CORRIDOR_MAX_LENGTH)) {
                break; // That's our direction!
        }
    }
    if (i==4) {
        return; // No valid direction for hallways.
    }

    if (dir == UP || dir == DOWN) {
        length = rand_range(VERTICAL_CORRIDOR_MIN_LENGTH, VERTICAL_CORRIDOR_MAX_LENGTH);
    } else {
        length = rand_range(HORIZONTAL_CORRIDOR_MIN_LENGTH, HORIZONTAL_CORRIDOR_MAX_LENGTH);
    }

    x = doorSites[dir].x;
    y = doorSites[dir].y;
    for (i = 0; i < length; i++) {
        if (coordinatesAreInMap(x, y)) {
            grid[x][y] = true;
        }
        x += nbDirs[dir][0];
        y += nbDirs[dir][1];
    }
    x = clamp(x - nbDirs[dir][0], 0, DCOLS - 1);
    y = clamp(y - nbDirs[dir][1], 0, DROWS - 1); // Now (x, y) points at the last interior cell of the hallway.
    allowObliqueHallwayExit = rand_percent(15);
    for (dir2 = 0; dir2 < 4; dir2++) {
        newX = x + nbDirs[dir2][0];
        newY = y + nbDirs[dir2][1];

        if ((dir2 != dir && !allowObliqueHallwayExit)
            || !coordinatesAreInMap(newX, newY)
            || grid[newX][newY]) {

            doorSites[dir2] = INVALID_POS;
        } else {
            doorSites[dir2] = (pos){ .x = newX, .y = newY };
        }
    }
}

// Put a random room shape somewhere on the binary grid,
// and optionally record the coordinates of up to four door sites in doorSites.
// If attachHallway is true, then it will bolt a perpendicular hallway onto the room at one of the four standard door sites,
// and then relocate three of the door sites to radiate from the end of the hallway. (The fourth is defunct.)
// RoomTypeFrequencies specifies the probability of each room type, in the following order:
//      0. Cross room
//      1. Small symmetrical cross room
//      2. Small room
//      3. Circular room
//      4. Chunky room
//      5. Cave
//      6. Cavern (the kind that fills a level)
//      7. Entrance room (the big upside-down T room at the start of depth 1)
//      8. L-shaped room
//      9. Pillared hall
//      10. Nested/donut room
//      11. Trefoil/clover room
//      12. Cathedral room
//      13. Alcove room
//      14. Z-shaped room
//      15. T-shaped room
//      16. Diamond room
//      17. Dumbbell room

static void designRandomRoom(short **grid, boolean attachHallway, pos doorSites[4],
                      const short roomTypeFrequencies[ROOM_TYPE_COUNT]) {
    short randIndex, i, sum;
    enum directions dir;

    sum = 0;
    for (i=0; i<ROOM_TYPE_COUNT; i++) {
        sum += roomTypeFrequencies[i];
    }
    randIndex = rand_range(0, sum - 1);
    for (i=0; i<ROOM_TYPE_COUNT; i++) {
        if (randIndex < roomTypeFrequencies[i]) {
            break; // "i" is our room type.
        } else {
            randIndex -= roomTypeFrequencies[i];
        }
    }
    switch (i) {
        case 0:
            designCrossRoom(grid);
            break;
        case 1:
            designSymmetricalCrossRoom(grid);
            break;
        case 2:
            designSmallRoom(grid);
            break;
        case 3:
            designCircularRoom(grid);
            break;
        case 4:
            designChunkyRoom(grid);
            break;
        case 5:
            switch (rand_range(0, 2)) {
                case 0:
                    designCavern(grid, 3, 12, 4, 8); // Compact cave room.
                    break;
                case 1:
                    designCavern(grid, 3, 12, 15, DROWS-2); // Large north-south cave room.
                    break;
                case 2:
                    designCavern(grid, 20, DROWS-2, 4, 8); // Large east-west cave room.
                    break;
                default:
                    break;
            }
            break;
        case 6:
            designCavern(grid, CAVE_MIN_WIDTH, DCOLS - 2, CAVE_MIN_HEIGHT, DROWS - 2);
            break;
        case 7:
            designEntranceRoom(grid);
            break;
        case 8:
            designLShapedRoom(grid);
            break;
        case 9:
            designPillaredHall(grid);
            break;
        case 10:
            designNestedRoom(grid);
            break;
        case 11:
            designTrefoilRoom(grid);
            break;
        case 12:
            designCathedralRoom(grid);
            break;
        case 13:
            designAlcoveRoom(grid);
            break;
        case 14:
            designZShapedRoom(grid);
            break;
        case 15:
            designTShapedRoom(grid);
            break;
        case 16:
            designDiamondRoom(grid);
            break;
        case 17:
            designDumbbellRoom(grid);
            break;
        default:
            break;
    }

    if (doorSites) {
        chooseRandomDoorSites(grid, doorSites);
        if (attachHallway) {
            dir = rand_range(0, 3);
            for (i=0; doorSites[dir].x == -1 && i < 3; i++) {
                dir = (dir + 1) % 4; // Each room will have at least 2 valid directions for doors.
            }
            attachHallwayTo(grid, doorSites);
        }
    }
}

// Public wrapper to generate a specific room type by index. Used by tests.
void designRoomOfType(short **grid, int roomType) {
    short frequencies[ROOM_TYPE_COUNT];
    memset(frequencies, 0, sizeof(frequencies));
    if (roomType >= 0 && roomType < ROOM_TYPE_COUNT) {
        frequencies[roomType] = 1;
    }
    designRandomRoom(grid, false, NULL, frequencies);
}

static boolean roomFitsAt(short **dungeonMap, short **roomMap, short roomToDungeonX, short roomToDungeonY) {
    short xRoom, yRoom, xDungeon, yDungeon, i, j;

    for (xRoom = 0; xRoom < DCOLS; xRoom++) {
        for (yRoom = 0; yRoom < DROWS; yRoom++) {
            if (roomMap[xRoom][yRoom]) {
                xDungeon = xRoom + roomToDungeonX;
                yDungeon = yRoom + roomToDungeonY;

                for (i = xDungeon - 1; i <= xDungeon + 1; i++) {
                    for (j = yDungeon - 1; j <= yDungeon + 1; j++) {
                        if (!coordinatesAreInMap(i, j)
                            || dungeonMap[i][j] > 0) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

void attachRooms(short **grid, const dungeonProfile *theDP, short attempts, short maxRoomCount) {
    short roomsBuilt, roomsAttempted;
    short **roomMap;
    pos doorSites[4];
    short i, x, y, sCoord[DCOLS*DROWS];
    enum directions dir, oppDir;

    fillSequentialList(sCoord, DCOLS*DROWS);
    shuffleList(sCoord, DCOLS*DROWS);

    roomMap = allocGrid();
    for (roomsBuilt = roomsAttempted = 0; roomsBuilt < maxRoomCount && roomsAttempted < attempts; roomsAttempted++) {
        // Build a room in hyperspace.
        fillGrid(roomMap, 0);
        designRandomRoom(roomMap, roomsAttempted <= attempts - 5 && rand_percent(theDP->corridorChance),
                         doorSites, theDP->roomFrequencies);

        if (D_INSPECT_LEVELGEN) {
            colorOverDungeon(&darkGray);
            hiliteGrid(roomMap, &blue, 100);
            if (doorSites[0].x != -1) plotCharWithColor('^', mapToWindow(doorSites[0]), &black, &green);
            if (doorSites[1].x != -1) plotCharWithColor('v', mapToWindow(doorSites[1]), &black, &green);
            if (doorSites[2].x != -1) plotCharWithColor('<', mapToWindow(doorSites[2]), &black, &green);
            if (doorSites[3].x != -1) plotCharWithColor('>', mapToWindow(doorSites[3]), &black, &green);
            temporaryMessage("Generating this room:", REQUIRE_ACKNOWLEDGMENT);
        }

        // Slide hyperspace across real space, in a random but predetermined order, until the room matches up with a wall.
        for (i = 0; i < DCOLS*DROWS; i++) {
            x = sCoord[i] / DROWS;
            y = sCoord[i] % DROWS;

            dir = directionOfDoorSite(grid, x, y);
            oppDir = oppositeDirection(dir);
            if (dir != NO_DIRECTION
                && doorSites[oppDir].x != -1
                && roomFitsAt(grid, roomMap, x - doorSites[oppDir].x, y - doorSites[oppDir].y)) {

                // Room fits here.
                if (D_INSPECT_LEVELGEN) {
                    colorOverDungeon(&darkGray);
                    hiliteGrid(grid, &white, 100);
                }
                insertRoomAt(grid, roomMap, x - doorSites[oppDir].x, y - doorSites[oppDir].y, doorSites[oppDir].x, doorSites[oppDir].y);
                grid[x][y] = 2; // Door site.
                if (D_INSPECT_LEVELGEN) {
                    hiliteGrid(grid, &green, 50);
                    temporaryMessage("Added room.", REQUIRE_ACKNOWLEDGMENT);
                }
                roomsBuilt++;
                break;
            }
        }
    }

    freeGrid(roomMap);
}

static void adjustDungeonProfileForDepth(dungeonProfile *theProfile) {
    const short descentPercent = clamp(100 * (rogue.depthLevel - 1) / (gameConst->amuletLevel - 1), 0, 100);

    theProfile->roomFrequencies[0] += 20 * (100 - descentPercent) / 100;
    theProfile->roomFrequencies[1] += 10 * (100 - descentPercent) / 100;
    theProfile->roomFrequencies[3] +=  7 * (100 - descentPercent) / 100;
    theProfile->roomFrequencies[5] += 10 * descentPercent / 100;

    // New room types: slightly higher frequency at shallow depths
    theProfile->roomFrequencies[8]  += 3 * (100 - descentPercent) / 100; // L-shaped
    theProfile->roomFrequencies[9]  += 2 * (100 - descentPercent) / 100; // Pillared hall
    theProfile->roomFrequencies[13] += 3 * (100 - descentPercent) / 100; // Alcove
    theProfile->roomFrequencies[14] += 2 * (100 - descentPercent) / 100; // Z-shaped
    theProfile->roomFrequencies[15] += 3 * (100 - descentPercent) / 100; // T-shaped

    theProfile->corridorChance += 80 * (100 - descentPercent) / 100;
}

static void adjustDungeonFirstRoomProfileForDepth(dungeonProfile *theProfile) {
    short i;
    const short descentPercent = clamp(100 * (rogue.depthLevel - 1) / (gameConst->amuletLevel - 1), 0, 100);

    if (rogue.depthLevel == 1) {
        // All dungeons start with the entrance room on depth 1.
        for (i = 0; i < ROOM_TYPE_COUNT; i++) {
            theProfile->roomFrequencies[i] = 0;
        }
        theProfile->roomFrequencies[7] = 1;
    } else {
        theProfile->roomFrequencies[6] += 50 * descentPercent / 100;
    }
}

// Called by digDungeon().
// Slaps a bunch of rooms and hallways into the grid.
// On the grid, a 0 denotes granite, a 1 denotes floor, and a 2 denotes a possible door site.
// -1 denotes off-limits areas -- rooms can't be placed there and also can't sprout off of there.
// Parent function will translate this grid into pmap[][] to make floors, walls, doors, etc.
static void carveDungeon(short **grid) {
    dungeonProfile theDP, theFirstRoomDP;

    theDP = dungeonProfileCatalog[DP_BASIC];
    adjustDungeonProfileForDepth(&theDP);

    theFirstRoomDP = dungeonProfileCatalog[DP_BASIC_FIRST_ROOM];
    adjustDungeonFirstRoomProfileForDepth(&theFirstRoomDP);

    designRandomRoom(grid, false, NULL, theFirstRoomDP.roomFrequencies);

    if (D_INSPECT_LEVELGEN) {
        colorOverDungeon(&darkGray);
        hiliteGrid(grid, &white, 100);
        temporaryMessage("First room placed:", REQUIRE_ACKNOWLEDGMENT);
    }

    attachRooms(grid, &theDP, 35, 35);

//    colorOverDungeon(&darkGray);
//    hiliteGrid(grid, &white, 100);
//    temporaryMessage("How does this finished level look?", REQUIRE_ACKNOWLEDGMENT);
}

static void finishWalls(boolean includingDiagonals) {
    short i, j, x1, y1;
    boolean foundExposure;
    enum directions dir;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].layers[DUNGEON] == GRANITE) {
                foundExposure = false;
                for (dir = 0; dir < (includingDiagonals ? 8 : 4) && !foundExposure; dir++) {
                    x1 = i + nbDirs[dir][0];
                    y1 = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(x1, y1)
                        && (!cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_VISION) || !cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_PASSABILITY))) {

                        pmap[i][j].layers[DUNGEON] = WALL;
                        foundExposure = true;
                    }
                }
            } else if (pmap[i][j].layers[DUNGEON] == WALL) {
                foundExposure = false;
                for (dir = 0; dir < (includingDiagonals ? 8 : 4) && !foundExposure; dir++) {
                    x1 = i + nbDirs[dir][0];
                    y1 = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(x1, y1)
                        && (!cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_VISION) || !cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_PASSABILITY))) {

                        foundExposure = true;
                    }
                }
                if (foundExposure == false) {
                    pmap[i][j].layers[DUNGEON] = GRANITE;
                }
            }
        }
    }
}

static void liquidType(short *deep, short *shallow, short *shallowWidth) {
    short randMin, randMax, rand;

    randMin = (rogue.depthLevel < gameConst->minimumLavaLevel ? 1 : 0);
    randMax = (rogue.depthLevel < gameConst->minimumBrimstoneLevel ? 2 : 3);
    rand = rand_range(randMin, randMax);
    if (rogue.depthLevel == gameConst->deepestLevel) {
        rand = 1;
    }

    switch(rand) {
        case 0:
            *deep = LAVA;
            *shallow = NOTHING;
            *shallowWidth = 0;
            break;
        case 1:
            *deep = DEEP_WATER;
            *shallow = SHALLOW_WATER;
            *shallowWidth = 2;
            break;
        case 2:
            *deep = CHASM;
            *shallow = CHASM_EDGE;
            *shallowWidth = 1;
            break;
        case 3:
            *deep = INERT_BRIMSTONE;
            *shallow = OBSIDIAN;
            *shallowWidth = 2;
            break;
    }
}

// Fills a lake marked in unfilledLakeMap with the specified liquid type, scanning outward to reach other lakes within scanWidth.
// Any wreath of shallow liquid must be done elsewhere.
static void fillLake(short x, short y, short liquid, short scanWidth, char wreathMap[DCOLS][DROWS], short **unfilledLakeMap) {
    short i, j;

    for (i = x - scanWidth; i <= x + scanWidth; i++) {
        for (j = y - scanWidth; j <= y + scanWidth; j++) {
            if (coordinatesAreInMap(i, j) && unfilledLakeMap[i][j]) {
                unfilledLakeMap[i][j] = false;
                pmap[i][j].layers[LIQUID] = liquid;
                wreathMap[i][j] = 1;
                fillLake(i, j, liquid, scanWidth, wreathMap, unfilledLakeMap);  // recursive
            }
        }
    }
}

static void lakeFloodFill(short x, short y, short **floodMap, short **grid, short **lakeMap, short dungeonToGridX, short dungeonToGridY) {
    short newX, newY;
    enum directions dir;

    floodMap[x][y] = true;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && !floodMap[newX][newY]
            && (!cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER) || cellHasTMFlag((pos){ newX, newY }, TM_CONNECTS_LEVEL))
            && !lakeMap[newX][newY]
            && (!coordinatesAreInMap(newX+dungeonToGridX, newY+dungeonToGridY) || !grid[newX+dungeonToGridX][newY+dungeonToGridY])) {

            lakeFloodFill(newX, newY, floodMap, grid, lakeMap, dungeonToGridX, dungeonToGridY);
        }
    }
}

static boolean lakeDisruptsPassability(short **grid, short **lakeMap, short dungeonToGridX, short dungeonToGridY) {
    boolean result;
    short i, j, x, y;
    short **floodMap;

    floodMap = allocGrid();
    fillGrid(floodMap, 0);
    x = y = -1;
    // Get starting location for the fill.
    for (i=0; i<DCOLS && x == -1; i++) {
        for (j=0; j<DROWS && x == -1; j++) {
            if (!cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                && !lakeMap[i][j]
                && (!coordinatesAreInMap(i+dungeonToGridX, j+dungeonToGridY) || !grid[i+dungeonToGridX][j+dungeonToGridY])) {

                x = i;
                y = j;
            }
        }
    }
    brogueAssert(x != -1);
    // Do the flood fill.
    lakeFloodFill(x, y, floodMap, grid, lakeMap, dungeonToGridX, dungeonToGridY);

    // See if any dry tiles weren't reached by the flood fill.
    result = false;
    for (i=0; i<DCOLS && result == false; i++) {
        for (j=0; j<DROWS && result == false; j++) {
            if (!cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                && !lakeMap[i][j]
                && !floodMap[i][j]
                && (!coordinatesAreInMap(i+dungeonToGridX, j+dungeonToGridY) || !grid[i+dungeonToGridX][j+dungeonToGridY])) {

//                if (D_INSPECT_LEVELGEN) {
//                    dumpLevelToScreen();
//                    hiliteGrid(lakeMap, &darkBlue, 75);
//                    hiliteGrid(floodMap, &white, 20);
//                    plotCharWithColor('X', mapToWindow((pos){ i, j }), &black, &red);
//                    temporaryMessage("Failed here.", REQUIRE_ACKNOWLEDGMENT);
//                }

                result = true;
            }
        }
    }

    freeGrid(floodMap);
    return result;
}

static void designLakes(short **lakeMap) {
    short i, j, k;
    short x, y;
    short lakeMaxHeight, lakeMaxWidth;
    short lakeX, lakeY, lakeWidth, lakeHeight;

    short **grid; // Holds the current lake.

    grid = allocGrid();
    fillGrid(lakeMap, 0);
    for (lakeMaxHeight = 15, lakeMaxWidth = 30; lakeMaxHeight >=10; lakeMaxHeight--, lakeMaxWidth -= 2) { // lake generations

        fillGrid(grid, 0);
        createBlobOnGrid(grid, &lakeX, &lakeY, &lakeWidth, &lakeHeight, 5, 4, 4, lakeMaxWidth, lakeMaxHeight, 55, "ffffftttt", "ffffttttt");

//        if (D_INSPECT_LEVELGEN) {
//            colorOverDungeon(&darkGray);
//            hiliteGrid(grid, &white, 100);
//            temporaryMessage("Generated a lake.", REQUIRE_ACKNOWLEDGMENT);
//        }

        for (k=0; k<20; k++) { // placement attempts
            // propose a position for the top-left of the grid in the dungeon
            x = rand_range(1 - lakeX, DCOLS - lakeWidth - lakeX - 2);
            y = rand_range(1 - lakeY, DROWS - lakeHeight - lakeY - 2);

            if (!lakeDisruptsPassability(grid, lakeMap, -x, -y)) { // level with lake is completely connected
                //printf("Placed a lake!");

                // copy in lake
                for (i = 0; i < lakeWidth; i++) {
                    for (j = 0; j < lakeHeight; j++) {
                        if (grid[i + lakeX][j + lakeY]) {
                            lakeMap[i + lakeX + x][j + lakeY + y] = true;
                            pmap[i + lakeX + x][j + lakeY + y].layers[DUNGEON] = FLOOR;
                        }
                    }
                }

                if (D_INSPECT_LEVELGEN) {
                    dumpLevelToScreen();
                    hiliteGrid(lakeMap, &white, 50);
                    temporaryMessage("Added a lake location.", REQUIRE_ACKNOWLEDGMENT);
                }
                break;
            }
        }
    }
    freeGrid(grid);
}

static void createWreath(short shallowLiquid, short wreathWidth, char wreathMap[DCOLS][DROWS]) {
    short i, j, k, l;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (wreathMap[i][j]) {
                for (k = i-wreathWidth; k<= i+wreathWidth; k++) {
                    for (l = j-wreathWidth; l <= j+wreathWidth; l++) {
                        if (coordinatesAreInMap(k, l) && pmap[k][l].layers[LIQUID] == NOTHING
                            && (i-k)*(i-k) + (j-l)*(j-l) <= wreathWidth*wreathWidth) {
                            pmap[k][l].layers[LIQUID] = shallowLiquid;
                            if (pmap[k][l].layers[DUNGEON] == DOOR) {
                                pmap[k][l].layers[DUNGEON] = FLOOR;
                            }
                        }
                    }
                }
            }
        }
    }
}

static void fillLakes(short **lakeMap) {
    short deepLiquid = CRYSTAL_WALL, shallowLiquid = CRYSTAL_WALL, shallowLiquidWidth = 0;
    char wreathMap[DCOLS][DROWS];
    short i, j;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (lakeMap[i][j]) {
                liquidType(&deepLiquid, &shallowLiquid, &shallowLiquidWidth);
                zeroOutGrid(wreathMap);
                fillLake(i, j, deepLiquid, 4, wreathMap, lakeMap);
                createWreath(shallowLiquid, shallowLiquidWidth, wreathMap);

                if (D_INSPECT_LEVELGEN) {
                    dumpLevelToScreen();
                    hiliteGrid(lakeMap, &white, 75);
                    temporaryMessage("Lake filled.", REQUIRE_ACKNOWLEDGMENT);
                }
            }
        }
    }
}

static void finishDoors(void) {
    short i, j;
    const short secretDoorChance = clamp((rogue.depthLevel - 1) * 67 / (gameConst->amuletLevel - 1), 0, 67);
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (pmap[i][j].layers[DUNGEON] == DOOR
                && pmap[i][j].machineNumber == 0) {
                if ((!cellHasTerrainFlag((pos){ i+1, j }, T_OBSTRUCTS_PASSABILITY) || !cellHasTerrainFlag((pos){ i-1, j }, T_OBSTRUCTS_PASSABILITY))
                    && (!cellHasTerrainFlag((pos){ i, j+1 }, T_OBSTRUCTS_PASSABILITY) || !cellHasTerrainFlag((pos){ i, j-1 }, T_OBSTRUCTS_PASSABILITY))) {
                    // If there's passable terrain to the left or right, and there's passable terrain
                    // above or below, then the door is orphaned and must be removed.
                    pmap[i][j].layers[DUNGEON] = FLOOR;
                } else if ((cellHasTerrainFlag((pos){ i+1, j }, T_PATHING_BLOCKER) ? 1 : 0)
                           + (cellHasTerrainFlag((pos){ i-1, j }, T_PATHING_BLOCKER) ? 1 : 0)
                           + (cellHasTerrainFlag((pos){ i, j+1 }, T_PATHING_BLOCKER) ? 1 : 0)
                           + (cellHasTerrainFlag((pos){ i, j-1 }, T_PATHING_BLOCKER) ? 1 : 0) >= 3) {
                    // If the door has three or more pathing blocker neighbors in the four cardinal directions,
                    // then the door is orphaned and must be removed.
                    pmap[i][j].layers[DUNGEON] = FLOOR;
                } else if (rand_percent(secretDoorChance)) {
                    pmap[i][j].layers[DUNGEON] = SECRET_DOOR;
                }
            }
        }
    }
}

static void clearLevel(void) {
    short i, j;

    for( i=0; i<DCOLS; i++ ) {
        for( j=0; j<DROWS; j++ ) {
            pmap[i][j].layers[DUNGEON] = GRANITE;
            pmap[i][j].layers[LIQUID] = NOTHING;
            pmap[i][j].layers[GAS] = NOTHING;
            pmap[i][j].layers[SURFACE] = NOTHING;
            pmap[i][j].machineNumber = 0;
            pmap[i][j].rememberedTerrain = NOTHING;
            pmap[i][j].rememberedTerrainFlags = (T_OBSTRUCTS_EVERYTHING);
            pmap[i][j].rememberedTMFlags = 0;
            pmap[i][j].rememberedCellFlags = 0;
            pmap[i][j].rememberedItemCategory = 0;
            pmap[i][j].rememberedItemKind = 0;
            pmap[i][j].rememberedItemQuantity = 0;
            pmap[i][j].rememberedItemOriginDepth = 0;
            pmap[i][j].flags = 0;
            pmap[i][j].volume = 0;
        }
    }
}

// Scans the map in random order looking for a good place to build a bridge.
// If it finds one, it builds a bridge there, halts and returns true.
static boolean buildABridge(void) {
    short i, j, k, l, i2, j2, nCols[DCOLS], nRows[DROWS];
    short bridgeRatioX, bridgeRatioY;
    boolean foundExposure;

    bridgeRatioX = (short) (100 + (100 + 100 * rogue.depthLevel * gameConst->depthAccelerator / 9) * rand_range(10, 20) / 10);
    bridgeRatioY = (short) (100 + (400 + 100 * rogue.depthLevel * gameConst->depthAccelerator / 18) * rand_range(10, 20) / 10);

    fillSequentialList(nCols, DCOLS);
    shuffleList(nCols, DCOLS);
    fillSequentialList(nRows, DROWS);
    shuffleList(nRows, DROWS);

    for (i2=1; i2<DCOLS-1; i2++) {
        i = nCols[i2];
        for (j2=1; j2<DROWS-1; j2++) {
            j = nRows[j2];
            if (!cellHasTerrainFlag((pos){ i, j }, (T_CAN_BE_BRIDGED | T_PATHING_BLOCKER))
                && !pmap[i][j].machineNumber) {

                // try a horizontal bridge
                foundExposure = false;
                for (k = i + 1;
                     k < DCOLS // Iterate across the prospective length of the bridge.
                     && !pmap[k][j].machineNumber // No bridges in machines.
                     && cellHasTerrainFlag((pos){ k, j }, T_CAN_BE_BRIDGED)  // Candidate tile must be chasm.
                     && !cellHasTMFlag((pos){ k, j }, TM_IS_SECRET) // Can't bridge over secret trapdoors.
                     && !cellHasTerrainFlag((pos){ k, j }, T_OBSTRUCTS_PASSABILITY)  // Candidate tile cannot be a wall.
                     && cellHasTerrainFlag((pos){ k, j-1 }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY))    // Only chasms or walls are permitted next to the length of the bridge.
                     && cellHasTerrainFlag((pos){ k, j+1 }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY));
                     k++) {

                    if (!cellHasTerrainFlag((pos){ k, j-1 }, T_OBSTRUCTS_PASSABILITY) // Can't run against a wall the whole way.
                        && !cellHasTerrainFlag((pos){ k, j+1 }, T_OBSTRUCTS_PASSABILITY)) {
                        foundExposure = true;
                    }
                }
                if (k < DCOLS
                    && (k - i > 3) // Can't have bridges shorter than 3 spaces.
                    && foundExposure
                    && !cellHasTerrainFlag((pos){ k, j }, T_PATHING_BLOCKER | T_CAN_BE_BRIDGED) // Must end on an unobstructed land tile.
                    && !pmap[k][j].machineNumber // Cannot end in a machine.
                    && 100 * pathingDistance(i, j, k, j, T_PATHING_BLOCKER) / (k - i) > bridgeRatioX) { // Must shorten the pathing distance enough.

                    for (l=i+1; l < k; l++) {
                        pmap[l][j].layers[LIQUID] = BRIDGE;
                    }
                    pmap[i][j].layers[SURFACE] = BRIDGE_EDGE;
                    pmap[k][j].layers[SURFACE] = BRIDGE_EDGE;
                    return true;
                }

                // try a vertical bridge
                foundExposure = false;
                for (k = j + 1;
                     k < DROWS
                     && !pmap[i][k].machineNumber
                     && cellHasTerrainFlag((pos){ i, k }, T_CAN_BE_BRIDGED)
                     && !cellHasTMFlag((pos){ i, k }, TM_IS_SECRET)
                     && !cellHasTerrainFlag((pos){ i, k }, T_OBSTRUCTS_PASSABILITY)
                     && cellHasTerrainFlag((pos){ i-1, k }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY))
                     && cellHasTerrainFlag((pos){ i+1, k }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY));
                     k++) {

                    if (!cellHasTerrainFlag((pos){ i-1, k }, T_OBSTRUCTS_PASSABILITY)
                        && !cellHasTerrainFlag((pos){ i+1, k }, T_OBSTRUCTS_PASSABILITY)) {
                        foundExposure = true;
                    }
                }
                if (k < DROWS
                    && (k - j > 3)
                    && foundExposure
                    && !cellHasTerrainFlag((pos){ i, k }, T_PATHING_BLOCKER | T_CAN_BE_BRIDGED)
                    && !pmap[i][k].machineNumber // Cannot end in a machine.
                    && 100 * pathingDistance(i, j, i, k, T_PATHING_BLOCKER) / (k - j) > bridgeRatioY) {

                    for (l=j+1; l < k; l++) {
                        pmap[i][l].layers[LIQUID] = BRIDGE;
                    }
                    pmap[i][j].layers[SURFACE] = BRIDGE_EDGE;
                    pmap[i][k].layers[SURFACE] = BRIDGE_EDGE;
                    return true;
                }
            }
        }
    }
    return false;
}

// This is the master function for digging out a dungeon level.
// Finishing touches -- items, monsters, staircases, etc. -- are handled elsewhere.
void digDungeon(void) {
    short i, j;

    short **grid;

    rogue.machineNumber = 0;
    rogue.placedMachineCount = 0;

    topBlobMinX = topBlobMinY = blobWidth = blobHeight = 0;

#ifdef AUDIT_RNG
    char RNGMessage[100];
    sprintf(RNGMessage, "\n\n\nDigging dungeon level %i:\n", rogue.depthLevel);
    RNGLog(RNGMessage);
#endif

    // Clear level and fill with granite
    clearLevel();

    grid = allocGrid();
    carveDungeon(grid);
    addLoops(grid, 20);
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (grid[i][j] == 1) {
                pmap[i][j].layers[DUNGEON] = FLOOR;
            } else if (grid[i][j] == 2) {
                pmap[i][j].layers[DUNGEON] = (rand_percent(60) && rogue.depthLevel < gameConst->deepestLevel ? DOOR : FLOOR);
            }
        }
    }
    freeGrid(grid);

    finishWalls(false);

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Carved into the granite:", REQUIRE_ACKNOWLEDGMENT);
    }
    //DEBUG printf("\n%i loops created.", numLoops);
    //DEBUG logLevel();

    // Time to add lakes and chasms. Strategy is to generate a series of blob lakes of decreasing size. For each lake,
    // propose a position, and then check via a flood fill that the level would remain connected with that placement (i.e. that
    // each passable tile can still be reached). If not, make 9 more placement attempts before abandoning that lake
    // and proceeding to generate the next smaller one.
    // Canvas sizes start at 30x15 and decrease by 2x1 at a time down to a minimum of 20x10. Min generated size is always 4x4.

    // DEBUG logLevel();

    // Now design the lakes and then fill them with various liquids (lava, water, chasm, brimstone).
    short **lakeMap = allocGrid();
    designLakes(lakeMap);
    fillLakes(lakeMap);
    freeGrid(lakeMap);

    // Run the non-machine autoGenerators.
    runAutogenerators(false);

    // Remove diagonal openings.
    removeDiagonalOpenings();

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Diagonal openings removed.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Now add some treasure machines.
    addMachines();

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Machines added.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Run the machine autoGenerators.
    runAutogenerators(true);

    // Now knock down the boundaries between similar lakes where possible.
    cleanUpLakeBoundaries();

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Lake boundaries cleaned up.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Now add some bridges.
    while (buildABridge());

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Bridges added.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Now remove orphaned doors and upgrade some doors to secret doors
    finishDoors();

    // Now finish any exposed granite with walls and revert any unexposed walls to granite
    finishWalls(true);

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Finishing touches added. Level has been generated.", REQUIRE_ACKNOWLEDGMENT);
    }
}

void updateMapToShore(void) {
    short i, j;
    short **costMap;

    rogue.updatedMapToShoreThisTurn = true;

    costMap = allocGrid();

    // Calculate the map to shore for this level
    if (!rogue.mapToShore) {
        rogue.mapToShore = allocGrid();
        fillGrid(rogue.mapToShore, 0);
    }
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)) {
                costMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                rogue.mapToShore[i][j] = 30000;
            } else {
                costMap[i][j] = 1;
                rogue.mapToShore[i][j] = (cellHasTerrainFlag((pos){ i, j }, T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_AUTO_DESCENT)
                                          && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)) ? 30000 : 0;
            }
        }
    }
    dijkstraScan(rogue.mapToShore, costMap, true);
    freeGrid(costMap);
}

// Calculates the distance map for the given waypoint.
// This is called on all waypoints during setUpWaypoints(),
// and then one waypoint is recalculated per turn thereafter.
void refreshWaypoint(short wpIndex) {
    short **costMap;

    costMap = allocGrid();
    populateGenericCostMap(costMap);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature* monst = nextCreature(&it);
        if ((monst->creatureState == MONSTER_SLEEPING || (monst->info.flags & MONST_IMMOBILE) || (monst->bookkeepingFlags & MB_CAPTIVE))
            && costMap[monst->loc.x][monst->loc.y] >= 0) {

            costMap[monst->loc.x][monst->loc.y] = PDS_FORBIDDEN;
        }
    }
    fillGrid(rogue.wpDistance[wpIndex], 30000);
    rogue.wpDistance[wpIndex][rogue.wpCoordinates[wpIndex].x][rogue.wpCoordinates[wpIndex].y] = 0;
    dijkstraScan(rogue.wpDistance[wpIndex], costMap, true);
    freeGrid(costMap);
}

void setUpWaypoints(void) {
    short i, j, sCoord[DCOLS * DROWS], x, y;
    char grid[DCOLS][DROWS];

    zeroOutGrid(grid);
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_SCENT)) {
                grid[i][j] = 1;
            }
        }
    }
    rogue.wpCount = 0;
    rogue.wpRefreshTicker = 0;
    fillSequentialList(sCoord, DCOLS*DROWS);
    shuffleList(sCoord, DCOLS*DROWS);
    for (i = 0; i < DCOLS*DROWS && rogue.wpCount < MAX_WAYPOINT_COUNT; i++) {
        x = sCoord[i]/DROWS;
        y = sCoord[i] % DROWS;
        if (!grid[x][y]) {
            getFOVMask(grid, x, y, WAYPOINT_SIGHT_RADIUS * FP_FACTOR, T_OBSTRUCTS_SCENT, 0, false);
            grid[x][y] = true;
            rogue.wpCoordinates[rogue.wpCount] = (pos) { x, y };
            rogue.wpCount++;
//            blackOutScreen();
//            dumpLevelToScreen();
//            hiliteCharGrid(grid, &yellow, 50);
//            temporaryMessage("Waypoint coverage so far:", REQUIRE_ACKNOWLEDGMENT);
        }
    }

    for (i=0; i<rogue.wpCount; i++) {
        refreshWaypoint(i);
//        blackOutScreen();
//        dumpLevelToScreen();
//        displayGrid(rogue.wpDistance[i]);
//        temporaryMessage("Waypoint distance map:", REQUIRE_ACKNOWLEDGMENT);
    }
}

void zeroOutGrid(char grid[DCOLS][DROWS]) {
    short i, j;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            grid[i][j] = 0;
        }
    }
}

short oppositeDirection(short theDir) {
    switch (theDir) {
        case UP:
            return DOWN;
        case DOWN:
            return UP;
        case LEFT:
            return RIGHT;
        case RIGHT:
            return LEFT;
        case UPRIGHT:
            return DOWNLEFT;
        case DOWNLEFT:
            return UPRIGHT;
        case UPLEFT:
            return DOWNRIGHT;
        case DOWNRIGHT:
            return UPLEFT;
        case NO_DIRECTION:
            return NO_DIRECTION;
        default:
            return -1;
    }
}

// blockingMap is optional.
// Returns the size of the connected zone, and marks visited[][] with the zoneLabel.
static short connectCell(short x, short y, short zoneLabel, char blockingMap[DCOLS][DROWS], char zoneMap[DCOLS][DROWS]) {
    enum directions dir;
    short newX, newY, size;

    zoneMap[x][y] = zoneLabel;
    size = 1;

    for (dir = 0; dir < 4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];

        if (coordinatesAreInMap(newX, newY)
            && zoneMap[newX][newY] == 0
            && (!blockingMap || !blockingMap[newX][newY])
            && cellIsPassableOrDoor(newX, newY)) {

            size += connectCell(newX, newY, zoneLabel, blockingMap, zoneMap);
        }
    }
    return size;
}

// Make a zone map of connected passable regions that include at least one passable
// cell that borders the blockingMap if blockingMap blocks. Keep track of the size of each zone.
// Then pretend that the blockingMap no longer blocks, and grow these zones into the resulting area
// (without changing the stored zone sizes). If two or more zones now touch, then we block.
// At that point, return the size in cells of the smallest of all of the touching regions
// (or just 1, i.e. true, if countRegionSize is false). If no zones touch, then we don't block, and we return zero, i.e. false.
short levelIsDisconnectedWithBlockingMap(char blockingMap[DCOLS][DROWS], boolean countRegionSize) {
    char zoneMap[DCOLS][DROWS];
    short i, j, dir, zoneSizes[200], zoneCount, smallestQualifyingZoneSize, borderingZone;

    zoneCount = 0;
    smallestQualifyingZoneSize = 10000;
    zeroOutGrid(zoneMap);

//  dumpLevelToScreen();
//  hiliteCharGrid(blockingMap, &omniscienceColor, 100);
//  temporaryMessage("Blocking map:", REQUIRE_ACKNOWLEDGMENT);

    // Map out the zones with the blocking area blocked.
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (cellIsPassableOrDoor(i, j) && zoneMap[i][j] == 0 && !blockingMap[i][j]) {
                for (dir=0; dir<4; dir++) {
                    if (blockingMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]]) {
                        zoneCount++;
                        zoneSizes[zoneCount - 1] = connectCell(i, j, zoneCount, blockingMap, zoneMap);
                        break;
                    }
                }
            }
        }
    }

    // Expand the zones into the blocking area.
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (blockingMap[i][j] && zoneMap[i][j] == 0 && cellIsPassableOrDoor(i, j)) {
                for (dir=0; dir<4; dir++) {
                    borderingZone = zoneMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]];
                    if (borderingZone != 0) {
                        connectCell(i, j, borderingZone, NULL, zoneMap);
                        break;
                    }
                }
            }
        }
    }

    // Figure out which zones touch.
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (zoneMap[i][j] != 0) {
                for (dir=0; dir<4; dir++) {
                    borderingZone = zoneMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]];
                    if (zoneMap[i][j] != borderingZone && borderingZone != 0) {
                        if (!countRegionSize) {
                            return true;
                        }
                        smallestQualifyingZoneSize = min(smallestQualifyingZoneSize, zoneSizes[zoneMap[i][j] - 1]);
                        smallestQualifyingZoneSize = min(smallestQualifyingZoneSize, zoneSizes[borderingZone - 1]);
                        break;
                    }
                }
            }
        }
    }
    return (smallestQualifyingZoneSize < 10000 ? smallestQualifyingZoneSize : 0);
}

void resetDFMessageEligibility(void) {
    short i;

    for (i=0; i<NUMBER_DUNGEON_FEATURES; i++) {
        dungeonFeatureCatalog[i].messageDisplayed = false;
    }
}

boolean fillSpawnMap(enum dungeonLayers layer,
                     enum tileType surfaceTileType,
                     char spawnMap[DCOLS][DROWS],
                     boolean blockedByOtherLayers,
                     boolean refresh,
                     boolean superpriority) {
    short i, j;
    creature *monst;
    item *theItem;
    boolean accomplishedSomething;

    accomplishedSomething = false;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (    // If it's flagged for building in the spawn map,
                spawnMap[i][j]
                    // and the new cell doesn't already contain the fill terrain,
                && pmap[i][j].layers[layer] != surfaceTileType
                    // and the terrain in the layer to be overwritten has a higher priority number (unless superpriority),
                && (superpriority || tileCatalog[pmap[i][j].layers[layer]].drawPriority >= tileCatalog[surfaceTileType].drawPriority)
                    // and we won't be painting into the surface layer when that cell forbids it,
                && !(layer == SURFACE && cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_SURFACE_EFFECTS))
                    // and, if requested, the fill won't violate the priority of the most important terrain in this cell:
                && (!blockedByOtherLayers || tileCatalog[pmap[i][j].layers[highestPriorityLayer(i, j, true)]].drawPriority >= tileCatalog[surfaceTileType].drawPriority)
                ) {

                if ((tileCatalog[surfaceTileType].flags & T_IS_FIRE)
                    && !(tileCatalog[pmap[i][j].layers[layer]].flags & T_IS_FIRE)) {
                    pmap[i][j].flags |= CAUGHT_FIRE_THIS_TURN;
                }

                if ((tileCatalog[pmap[i][j].layers[layer]].flags & T_PATHING_BLOCKER)
                    != (tileCatalog[surfaceTileType].flags & T_PATHING_BLOCKER)) {

                    rogue.staleLoopMap = true;
                }

                pmap[i][j].layers[layer] = surfaceTileType; // Place the terrain!
                accomplishedSomething = true;

                if (refresh) {
                    refreshDungeonCell((pos){ i, j });
                    if (player.loc.x == i && player.loc.y == j && !player.status[STATUS_LEVITATING] && refresh) {
                        flavorMessage(tileFlavor(player.loc.x, player.loc.y));
                    }
                    if (pmap[i][j].flags & (HAS_MONSTER | HAS_PLAYER)) {
                        monst = monsterAtLoc((pos){ i, j });
                        applyInstantTileEffectsToCreature(monst);
                        if (rogue.gameHasEnded) {
                            return true;
                        }
                    }
                    if (tileCatalog[surfaceTileType].flags & T_IS_FIRE) {
                        if (pmap[i][j].flags & HAS_ITEM) {
                            theItem = itemAtLoc((pos){ i, j });
                            if (theItem->flags & ITEM_FLAMMABLE) {
                                burnItem(theItem);
                            }
                        }
                    }
                }
            } else {
                spawnMap[i][j] = false; // so that the spawnmap reflects what actually got built
            }
        }
    }
    return accomplishedSomething;
}

static void spawnMapDF(short x, short y,
                enum tileType propagationTerrain,
                boolean requirePropTerrain,
                short startProb,
                short probDec,
                char spawnMap[DCOLS][DROWS]) {

    short i, j, dir, t, x2, y2;
    boolean madeChange;

    spawnMap[x][y] = t = 1; // incremented before anything else happens

    madeChange = true;

    while (madeChange && startProb > 0) {
        madeChange = false;
        t++;
        for (i = 0; i < DCOLS; i++) {
            for (j=0; j < DROWS; j++) {
                if (spawnMap[i][j] == t - 1) {
                    for (dir = 0; dir < 4; dir++) {
                        x2 = i + nbDirs[dir][0];
                        y2 = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(x2, y2)
                            && (!requirePropTerrain || (propagationTerrain > 0 && cellHasTerrainType((pos){ x2, y2 }, propagationTerrain)))
                            && (!cellHasTerrainFlag((pos){ x2, y2 }, T_OBSTRUCTS_SURFACE_EFFECTS) || (propagationTerrain > 0 && cellHasTerrainType((pos){ x2, y2 }, propagationTerrain)))
                            && rand_percent(startProb)) {

                            spawnMap[x2][y2] = t;
                            madeChange = true;
                        }
                    }
                }
            }
        }
        startProb -= probDec;
        if (t > 100) {
            for (i = 0; i < DCOLS; i++) {
                for (j=0; j < DROWS; j++) {
                    if (spawnMap[i][j] == t) {
                        spawnMap[i][j] = 2;
                    } else if (spawnMap[i][j] > 0) {
                        spawnMap[i][j] = 1;
                    }
                }
            }
            t = 2;
        }
    }
    if (requirePropTerrain && !cellHasTerrainType((pos){ x, y }, propagationTerrain)) {
        spawnMap[x][y] = 0;
    }
}

static void evacuateCreatures(char blockingMap[DCOLS][DROWS]) {
    creature *monst;

    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            if (blockingMap[i][j]
                && (pmap[i][j].flags & (HAS_MONSTER | HAS_PLAYER))) {

                monst = monsterAtLoc((pos) { i, j });
                pos newLoc;
                getQualifyingLocNear(&newLoc,
                                     (pos){ i, j },
                                     true,
                                     blockingMap,
                                     forbiddenFlagsForMonster(&(monst->info)),
                                     (HAS_MONSTER | HAS_PLAYER),
                                     false,
                                     false);
                monst->loc = newLoc;
                pmap[i][j].flags &= ~(HAS_MONSTER | HAS_PLAYER);
                pmapAt(newLoc)->flags |= (monst == &player ? HAS_PLAYER : HAS_MONSTER);
            }
        }
    }
}

// returns whether the feature was successfully generated (false if we aborted because of blocking)
boolean spawnDungeonFeature(short x, short y, dungeonFeature *feat, boolean refreshCell, boolean abortIfBlocking) {
    short i, j, layer;
    char blockingMap[DCOLS][DROWS];
    boolean blocking;
    boolean succeeded;

    if ((feat->flags & DFF_RESURRECT_ALLY)
        && !resurrectAlly((pos){ x, y })) {
        return false;
    }

    if (feat->description[0] && !feat->messageDisplayed && playerCanSee(x, y)) {
        feat->messageDisplayed = true;
        message(feat->description, 0);
    }

    zeroOutGrid(blockingMap);

    // Blocking keeps track of whether to abort if it turns out that the DF would obstruct the level.
    blocking = ((abortIfBlocking
                 && !(feat->flags & DFF_PERMIT_BLOCKING)
                 && ((tileCatalog[feat->tile].flags & (T_PATHING_BLOCKER))
                     || (feat->flags & DFF_TREAT_AS_BLOCKING))) ? true : false);

    if (feat->tile) {
        if (feat->layer == GAS) {
            pmap[x][y].volume += feat->startProbability;
            pmap[x][y].layers[GAS] = feat->tile;
            if (refreshCell) {
                refreshDungeonCell((pos){ x, y });
            }
            succeeded = true;
        } else {
            spawnMapDF(x, y,
                       feat->propagationTerrain,
                       (feat->propagationTerrain ? true : false),
                       feat->startProbability,
                       feat->probabilityDecrement,
                       blockingMap);
            if (!blocking || !levelIsDisconnectedWithBlockingMap(blockingMap, false)) {
                if (feat->flags & DFF_EVACUATE_CREATURES_FIRST) { // first, evacuate creatures if necessary, so that they do not re-trigger the tile.
                    evacuateCreatures(blockingMap);
                }

                //succeeded = fillSpawnMap(feat->layer, feat->tile, blockingMap, (feat->flags & DFF_BLOCKED_BY_OTHER_LAYERS), refreshCell, (feat->flags & DFF_SUPERPRIORITY));
                fillSpawnMap(feat->layer,
                             feat->tile,
                             blockingMap,
                             (feat->flags & DFF_BLOCKED_BY_OTHER_LAYERS),
                             refreshCell,
                             (feat->flags & DFF_SUPERPRIORITY)); // this can tweak the spawn map too
                succeeded = true; // fail ONLY if we blocked the level. We succeed even if, thanks to priority, nothing gets built.
            } else {
                succeeded = false;
            }
        }
    } else {
        blockingMap[x][y] = true;
        succeeded = true; // Automatically succeed if there is no terrain to place.
        if (feat->flags & DFF_EVACUATE_CREATURES_FIRST) { // first, evacuate creatures if necessary, so that they do not re-trigger the tile.
            evacuateCreatures(blockingMap);
        }
    }

    if (succeeded && (feat->flags & (DFF_CLEAR_LOWER_PRIORITY_TERRAIN | DFF_CLEAR_OTHER_TERRAIN))) {
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                if (blockingMap[i][j]) {
                    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                        if (layer != feat->layer && layer != GAS) {
                            if (feat->flags & DFF_CLEAR_LOWER_PRIORITY_TERRAIN) {
                                if (tileCatalog[pmap[i][j].layers[layer]].drawPriority <= tileCatalog[feat->tile].drawPriority) {
                                    continue;
                                }
                            }
                            pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                        }
                    }
                }
            }
        }
    }

    if (succeeded) {
        if ((feat->flags & DFF_AGGRAVATES_MONSTERS) && feat->effectRadius) {
            aggravateMonsters(feat->effectRadius, x, y, &gray);
        }
        if (refreshCell && feat->flashColor && feat->effectRadius) {
            colorFlash(feat->flashColor, 0, (IN_FIELD_OF_VIEW | CLAIRVOYANT_VISIBLE), 4, feat->effectRadius, x, y);
        }
        if (refreshCell && feat->lightFlare) {
            createFlare(x, y, feat->lightFlare);
        }
    }

    if (refreshCell
        && (tileCatalog[feat->tile].flags & (T_IS_FIRE | T_AUTO_DESCENT))
        && cellHasTerrainFlag(player.loc, (T_IS_FIRE | T_AUTO_DESCENT))) {

        applyInstantTileEffectsToCreature(&player);
    }
    if (rogue.gameHasEnded) {
        return succeeded;
    }
    //  if (succeeded && feat->description[0] && !feat->messageDisplayed && playerCanSee(x, y)) {
    //      feat->messageDisplayed = true;
    //      message(feat->description, 0);
    //  }
    if (succeeded) {
        if (feat->subsequentDF) {
            if (feat->flags & DFF_SUBSEQ_EVERYWHERE) {
                for (i=0; i<DCOLS; i++) {
                    for (j=0; j<DROWS; j++) {
                        if (blockingMap[i][j]) {
                            spawnDungeonFeature(i, j, &dungeonFeatureCatalog[feat->subsequentDF], refreshCell, abortIfBlocking);
                        }
                    }
                }
            } else {
                spawnDungeonFeature(x, y, &dungeonFeatureCatalog[feat->subsequentDF], refreshCell, abortIfBlocking);
            }
        }
        if (feat->tile
            && (tileCatalog[feat->tile].flags & (T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_AUTO_DESCENT))) {

            rogue.updatedMapToShoreThisTurn = false;
        }

        // awaken dormant creatures?
        if (feat->flags & DFF_ACTIVATE_DORMANT_MONSTER) {
            for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if (monst->loc.x == x && monst->loc.y == y || blockingMap[monst->loc.x][monst->loc.y]) {
                    // found it!
                    toggleMonsterDormancy(monst);
                }
            }
        }
    }
    return succeeded;
}

void restoreMonster(creature *monst, short **mapToStairs, short **mapToPit) {
    short i, *x, *y, turnCount;
    boolean foundLeader = false;
    short **theMap;
    enum directions dir;

    x = &(monst->loc.x);
    y = &(monst->loc.y);

    if (monst->status[STATUS_ENTERS_LEVEL_IN] > 0) {
        if (monst->bookkeepingFlags & (MB_APPROACHING_PIT)) {
            theMap = mapToPit;
        } else {
            theMap = mapToStairs;
        }

        pmap[*x][*y].flags &= ~HAS_MONSTER;
        if (theMap) {
            // STATUS_ENTERS_LEVEL_IN accounts for monster speed; convert back to map distance and subtract from distance to stairs
            turnCount = (theMap[monst->loc.x][monst->loc.y] - (monst->status[STATUS_ENTERS_LEVEL_IN] * 100 / monst->movementSpeed));
            for (i=0; i < turnCount; i++) {
                if ((dir = nextStep(theMap, monst->loc, NULL, true)) != NO_DIRECTION) {
                    monst->loc.x += nbDirs[dir][0];
                    monst->loc.y += nbDirs[dir][1];
                } else {
                    break;
                }
            }
        }
        monst->bookkeepingFlags |= MB_PREPLACED;
    }

    if ((pmap[*x][*y].flags & (HAS_PLAYER | HAS_STAIRS))
        || (monst->bookkeepingFlags & MB_PREPLACED)) {

        if (!(monst->bookkeepingFlags & MB_PREPLACED)) {
            // (If if it's preplaced, it won't have set the HAS_MONSTER flag in the first place,
            // so clearing it might screw up an existing monster.)
            pmap[*x][*y].flags &= ~HAS_MONSTER;
        }

        pos qualifiedPosition = getQualifyingPathLocNear((pos){ *x, *y }, true, T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)), 0,
                                 avoidedFlagsForMonster(&(monst->info)), (HAS_MONSTER | HAS_PLAYER | HAS_STAIRS | IS_IN_MACHINE), true);
        *x = qualifiedPosition.x;
        *y = qualifiedPosition.y;
    }
    pmap[*x][*y].flags |= HAS_MONSTER;
    monst->bookkeepingFlags &= ~(MB_PREPLACED | MB_APPROACHING_DOWNSTAIRS | MB_APPROACHING_UPSTAIRS | MB_APPROACHING_PIT | MB_ABSORBING);
    monst->status[STATUS_ENTERS_LEVEL_IN] = 0;
    monst->corpseAbsorptionCounter = 0;

    if ((monst->bookkeepingFlags & MB_SUBMERGED) && !cellHasTMFlag((pos){ *x, *y }, TM_ALLOWS_SUBMERGING)) {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }

    if (monst->bookkeepingFlags & MB_FOLLOWER) {
        // is the leader on the same level?
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *leader = nextCreature(&it);
            if (leader == monst->leader) {
                foundLeader = true;
                break;
            }
        }
        // if not, it is time to spread your wings and fly solo
        if (!foundLeader) {
            monst->bookkeepingFlags &= ~MB_FOLLOWER;
            monst->leader = NULL;
        }
    }
}

void restoreItems(void) {
    item *theItem, *nextItem;
    pos loc;
    item preplaced;
    preplaced.nextItem = NULL;

    // Remove preplaced items from the floor chain
    for (theItem = floorItems->nextItem; theItem != NULL; theItem = nextItem) {
        nextItem = theItem->nextItem;

        if (theItem->flags & ITEM_PREPLACED) {
            theItem->flags &= ~ITEM_PREPLACED;
            removeItemFromChain(theItem, floorItems);
            addItemToChain(theItem, &preplaced);
        }
    }

    // Place items properly
    for (theItem = preplaced.nextItem; theItem != NULL; theItem = nextItem) {
        nextItem = theItem->nextItem;

        // Items can fall into deep water, enclaved lakes, another chasm, even lava!
        getQualifyingLocNear(&loc, theItem->loc, true, 0,
                            (T_OBSTRUCTS_ITEMS),
                            (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), false, false);
        placeItemAt(theItem, loc);
    }
}

// Returns true iff the location is a plain wall, three of the four cardinal neighbors are walls, the remaining cardinal neighbor
// is not a pathing blocker, the two diagonals between the three cardinal walls are also walls, and none of the eight neighbors are in machines.
static boolean validStairLoc(short x, short y) {
    short newX, newY, dir, neighborWallCount;

    if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1 || pmap[x][y].layers[DUNGEON] != WALL) {
        return false;
    }

    for (dir=0; dir< DIRECTION_COUNT; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (pmap[newX][newY].flags & IS_IN_MACHINE) {
            return false;
        }
    }

    neighborWallCount = 0;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];

        if (cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
            // neighbor is a wall
            neighborWallCount++;
        } else {
            // neighbor is not a wall
            if (cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)
                || passableArcCount(newX, newY) >= 2) {
                return false;
            }
            // now check the two diagonals between the walls

            newX = x - nbDirs[dir][0] + nbDirs[dir][1];
            newY = y - nbDirs[dir][1] + nbDirs[dir][0];
            if (!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
                return false;
            }

            newX = x - nbDirs[dir][0] - nbDirs[dir][1];
            newY = y - nbDirs[dir][1] - nbDirs[dir][0];
            if (!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
                return false;
            }
        }
    }
    if (neighborWallCount != 3) {
        return false;
    }
    return true;
}

// The walls on either side become torches. Any adjacent granite then becomes top_wall. All wall neighbors are un-tunnelable.
// Grid is zeroed out within 5 spaces in all directions.
static void prepareForStairs(short x, short y, char grid[DCOLS][DROWS]) {
    short newX, newY, dir;

    // Add torches to either side.
    for (dir=0; dir<4; dir++) {
        if (!cellHasTerrainFlag((pos){ x + nbDirs[dir][0], y + nbDirs[dir][1] }, T_OBSTRUCTS_PASSABILITY)) {
            newX = x - nbDirs[dir][1];
            newY = y - nbDirs[dir][0];
            pmap[newX][newY].layers[DUNGEON] = TORCH_WALL;
            newX = x + nbDirs[dir][1];
            newY = y + nbDirs[dir][0];
            pmap[newX][newY].layers[DUNGEON] = TORCH_WALL;
            break;
        }
    }
    // Expose granite.
    for (dir=0; dir< DIRECTION_COUNT; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (pmap[newX][newY].layers[DUNGEON] == GRANITE) {
            pmap[newX][newY].layers[DUNGEON] = WALL;
        }
        if (cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
            pmap[newX][newY].flags |= IMPREGNABLE;
        }
    }
    // Zero out grid in the vicinity.
    for (newX = max(0, x - 5); newX < min(DCOLS, x + 5); newX++) {
        for (newY = max(0, y - 5); newY < min(DROWS, y + 5); newY++) {
            grid[newX][newY] = false;
        }
    }
}

boolean placeStairs(pos *upStairsLoc) {
    char grid[DCOLS][DROWS];
    short n = rogue.depthLevel - 1;

    for (int i=0; i < DCOLS; i++) {
        for (int j=0; j < DROWS; j++) {
            grid[i][j] = validStairLoc(i, j);
        }
    }

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        hiliteCharGrid(grid, &teal, 100);
        temporaryMessage("Stair location candidates:", REQUIRE_ACKNOWLEDGMENT);
    }

    pos downLoc;
    if (getQualifyingGridLocNear(&downLoc, levels[n].downStairsLoc, grid, false)) {
        prepareForStairs(downLoc.x, downLoc.y, grid);
    } else {
        boolean hasQualifyingLoc = getQualifyingLocNear(&downLoc, levels[n].downStairsLoc, false, 0,
                                                        (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_AUTO_DESCENT | T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_IS_DF_TRAP),
                                                        (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), true, false);
        if (!hasQualifyingLoc) {
            return false;
        }
    }

    if (rogue.depthLevel == gameConst->deepestLevel) {
        pmapAt(downLoc)->layers[DUNGEON] = DUNGEON_PORTAL;
    } else {
        pmapAt(downLoc)->layers[DUNGEON] = DOWN_STAIRS;
    }
    pmapAt(downLoc)->layers[LIQUID]     = NOTHING;
    pmapAt(downLoc)->layers[SURFACE]    = NOTHING;

    if (!levels[n+1].visited) {
        levels[n+1].upStairsLoc = downLoc;
    }
    levels[n].downStairsLoc = downLoc;

    pos upLoc;
    if (getQualifyingGridLocNear(&upLoc, levels[n].upStairsLoc, grid, false)) {
        prepareForStairs(upLoc.x, upLoc.y, grid);
    } else { // Hopefully this never happens.
        boolean hasQualifyingLoc;
        hasQualifyingLoc = getQualifyingLocNear(&upLoc, levels[n].upStairsLoc, false, 0,
                             (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_AUTO_DESCENT | T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_IS_DF_TRAP),
                             (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), true, false);
        if (!hasQualifyingLoc) {
            return false;
        }
    }

    levels[n].upStairsLoc = upLoc;

    if (rogue.depthLevel == 1) {
        pmapAt(upLoc)->layers[DUNGEON] = DUNGEON_EXIT;
    } else {
        pmapAt(upLoc)->layers[DUNGEON] = UP_STAIRS;
    }
    pmapAt(upLoc)->layers[LIQUID] = NOTHING;
    pmapAt(upLoc)->layers[SURFACE] = NOTHING;

    rogue.downLoc = downLoc;
    pmapAt(downLoc)->flags |= HAS_STAIRS;
    rogue.upLoc = upLoc;
    pmapAt(upLoc)->flags |= HAS_STAIRS;

    *upStairsLoc = upLoc;
    return true;
}

// Places the player, monsters, items and stairs.
void initializeLevel(pos upStairsLoc) {
    short i, j, dir;
    short **mapToStairs, **mapToPit;
    char grid[DCOLS][DROWS];

    pos upLoc = upStairsLoc;

    if (!levels[rogue.depthLevel-1].visited) {

        // Run a field of view check from up stairs so that monsters do not spawn within sight of it.
        for (dir=0; dir<4; dir++) {
            pos nextLoc = posNeighborInDirection(upLoc, dir);
            if (isPosInMap(nextLoc) && !cellHasTerrainFlag(nextLoc, T_OBSTRUCTS_PASSABILITY)) {
                upLoc = nextLoc;
                break;
            }
        }
        zeroOutGrid(grid);
        getFOVMask(grid, upLoc.x, upLoc.y, max(DCOLS, DROWS) * FP_FACTOR, (T_OBSTRUCTS_VISION), 0, false);
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                if (grid[i][j]) {
                    pmap[i][j].flags |= IN_FIELD_OF_VIEW;
                }
            }
        }
        populateItems(upLoc);
        populateMonsters();
    }

    // Restore items that fell from the previous depth.
    restoreItems();

    // Restore creatures that fell from the previous depth or that have been pathing toward the stairs.
    mapToStairs = allocGrid();
    fillGrid(mapToStairs, 0);
    mapToPit = allocGrid();
    fillGrid(mapToPit, 0);
    calculateDistances(mapToStairs, player.loc.x, player.loc.y, T_PATHING_BLOCKER, NULL, true, true);
    calculateDistances(mapToPit,
                       levels[rogue.depthLevel - 1].playerExitedVia.x,
                       levels[rogue.depthLevel - 1].playerExitedVia.y,
                       T_PATHING_BLOCKER,
                       NULL,
                       true,
                       true);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        restoreMonster(monst, mapToStairs, mapToPit);
    }
    freeGrid(mapToStairs);
    freeGrid(mapToPit);
}

// fills (*x, *y) with the coordinates of a random cell with
// no creatures, items or stairs and with either a matching liquid and dungeon type
// or at least one layer of type terrainType.
// A dungeon, liquid type of -1 will match anything.
boolean randomMatchingLocation(pos* loc, short dungeonType, short liquidType, short terrainType) {
    short failsafeCount = 0;
    do {
        failsafeCount++;
        loc->x = rand_range(0, DCOLS - 1);
        loc->y = rand_range(0, DROWS - 1);
    } while (failsafeCount < 500 && ((terrainType >= 0 && !cellHasTerrainType(*loc, terrainType))
                                     || (((dungeonType >= 0 && pmapAt(*loc)->layers[DUNGEON] != dungeonType) || (liquidType >= 0 && pmapAt(*loc)->layers[LIQUID] != liquidType)) && terrainType < 0)
                                     || (pmapAt(*loc)->flags & (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS | HAS_ITEM | IS_IN_MACHINE))
                                     || (terrainType < 0 && !(tileCatalog[dungeonType].flags & T_OBSTRUCTS_ITEMS)
                                         && cellHasTerrainFlag(*loc, T_OBSTRUCTS_ITEMS))));
    if (failsafeCount >= 500) {
        return false;
    }
    return true;
}
