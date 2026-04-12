# Modern Keyboard Layout (uio/jkl/m,.)

## Overview

Added an alternative keyboard layout for movement that maps directions to their physical positions on a QWERTY keyboard, replacing the traditional vi-style `hjklyubn` keys.

```
Classic (vi):     Modern:
  y k u           u i o
  h . l           j k l
  b j n           m , .
```

Modern layout is the default. Users can switch between layouts via a button on the title screen or the `--classic-keys` CLI flag.

## Key Bindings (Modern Mode)

| Key           | Action                             |
|---------------|------------------------------------|
| `u`           | move up-left                       |
| `i`           | move up                            |
| `o`           | move up-right                      |
| `j`           | move left                          |
| `k`           | rest (wait one turn)               |
| `l`           | move right                         |
| `m`           | move down-left                     |
| `,`           | move down                          |
| `.`           | move down-right                    |
| `f`           | inventory (replaces `i`)           |
| Shift+key     | run in that direction              |
| Ctrl+key      | run in that direction              |
| `K` (Shift+k) | auto-rest (rest until interrupted) |

Arrow keys and numpad work in both modes.

### Blocked Keys in Modern Mode

Old vi movement keys (`h`, `y`, `b`, `n` and their uppercase forms) are blocked in modern mode to prevent accidental movement.

### Special Cases

- `Shift+M` is NOT remapped (reserved for Message Archive `M`). Use `Ctrl+m` for running down-left.
- `Shift+,` produces `<` (ascend stairs) -- unchanged.
- `Shift+.` produces `>` (descend stairs) -- unchanged.

## Title Screen

A "Keys: Modern" / "Keys: Classic" toggle button was added between "View" and "Quit" on the title screen. Hotkey: `K`.

## Persistence

The setting is saved to `BroguePrefs.txt` (same directory as high scores) and loaded on next launch. CLI flag `--classic-keys` overrides the saved preference for that session.

Load order: hardcoded default (modern) -> saved prefs file -> CLI args.

## Files Changed

| File | Changes |
|------|---------|
| `src/brogue/Rogue.h` | Added `modernKeys` field to `playerCharacter` struct; added `NG_MODERN_KEYS` enum; declared `translateModernKeys()` |
| `src/brogue/IO.c` | Implemented `translateModernKeys()` -- maps modern keys to internal constants and blocks old vi-keys; updated help screen text |
| `src/brogue/Items.c` | Added `translateModernKeys()` call in `moveCursor()` for targeting mode |
| `src/brogue/RogueMain.c` | Preserved `modernKeys` across `initializeRogue()` memset (save/restore pattern) |
| `src/brogue/MainMenu.c` | Added "Keys" toggle button to main menu; calls `savePrefs()` on toggle |
| `src/platform/main.c` | Set default `modernKeys = true`; added `--classic-keys` CLI flag; calls `loadPrefs()` at startup |
| `src/platform/platform.h` | Declared `loadPrefs()` and `savePrefs()` |
| `src/platform/platformdependent.c` | Implemented `loadPrefs()` / `savePrefs()` using `BroguePrefs.txt` |

## Implementation Details

### Translation Approach

Rather than changing the switch/case statements throughout the codebase, a `translateModernKeys()` function is called early in the input pipeline (before `stripShiftFromMovementKeystroke()`). It maps raw input to the internal key constants, so all downstream game logic remains unchanged.

### Recording Compatibility

Translation is skipped during `rogue.playbackMode`, so old `.broguerec` recordings (which store internal key constants) replay correctly regardless of the active layout.

### Prefs File Format

`BroguePrefs.txt` uses a simple space-separated key-value format:

```
modernKeys 1
```

New preferences can be added as additional lines in the future.
