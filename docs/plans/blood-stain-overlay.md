# Plan: Blood Stain Overlay

## Goal
Add a blood stain tinting system so that blood splatters visually paint walls and vegetation with the blood's color, rather than being hidden behind them.

## Context
Blood currently only appears on the SURFACE layer (drawPriority 80). Walls (DUNGEON layer, priority 0) and vegetation (SURFACE, priority 45-60) have higher visual priority, so blood is invisible on those cells. The gas overlay system in `getCellAppearance()` provides a precedent for volume-based color tinting across layers.

Directional blood splatter (Combat.c:1763) currently skips cells with `T_OBSTRUCTS_PASSABILITY`, so walls never receive blood. This check needs to be relaxed to allow staining walls without placing actual blood terrain.

## Acceptance Criteria
- [ ] Blood splatters tint wall foreground/background colors red (or appropriate blood color)
- [ ] Blood splatters tint vegetation foreground/background colors with the blood color
- [ ] Different blood types (red, green, purple, worm, acid) each tint with their respective color
- [ ] Liquid tiles (water, lava, etc.) are NOT stained — blood dissipates in liquid
- [ ] Gas overlay takes priority over blood stain overlay — if gas is present, its tinting wins
- [ ] Stains are permanent (persist until terrain changes)
- [ ] Stains are saved/restored with level data
- [ ] Existing tests pass (`make test`)
- [ ] Determinism is preserved (seed catalogs still match)

## Tasks
- [ ] **Add `bloodStainColor` field to `pcell`** in Rogue.h — a `color` struct (or index + weight) to track stain color per cell
- [ ] **Record blood stain when blood spawns** — In `spawnDungeonFeature()` or the blood-spawning code in Combat.c, set `bloodStainColor` on the target cell regardless of whether the SURFACE tile was actually placed
- [ ] **Allow directional splatter onto walls** — Relax the `T_OBSTRUCTS_PASSABILITY` check in Combat.c to still record blood stains on walls (without placing blood terrain)
- [ ] **Render blood stain overlay in `getCellAppearance()`** — After normal layer color resolution, blend `bloodStainColor` onto `cellForeColor`/`cellBackColor` using `applyColorAverage()`, similar to gas overlay. Skip stain rendering if the cell has a LIQUID layer tile. If gas is present, apply gas overlay AFTER blood stain so gas tinting takes priority
- [ ] **Skip staining on liquid tiles** — Do not record blood stains on cells that have a LIQUID layer tile (water, lava, etc.), and clear existing stains if liquid floods a cell
- [ ] **Clear stains on terrain change** — When a cell's terrain is replaced (e.g., wall destroyed, vegetation burned, liquid floods in), clear the stain
- [ ] **Save/load stain data** — Ensure `bloodStainColor` is included in level serialization (levelData save/restore)
- [ ] **Update remembered appearance** — Stained cells should be remembered with their stained colors
- [ ] **Verify tests and determinism** — Run `make test` and seed catalog checks

## Ask User
(empty)

## Critic Findings
(empty)
