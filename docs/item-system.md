# BrogueCE Item System

## Item Structure (`Rogue.h:1410-1435`)

Key fields:
- `category` — WEAPON, ARMOR, POTION, SCROLL, etc. (bitmask)
- `kind` — specific type within category
- `flags` — ITEM_IDENTIFIED, ITEM_CURSED, ITEM_EQUIPPED, ITEM_RUNIC, etc.
- `damage` — `{lowerBound, upperBound, clumpFactor}` randomRange
- `armor` — defense value
- `charges` — staff/wand charges, or auto-ID counter for weapons/armor
- `enchant1` — primary enchantment level
- `enchant2` — runic type (if ITEM_RUNIC) or staff recharge delay
- `timesEnchanted` — scroll of enchanting count
- `vorpalEnemy` — target monster type for slaying/immunity runics
- `strengthRequired` — minimum strength for effective use
- `quantity` — stack count (throwing weapons, gold)

## Item Categories (`Rogue.h:761-789`)

| Category | Kinds | Notes |
|----------|-------|-------|
| **FOOD** | Ration, Fruit | Always identified |
| **WEAPON** | 16 kinds (Dagger→War Axe, Dart, Incendiary Dart, Javelin) | Throwing weapons stack |
| **ARMOR** | 6 kinds (Leather→Plate Mail) | Single equip slot |
| **POTION** | 17 kinds | Unidentified until used or scroll of ID |
| **SCROLL** | 14 kinds | Unidentified, flammable |
| **STAFF** | 13 kinds | Charges, rechargeable |
| **WAND** | 9 kinds | Limited charges, not rechargeable |
| **RING** | 8 kinds | Two equip slots, continuous effect |
| **CHARM** | 12 kinds | Rechargeable cooldown effects |
| **GOLD** | — | Auto-stacks, scales with depth |
| **AMULET** | 1 | Win condition |
| **GEM** | Lumenstones | Post-amulet score collectibles |
| **KEY** | Door, Cage, Portal | Machine puzzle keys |

**Category masks:**
- `CAN_BE_ENCHANTED`: WEAPON, ARMOR, RING, CHARM, WAND, STAFF
- `HAS_INTRINSIC_POLARITY`: POTION, SCROLL, RING, WAND, STAFF
- `PRENAMED_CATEGORY`: FOOD, GOLD, AMULET, GEM, KEY (always identified by name)

## Item Generation (`Items.c:75-419`)

### Flow
1. `generateItem(category, kind)` → `initializeItem()` → `makeItemInto()`
2. Category selected via `pickItemCategory()` using weighted `itemGenerationProbabilities[]`
3. Kind selected via `chooseKind()` using per-kind `frequency` weights in itemTable

### Generation Details by Category

**Weapons** (lines 201-276):
- 40% chance enchanted (+1 to +3)
- 50% of enchanted are cursed (negative enchant)
- 33% of cursed get bad runic
- Positive weapons: chance for good runic based on damage
- Throwing weapons: quantity 3-18, no enchant/runic/curse

**Armor** (lines 278-308):
- Same 40% enchant / 50% curse / 33% bad runic pattern
- Positive armor: chance for good runic if `armor < rand(0, 95)`

**Rings** (lines 347-363):
- `enchant1 = randClump(range)`, then +1 per 10% chance
- 16% chance cursed (negative enchant)

**Staffs** (lines 322-339):
- Charges: 2 base + cascading 50%/15%/10% bonuses
- `enchant1 = charges`, `enchant2 = recharge delay` (500 or 1000)

**Wands** (lines 340-346):
- Charges from `randClump(wandTable[kind].range)`

**Charms** (lines 364-374):
- Always identified, charges start at 0 (immediately ready)

**Gold** (lines 375-378):
- `quantity = rand(50 + depth*10*accel, 100 + depth*15*accel)`

## Identification System

### Item Flags
- `ITEM_IDENTIFIED` — fully revealed
- `ITEM_CAN_BE_IDENTIFIED` — eligible for scroll of identify
- `ITEM_RUNIC_HINTED` / `ITEM_RUNIC_IDENTIFIED` — runic discovery stages
- `ITEM_MAGIC_DETECTED` — detected by Detect Magic potion

### Identification Methods
1. **Scroll of Identify** — player selects item, calls `identify()` (`Items.c:6845`)
2. **Usage-based auto-ID**:
   - Weapons: after 20 kills (charges field counts down)
   - Armor: after equipped for N turns (armorDelayToAutoID)
   - Rings: after N turns equipped (ringDelayToAutoID)
