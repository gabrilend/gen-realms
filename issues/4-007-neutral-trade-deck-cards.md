# 2-007: Neutral Trade Deck Cards

## Current Behavior
No neutral cards exist for the trade deck.

## Intended Behavior
Create neutral cards that appear in the trade row:
- Explorer equivalent (always available, basic utility)
- 4-6 neutral ships with varied effects
- Provide options for players not committing to factions
- Generally less efficient than faction cards (no ally bonuses)
- Fantasy themed without faction allegiance

## Suggested Implementation Steps

1. Create `assets/cards/neutral/` directory
2. Define Explorer equivalent:
   ```lua
   {
     id = "wandering_merchant",
     name = "Wandering Merchant",
     cost = 2,
     faction = "neutral",
     card_type = "ship",
     effects = {{type = "add_trade", value = 2}},
     scrap_effects = {{type = "add_combat", value = 2}},
     flavor = "No allegiance but to coin."
   }
   ```
3. Design 4-6 varied neutral cards:
   - Combat focused
   - Trade focused
   - Hybrid
   - Utility (draw, scrap)
4. Create card files
5. Integrate with trade deck building
6. Ensure Explorer always available mechanic

## Related Documents
- docs/02-game-mechanics.md
- 1-004-trade-row-implementation.md

## Dependencies
- 2-001: Card Template Format

## Example Cards

**Wandering Merchant** (Cost 2, always available)
- Effects: +2 Trade
- Scrap: +2 Combat

**Sellsword** (Cost 3)
- Effects: +3 Combat
- Scrap: +3 Trade

**Traveling Scholar** (Cost 4)
- Effects: +2 Trade, Draw a card

**Fortune Seeker** (Cost 5)
- Effects: +3 Trade, +3 Combat

## Acceptance Criteria
- [ ] Wandering Merchant always available
- [ ] 4-6 neutral trade deck cards
- [ ] Neutral cards in trade deck rotation
- [ ] Effects varied and useful
- [ ] Less efficient than faction-committed play
