# 2-004: Faction Design - High Kingdom

## Current Behavior
No faction cards exist.

## Intended Behavior
Design and implement the High Kingdom faction with:
- Theme: Noble authority, order, defensive strength, endurance
- Primary mechanic: Authority gain, defensive bases
- Secondary mechanic: Opponent disruption (discard)
- Ally bonus focus: Authority gain, enemy weakening
- Fantasy flavor: Knights, castles, paladins, holy light

## Suggested Implementation Steps

1. Design 8-12 faction cards with cost curve (1-7 trade):
   ```
   Low cost (1-2): Squires, heralds
   Mid cost (3-4): Knights, battle priests
   High cost (5-7): Paladins, warlords
   ```
2. Design 2-3 faction bases:
   - Watchtower (outpost with authority)
   - Grand castle (spawns infantry)
   - Cathedral (massive authority gain)
3. Create card files in `assets/cards/kingdom/`
4. Include spawning (castle spawns infantry tokens)
5. Balance authority gain against tempo loss
6. Write flavor text reflecting noble themes
7. Document faction strategy guide

## Related Documents
- docs/02-game-mechanics.md
- 2-001-card-template-format.md

## Dependencies
- 2-001: Card Template Format (file structure)

## Example Cards

**Squire** (Cost 1, Ship)
- Effects: +1 Combat, +1 Authority
- Ally: +1 Authority

**Knight Commander** (Cost 4, Ship)
- Effects: +4 Combat, +2 Authority
- Ally: Target opponent discards a card

**High Paladin** (Cost 6, Ship)
- Effects: +6 Combat, +4 Authority
- Ally: +3 Authority

**Grand Castle** (Cost 5, Base, Defense 6)
- Effects: +3 Authority, spawn Infantry Token each turn
- Infantry Token: Draw a card, +1 Combat

**Watchtower** (Cost 2, Base, Defense 3, Outpost)
- Effects: +2 Authority

## Acceptance Criteria
- [ ] 8-12 ship cards created
- [ ] 2-3 base cards created
- [ ] Strong defensive identity
- [ ] Authority gain meaningful but not overpowered
- [ ] Disruption effects balanced
