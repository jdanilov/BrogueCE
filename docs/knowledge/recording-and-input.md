# BrogueCE Recording & Input System

## Main Input Loop (`IO.c:537-833`)

Core game loop in `mainInputLoop()`:
1. Calculate pathfinding via Dijkstra for movement
2. Manage cursor position for targeting
3. Acquire input via `moveCursor()` → `nextBrogueEvent()`
4. **During playback**: `executePlaybackInput()` processes recorded events
5. **Normal play**: `executeEvent()` dispatches to handlers
6. Execute path-following or targeted actions

## Input Events

### rogueEvent Structure (`Rogue.h:371-377`)
```c
typedef struct rogueEvent {
    enum eventTypes eventType;  // KEYSTROKE, MOUSE_UP/DOWN, RNG_CHECK, etc.
    signed long param1;         // Key code or X coordinate
    signed long param2;         // Y coordinate (mouse events)
    boolean controlKey;         // Ctrl modifier
    boolean shiftKey;           // Shift modifier
} rogueEvent;
```

### Event Types (`Rogue.h:349-361`)
KEYSTROKE, MOUSE_UP, MOUSE_DOWN, RIGHT_MOUSE_DOWN, RIGHT_MOUSE_UP, MOUSE_ENTERED_CELL, RNG_CHECK, SAVED_GAME_LOADED, END_OF_RECORDING, EVENT_ERROR

## Action Processing

### `executeKeystroke()` (`IO.c:2451-2718`)

**Movement**: `hjklyubn` / arrow keys / numpad → `playerMoves()` or `playerRuns()` (with shift/ctrl)

**Stairs**: `<` ascend, `>` descend → `useStairs(-1/+1)`

**Rest/Search**: `z` rest one turn, `Z` auto-rest until healed, `s` search (ctrl+s auto-search)

**Inventory**: `i` view, `e` equip, `u` unequip, `d` drop, `a` apply/use, `t` throw, `r` relabel, `w` swap

**Automation**: `x` auto-explore, `A` auto-play level

