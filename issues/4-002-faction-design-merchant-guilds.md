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

### Addendum (2026-02-10)
Added **Golden Vault** (Cost 5) to bring faction to 15 cards total.
