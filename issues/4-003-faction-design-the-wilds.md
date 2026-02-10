# 4-003: Faction Design - The Wilds

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
- 4-001-card-json-schema.md

## Dependencies
- 4-001: Card JSON Schema

## Acceptance Criteria
- [x] 8-12 ship cards created
- [x] 2-3 base cards created
- [x] Strong combat focus evident
- [x] Spawning mechanic utilized
- [x] Ally chains feel impactful

## Completion Notes (2026-02-10)

**Status: COMPLETED**

### Cards Created (15 total)

**Ships (11):**
| Card | Cost | Effects | Ally Effects |
|------|------|---------|--------------|
| Wolf Scout | 1 | +2 Combat | +2 Combat |
| Forest Sprite | 1 | +1 Combat | Draw 1 |
| Pack Hunter | 2 | +2 Combat | +1 Combat, Draw 1 |
| Beastcaller | 3 | +2 Combat | +3 Combat, Draw 1 |
| Thornback Boar | 3 | +4 Combat | - (Scrap: +3 Combat) |
| Swarm of Crows | 3 | +3 Combat | +2 Combat |
| Dire Bear | 4 | +5 Combat | Draw 1 |
| Forest Shaman | 4 | +2 Combat, Draw 1 | +3 Combat |
| Alpha Pack Leader | 5 | +4 Combat, Draw 1 | +4 Combat |
| Ancient Treant | 5 | +6 Combat | +2 Combat, +2 Authority |
| Primal Titan | 7 | +8 Combat, Draw 1 | +4 Combat |

**Bases (3):**
| Card | Cost | Defense | Effects |
|------|------|---------|---------|
| Thornwall | 3 | 4 (Outpost) | +2 Combat (Ally: +1 Combat) |
| Sacred Grove | 4 | 5 | +1 Combat, Spawns Wolf Token |
| Heart of the Forest | 6 | 7 | +2 Combat (Ally: +4 Combat, Draw 1) |

**Token (1):**
- Spirit Wolf: +2 Combat (Scrap: Draw 1)

### Addendum (2026-02-10)
Added **Beastcaller** (Cost 3) to bring faction to 15 cards total.
