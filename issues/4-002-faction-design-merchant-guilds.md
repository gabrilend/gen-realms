# 4-002: Faction Design - Merchant Guilds

## Current Behavior
No faction cards exist.

## Intended Behavior
Design and implement the Merchant Guilds faction with:
- Theme: Trade, commerce, wealth accumulation, economic warfare
- Primary mechanic: Trade generation, card acquisition
- Secondary mechanic: Authority gain (prosperity)
- Ally bonus focus: Extra trade or card draw
- Fantasy flavor: Guild halls, caravans, merchant princes, gold magic

## Suggested Implementation Steps

1. Design 8-12 faction cards with cost curve (1-7 trade):
   ```
   Low cost (1-2): Basic traders, couriers
   Mid cost (3-4): Guild agents, trade ships
   High cost (5-7): Guild masters, merchant princes
   ```
2. Design 2-3 faction bases:
   - Trading post (generates trade)
   - Guild hall (ally bonuses)
   - Market fortress (outpost with trade)
3. Create card files in `assets/cards/merchant/`
4. Include at least one card with spawning (merchant spawns gold tokens)
5. Balance trade generation vs other factions
6. Write flavor text reflecting fantasy merchant theme
7. Document faction strategy guide

## Related Documents
- docs/02-game-mechanics.md
- 4-001-card-json-schema.md

## Dependencies
- 4-001: Card JSON Schema

## Acceptance Criteria
- [x] 8-12 ship cards created
- [x] 2-3 base cards created
- [x] Cost curve is balanced
- [x] Ally abilities synergize within faction
- [x] Flavor text is consistent with theme

## Completion Notes (2026-02-10)

**Status: COMPLETED**

### Cards Created (15 total)

**Ships (11):**
| Card | Cost | Effects | Ally Effects |
|------|------|---------|--------------|
| Guild Courier | 1 | +2 Trade | Draw 1 |
| Coin Changer | 1 | +1 Trade, +1 Authority | +1 Trade |
| Trade Caravan | 3 | +3 Trade | - (Scrap: +2 Trade) |
| Guild Factor | 3 | +2 Trade | +2 Trade |
| Trade Galleon | 4 | +4 Trade | +2 Authority |
| Guild Enforcer | 4 | +2 Trade, +3 Combat | +1 Trade |
| Golden Vault | 5 | +2 Trade, +3 Authority | +2 Trade, Acquire ≤2 free |
| Gold Mage | 5 | +3 Trade, Draw 1 | +2 Trade |
| Guild Treasury | 5 | +3 Trade, +2 Authority | Acquire ≤3 free |
| Merchant Prince | 6 | +5 Trade, +3 Authority | Draw 1, +2 Authority |
| Master of Coin | 7 | +4 Trade, Draw 2 | +3 Trade |

**Bases (3):**
| Card | Cost | Defense | Effects |
|------|------|---------|---------|
| Trading Post | 3 | 4 | +2 Trade |
| Guild Hall | 4 | 5 | +1 Trade (Ally: +2 Trade, Draw 1) |
| Market Fortress | 6 | 6 (Outpost) | +2 Trade, +2 Authority, Spawns Gold Token |

**Token (1):**
- Gold Tribute: +1 Trade (Scrap: +2 Authority)

### Addendum 1 (2026-02-10)
Added **Golden Vault** (Cost 5) to bring faction to 15 cards total.

### Addendum 2 (2026-02-10) - Kingdom Coin & Forecast System

**Major faction expansion with two new mechanics:**

#### Kingdom Coin (Secondary Currency)
A persistent currency (doesn't reset each turn) for hiring mercenaries.

**Lore:** Castle-stamped coin made from high-melting-point tungsten alloy.
Too hot to de-mint, too common to covet—it just... is. Used for inter-guild
trade when regular currency won't do.

**New Coin-Generating Cards:**
| Card | Cost | Def | Coin | Notes |
|------|------|-----|------|-------|
| Royal Coin Mint | 3 | 4 | +1 (ally: +1) | Core coin generator |
| Exchange House | 5 | 5 | +1 | Also has forecast: 5 |
| Guild Observatory | 7 | 6 | +2 | Also has forecast: 8 |

**Spending Coin (Requires Merchant Ally):**
Coins persist between turns, but spending requires a Merchant ally effect.

| Card | Ally Effect | Coin Cost | Reward |
|------|-------------|-----------|--------|
| Guild Courier | spend_coin | 1 | +1 Combat |
| Guild Factor | spend_coin | 2 | Spawn Coin Guard |
| Gold Mage | spend_coin | 3 | Spawn Coin Scout |
| Merchant Prince | spend_coin | 5 | Spawn Coin Captain |

**Coin Troops (Spawned Units):**
| Unit | Effects | Notes |
|------|---------|-------|
| Coin Guard | Draw 1, +1 Combat | Spawned for 2 coin |
| Coin Scout | Draw 1, +1 Trade | Spawned for 3 coin |
| Coin Captain | Draw 1, +2 Combat, +1 Authority | Spawned for 5 coin |

All Coin Troops have `auto_draw: true` - their draw effects resolve at
turn start, before draw order selection. Like Survey Ship from Star Realms,
they permanently improve deck quality with small consistent effects.

#### Forecast (Information Advantage)
High-level bases reveal how many draw effects exist in your top N cards.

**How It Works:**
- At turn start, you learn: "Top 5 cards contain 2 draw effects"
- You don't know which cards, just how many bonus draws await
- Helps plan turn strategy and card purchases

**Schema Additions:**
- `add_coin` effect type
- `coin_cost` property (alternative to trade cost)
- `auto_draw` property (pre-turn draw resolution)
- `forecast` property (reveal draw count on top N cards)

**Design Intent:**
Merchants trade in information and compound growth. Kingdom Coin creates
a secondary economy. Forecast provides planning advantage. Auto-draw Coin
Troops are like hiring permanent staff—small effects that add up over time.
