# 2-005: Faction Design - Artificer Order

## Current Behavior
No faction cards exist.

## Intended Behavior
Design and implement the Artificer Order faction with:
- Theme: Magical constructs, recycling, efficiency, transformation
- Primary mechanic: Scrapping (self and trade row)
- Secondary mechanic: Deck thinning, card manipulation
- Ally bonus focus: Scrap synergies, free acquisitions
- Fantasy flavor: Golem crafters, arcane engineers, magical workshops

## Suggested Implementation Steps

1. Design 8-12 faction cards with cost curve (1-7 trade):
   ```
   Low cost (1-2): Apprentice, small constructs
   Mid cost (3-4): Journeyman, battle golems
   High cost (5-7): Master artificers, colossi
   ```
2. Design 2-3 faction bases:
   - Workshop (scrap trade row cards)
   - Golem foundry (spawns construct tokens)
   - Arcane forge (transform cards)
3. Create card files in `assets/cards/artificer/`
4. Include strong scrap abilities
5. Focus on deck efficiency and quality over quantity
6. Write flavor text reflecting crafting themes
7. Document faction strategy guide

## Related Documents
- docs/02-game-mechanics.md
- 2-001-card-template-format.md

## Dependencies
- 2-001: Card Template Format (file structure)

## Example Cards

**Construct Apprentice** (Cost 1, Ship)
- Effects: +1 Trade
- Scrap: +2 Combat

**Battle Golem** (Cost 3, Ship)
- Effects: +4 Combat
- Ally: Scrap a card from trade row

**Master Artificer** (Cost 6, Ship)
- Effects: +3 Trade, +3 Combat
- Ally: Acquire a ship costing 4 or less for free
- Scrap: Draw 2 cards

**Golem Foundry** (Cost 4, Base, Defense 5)
- Effects: Spawn Construct Token each turn
- Construct Token: +2 Combat, scrap to gain +3 Trade

**Arcane Workshop** (Cost 3, Base, Defense 4)
- Effects: You may scrap a card from your hand or discard

## Acceptance Criteria
- [ ] 8-12 ship cards created
- [ ] 2-3 base cards created
- [ ] Scrap abilities are central
- [ ] Deck thinning strategy viable
- [ ] Free acquisition balanced
