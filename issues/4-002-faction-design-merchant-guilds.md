# 2-002: Faction Design - Merchant Guilds

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
- 2-001-card-template-format.md

## Dependencies
- 2-001: Card Template Format (file structure)

## Example Cards

**Guild Courier** (Cost 1, Ship)
- Effects: +2 Trade
- Ally: Draw a card

**Trade Caravan** (Cost 3, Ship)
- Effects: +3 Trade
- Scrap: +2 Trade

**Merchant Prince** (Cost 6, Ship)
- Effects: +5 Trade, +3 Authority
- Ally: Draw a card, +2 Authority

**Trading Post** (Cost 3, Base, Defense 4)
- Effects: +2 Trade each turn

## Acceptance Criteria
- [ ] 8-12 ship cards created
- [ ] 2-3 base cards created
- [ ] Cost curve is balanced
- [ ] Ally abilities synergize within faction
- [ ] Flavor text is consistent with theme
