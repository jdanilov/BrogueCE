# Main Menu Refactor

## Overview

Replaced the large ASCII-art "BROGUE" title with a compact decorative glyph, dimmed the background flame animation, and repositioned the title label. The version string was removed from the bottom-right corner of the menu screen in favor of a small "BROGUE" label rendered above the menu buttons.

## Before vs After

### Before

- The title art was a 68x26 character block spelling out "BROGUE" in large block-letter ASCII art (with variant names like "RAPID" or "BULLET" above it for non-standard variants).
- The title was wreathed in flame-colored sources and masked in black with anti-aliased edges blending into the fire.
- The version string (e.g. "CE 1.14.0") was displayed at the bottom-right corner of the screen.
- Flame animation was bright and fast: rise speed 50, spread speed 20, fade speed 20.

### After

- The title art is a 26x11 character compact ornamental glyph (a stylized gem/flame shape using special characters like `$`, `^`, `%`, `8`).
- Title characters are rendered in a warm amber/gold color directly, with no flame sources behind the title and no anti-aliasing pass.
- The version string no longer appears on screen. Instead, a static "BROGUE" text label is rendered in a dimmer amber color at a fixed position near the right side of the screen (`COLS - 14`, `ROWS - 12`), above the menu buttons.
- Flame animation is slower and dimmer: rise speed 15, spread speed 8, fade speed 80. The flame source colors themselves are also significantly dimmed (from `{20,7,7,60,40,40}` to `{8,3,2,20,10,8}`).

## Files Changed

| File | Changes |
|------|---------|
| `src/brogue/GlobalsBase.h` | Reduced `MENU_TITLE_WIDTH` from 68 to 26 and `MENU_TITLE_HEIGHT` from 26 to 11 to match the new compact title art. |
| `src/brogue/Globals.c` | Dimmed `flameSourceColor` and `flameSourceColorSecondary` to produce a subtler ember glow on the menu background. |
| `src/brogue/MainMenu.c` | Lowered flame rise/spread speeds and increased fade speed for calmer animation. Added `titleTextColor` (amber) and `titleLabelColor` (dim amber) constants. Changed `drawMenuFlames()` to render title characters in amber instead of masking them in black. Added a "BROGUE" label rendered at a fixed position. Removed `antiAlias()` function entirely. Removed flame color sources behind title characters in `initializeMenuFlames()`. Changed title offset from `(-7, -2)` to `(0, -5)`. Removed version string rendering from the bottom-right corner. |
| `src/variants/GlobalsBrogue.c` | Replaced the 68x26 block-letter "BROGUE" ASCII art with an 11-line decorative glyph. |
| `src/variants/GlobalsRapidBrogue.c` | Same title art replacement. The "RAPID" sub-header above the old block letters is gone. |
| `src/variants/GlobalsBulletBrogue.c` | Same title art replacement. The "BULLET" sub-header above the old block letters is gone. |

## Implementation Details

### Title Rendering

Previously, title characters were used as flame color sources (using `flameTitleColor`) and then masked over in black with anti-aliased edges, creating a flame-wreathed silhouette effect. Now, the mask is still set for title character positions, but `drawMenuFlames()` looks up the actual character from the title art and renders it directly in `titleTextColor` (amber), producing visible stylized text rather than a silhouette.

### Anti-Aliasing Removal

The `antiAlias()` function, which smoothed the edges of the title mask by averaging neighboring cell values, was removed entirely. With the new approach of rendering visible characters instead of a black mask, edge smoothing is no longer needed.

### Variant Title Differentiation

All three variants (Brogue, Rapid Brogue, Bullet Brogue) now use the same style of compact glyph art, differing only in minor spacing. The old approach had variant names ("RAPID", "BULLET") rendered as additional block text above the main "BROGUE" title; these are gone. Variant identification now relies on the game mode string shown in the bottom-left corner (for non-standard variants) and the "BROGUE" label.