3. **Category-wide ID**: Once a potion/scroll/wand kind is identified, all of that kind are known
4. **Last-of-category**: Auto-identifies when only one unidentified kind remains
5. **Detect Magic potion**: Reveals magic polarity (good/bad/neutral) but not specifics

### Unidentified Display
- Potions/Scrolls: color/label flavor text ("a bubbly potion", "a scroll labeled...")
- Weapons/Armor: show damage/armor range estimates
- Runics: hinted first ("a\"runic\" broadsword"), then fully named

## Enchantment System

### Scroll of Enchanting Effects (`Items.c:7025-7087`)
| Category | Effect |
|----------|--------|
| WEAPON | `enchant1 += 1`, `strengthRequired -= 1` |
| ARMOR | `enchant1 += 1` |
| RING | `enchant1 += 1` |
| STAFF | `charges += 1`, `enchant1 = charges` |
| WAND | `charges += 1` |
| CHARM | `enchant1 += 1` |

All increment `timesEnchanted`.

### How Enchantment Scales
- Weapons: affects damage (1.065^enchant) and accuracy (same curve)
- Armor: +10 defense per enchant point
- Rings: effect magnitude scales linearly
- Staffs: more charges = more uses
- Charms: reduced cooldown per enchant

## Cursed Items

- Generated with negative `enchant1` + `ITEM_CURSED` flag
- Cannot be unequipped until `Scroll of Remove Curse` used
- `uncurse()` (`Items.c:6949`) removes flag from all pack items
- `ITEM_PROTECTED` flag (via Scroll of Protect) prevents future cursing
- Bad runics (W_PLENTY, A_VULNERABILITY, A_IMMOLATION) only appear on cursed items

## Equipment Slots

- `rogue.weapon` — one weapon
- `rogue.armor` — one armor
- `rogue.ringLeft`, `rogue.ringRight` — two rings (stacking allowed)
- Equipping calls `recalculateEquipmentBonuses()` to update player stats
- Cursed items block unequipping

## Item Usage (`apply()`, `Items.c:6793-6830`)

| Category | Function | Key Effects |
|----------|----------|-------------|
| FOOD | `eat()` | Restores nutrition |
| POTION | `drinkPotion()` | Life (full heal + max HP), Strength, Telepathy, Levitation, Haste, Fire Immunity, Invisibility; bad: Poison gas, Paralysis gas, Hallucination, Confusion, Incineration, Darkness, Descent |
| SCROLL | `readScroll()` | Enchanting, Identify, Teleport, Remove Curse, Recharging, Protect Armor/Weapon, Sanctuary, Magic Mapping, Negation, Shattering, Discord; bad: Aggravate, Summon Monster |
| STAFF | `staffZap()` | Consumes charge, fires bolt |
| WAND | `wandZap()` | Consumes charge, fires bolt |
| CHARM | `activateCharm()` | Cooldown-based effect, recharge scales with enchant |

Bad potions/scrolls prompt for confirmation if already identified.

## Inventory Management

- **Pack limit**: 26 items (letters a-z)
- **Stacking**: Gold always combines; potions/scrolls/wands stack by kind; weapons/gems don't stack
- **Item counting**: Weapons and gems count as 1 regardless of quantity; others count by quantity field

## Item Placement

- `placeItemAt()` (`Items.c:422-461`): Places on map or random floor tile
- During generation: items spawned via `generateItem()` and placed at machine/feature locations
- Depth-scaled: metered generation tables control per-level item counts
- Key items tracked via `keyLocationProfile` for machine puzzles

## Key Files

| File | Key Functions |
|------|--------------|
| `Items.c` | `generateItem()` (75), `makeItemInto()` (179), `chooseKind()` (409), `placeItemAt()` (422), `identify()` (6845), `readScroll()` (6957), `drinkPotion()` (7239), `apply()` (6793), `equipItem()` (7718) |
| `Rogue.h` | item struct (1410), itemTable struct (1437), category enums (761), kind enums (804-900), item flags (1373) |
| `Globals.c` | weaponTable, armorTable, foodTable, staffTable, ringTable (shared tables) |
| `GlobalsBrogue.c` | potionTable, scrollTable, wandTable, charmTable, itemGenerationProbabilities (variant-specific) |
| `Combat.c` | `netEnchant()` (69) |