**Interface**: `?` help, `D` discoveries, `M` message archive, `\` true colors, `]` stealth range

**Game**: `S` save, `N` new game, `Q` quit, `G` cycle graphics

## Recording System

### File Format (`Recordings.c:177-221`)

**Header (36 bytes):**

| Bytes | Content                                |
|-------|----------------------------------------|
| 0-15  | Version string (15 chars + null)       |
| 16    | Game mode (normal/easy/wizard)         |
| 17-24 | Master RNG seed (uint64_t, 8 bytes)    |
| 25-28 | Player turn count (4 bytes)            |
| 29-32 | Deepest level reached (4 bytes)        |
| 33-36 | File length including header (4 bytes) |

**Event Encoding (`Recordings.c:100-130`):**

Keystroke events (3 bytes):
1. Event type byte (0 = KEYSTROKE)
2. Compressed keystroke code (ASCII or 128+index for special keys)
3. Modifier flags (bit 0: ctrl, bit 1: shift)

Mouse events (4 bytes):
1. Event type byte
2. X coordinate
3. Y coordinate
4. Modifier flags

### Keystroke Compression (`Recordings.c:32-36`)
Special keys (arrows, escape, return, delete, tab, numpad 0-9) stored as `128 + table_index`. ASCII keys stored directly.

### Buffer Management
- `inputRecordBuffer[1100]` — write buffer
- Flushes to file at 1000 bytes (`flushBufferToFile()`, lines 223-249)
- Append mode ("ab") with header length update

### Recording Functions
- `recordKeystroke()` (133) — convenience wrapper
- `recordMouseClick()` (162) — mouse events
- `cancelKeystroke()` (147) — undo last (for prediction rollback)
- `recordKeystrokeSequence()` (154) — record string

## Playback System

### Initialization (`initRecording()`, `Recordings.c:465-555`)
1. Load header from file
2. Validate version compatibility
3. Read seed, turn count, deepest level, file length
4. Reseed RNG with loaded seed
5. Load annotation file if exists

### Event Retrieval (`recallEvent()`, `Recordings.c:340-383`)
Reads events from buffer (reverse of recording format):
- Keystroke: decompress code + modifiers
- Mouse: X, Y + modifiers
- Special: SAVED_GAME_LOADED, RNG_CHECK, END_OF_RECORDING

### Playback Controls (`executePlaybackInput()`, `Recordings.c:831-1102`)

| Key       | Action                         |
|-----------|--------------------------------|
| `k`/UP    | Faster (reduce delay by 1/3)   |
| `j`/DOWN  | Slower (increase delay by 1/2) |
| SPACE     | Pause/unpause                  |
| `<`/`>`   | Jump to prev/next depth        |
| `h`/LEFT  | Rewind 1-20 turns              |
| `l`/RIGHT | Advance 1-20 turns             |
| `0-9`     | Jump to specific turn          |
| TAB       | Toggle omniscience             |
| `i`       | Show inventory                 |

### Seeking (`seek()`, `Recordings.c:688-780`)
- Rewind requires `resetPlayback()` (restart from beginning)
- Fast-forward loops through events until destination
- Progress bar for seeks > 100 turns

## Random Seed Management

### Deterministic Replay
- Master seed stored in `rogue.seed` (uint64_t)
- Each level gets per-seed: `levels[i].levelSeed` (`RogueMain.c:261-268`)
- Recording stores seed in header for replay
- `seedRandomGenerator()` initializes RNG state from seed

### Out-of-Sync Detection (`Recordings.c:557-596`)
- `RNGCheck()` called once per player turn
- Records/verifies RNG output periodically
- Mismatch triggers "Out of sync" error + pause
- Optional AUDIT_RNG mode logs all RNG calls

### Cosmetic RNG
Separate `RNG_COSMETIC` stream for visual effects — doesn't affect recording determinism.

## Turn Counting

| Counter              | Increments                                | Use                                |
|----------------------|-------------------------------------------|------------------------------------|
| `playerTurnNumber`   | Each player action (not during paralysis) | Playback seeking, recording header |
| `absoluteTurnNumber` | Every turn regardless                     | Internal timing                    |
| `scentTurnNumber`    | Each turn                                 | Scent map timing                   |

Initialized in `initializeRogue()` (`RogueMain.c:376-380`):
```c
rogue.scentTurnNumber = 1000;
rogue.playerTurnNumber = 0;
rogue.absoluteTurnNumber = 0;
```

## What Breaks Recordings

### MUST NOT CHANGE
- Master seed format (64-bit in header)
- `playerTurnNumber` increment logic
- RNG sequence (any change to `rand_range()`, `rand_64bits()`)
- Event encoding format (3-4 bytes per event)
- Header format (36-byte structure)
- Dungeon generation algorithm
- Turn processing order (monsters → environment → vision)
- Input-to-action mapping

### Safe to Change
- UI improvements not affecting gameplay
- Graphics mode additions
- Message text changes
- New features that don't interact with recorded state
- Cosmetic RNG usage (separate stream)

## Auto-Rest & Auto-Explore

### Auto-Rest (`autoRest()`, `Time.c:2087-2144`)
Rests until: HP full, all bad statuses cleared, not trapped. Stops on danger (monster appears). Records REST_KEY per turn.

### Auto-Play (`autoPlayLevel()`, `Movement.c:2052-2074`)
Full automation: `explore()` repeatedly → auto-descend when exploration complete. 1ms (fast-forward) or 50ms (normal) delay. Stops on user input.

### Explore (`Movement.c:1950`)
Pathfinding to undiscovered areas. Returns true if progress made. Records individual movement keystrokes.

## Key Files

| File | Key Functions |
|------|--------------|
| `IO.c` | `mainInputLoop()` (537), `executeKeystroke()` (2451), `nextBrogueEvent()` (2390) |
| `Recordings.c` | `recordEvent()` (100), `recallEvent()` (340), `initRecording()` (465), `executePlaybackInput()` (831), `seek()` (688), `RNGCheck()` (581) |
| `RogueMain.c` | `initializeRogue()` (190), `executeEvent()` (45) |
| `Rogue.h` | `rogueEvent` (371), event types (349), key bindings (1174-1236) |
| `Time.c` | `autoRest()` (2087) |
| `Movement.c` | `autoPlayLevel()` (2052), `explore()` (1950) |
