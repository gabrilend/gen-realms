# 2-003: Faction Design - The Wilds

## Current Behavior
No faction cards exist.

## Intended Behavior
Design and implement The Wilds faction with:
- Theme: Nature's fury, beasts, growth, overwhelming numbers
- Primary mechanic: Combat power, swarming
- Secondary mechanic: Card draw (pack hunting)
- Ally bonus focus: Combat multipliers
- Fantasy flavor: Dire beasts, forest spirits, druids, primal magic

## Suggested Implementation Steps

1. Design 8-12 faction cards with cost curve (1-7 trade):
   ```
   Low cost (1-2): Wolf scouts, forest sprites
   Mid cost (3-4): Dire beasts, pack leaders
   High cost (5-7): Ancient treants, primal titans
   ```
2. Design 2-3 faction bases:
   - Sacred grove (spawns beast tokens)
   - Thornwall (defensive outpost)
   - Heart of the Forest (massive ally bonuses)
3. Create card files in `assets/cards/wilds/`
4. Include spawning bases (grove spawns wolves)
5. Emphasize combat through ally chains
6. Write flavor text reflecting primal nature
7. Document faction strategy guide

## Related Documents
- docs/02-game-mechanics.md
- 2-001-card-template-format.md

## Dependencies
- 2-001: Card Template Format (file structure)

## Example Cards

**Wolf Scout** (Cost 1, Ship)
- Effects: +2 Combat
- Ally: +2 Combat

**Dire Bear** (Cost 4, Ship)
- Effects: +5 Combat
- Ally: Draw a card

**Primal Titan** (Cost 7, Ship)
- Effects: +8 Combat, Draw a card
- Ally: +4 Combat

**Sacred Grove** (Cost 4, Base, Defense 5)
- Effects: Spawn a Wolf Token each turn
- Wolf Token: +2 Combat, scrap to draw a card

## Acceptance Criteria
- [ ] 8-12 ship cards created
- [ ] 2-3 base cards created
- [ ] Strong combat focus evident
- [ ] Spawning mechanic utilized
- [ ] Ally chains feel impactful
