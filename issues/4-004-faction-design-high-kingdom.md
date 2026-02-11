# 4-004: Faction Design - High Kingdom

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
- 4-001-card-json-schema.md

## Dependencies
- 4-001: Card JSON Schema

## Acceptance Criteria
- [x] 8-12 ship cards created
- [x] 2-3 base cards created
- [x] Strong defensive identity
- [x] Authority gain meaningful but not overpowered
- [x] Disruption effects balanced

## Completion Notes (2026-02-10)

**Status: COMPLETED**

### Cards Created (15 total)

**Ships (11):**
| Card | Cost | Effects | Ally Effects |
|------|------|---------|--------------|
| Squire | 1 | +1 Combat, +1 Authority | +1 Authority |
| Royal Herald | 1 | +2 Authority | Draw 1 |
| Battle Priest | 2 | +2 Combat | +2 Authority |
| Knight Errant | 3 | +3 Combat, +1 Authority | +1 Combat |
| Banner Knight | 3 | +2 Combat, +2 Authority | +2 Authority, Draw 1 |
| Knight Commander | 4 | +4 Combat, +2 Authority | Opponent discards 1 |
| Royal Champion | 4 | +5 Combat | +3 Authority |
| Blessed Guardian | 5 | +3 Combat, +4 Authority | +2 Authority |
| Warlord | 5 | +5 Combat, Draw 1 | Opponent discards 1 |
| High Paladin | 6 | +6 Combat, +4 Authority | +3 Authority |
| Divine Sovereign | 7 | +5 Combat, +6 Authority | +4 Authority, Opponent discards 1 |

**Bases (3):**
| Card | Cost | Defense | Effects |
|------|------|---------|---------|
| Watchtower | 2 | 3 (Outpost) | +2 Authority |
| Grand Castle | 5 | 6 | +3 Authority, Spawns Infantry Token |
| Cathedral of Light | 6 | 5 | +4 Authority (Ally: +3 Authority) |

**Token (1):**
- Castle Infantry: +1 Combat, Draw 1 (Scrap: +1 Authority)

### Addendum 1 (2026-02-10)
Added **Banner Knight** (Cost 3) to bring faction to 15 cards total.

### Addendum 2 (2026-02-10) - Recruit Mechanic (The March Column)

**New faction-defining ability added:** `recruit`
- Send a card from hand to the back of deck (the march column)
- Draw from the front of the column
- Card will return when deck reshuffles (column cycles)

**Thematic Flavor:**
The Kingdom's army marches in an endless 2x2 mile column of sworn
countrymen. Recruited cards are sent to gather reinforcements—peasants
filling armor, squires seeking knights—they'll catch up when the column
cycles. Unlike Artificer scrap (permanent removal), recruit is temporary.

**Cards Updated with Recruit:**
| Card | Cost | When |
|------|------|------|
| Royal Herald | 1 | Main effect (reduced authority to 1, added recruit) |
| Watchtower | 2 | Main effect (reduced authority to 1, added recruit) |
| Battle Priest | 2 | Ally effect (added recruit) |
| Blessed Guardian | 5 | Ally effect (added recruit) |

**Design Intent:**
Kingdom now has a unique mechanical identity: march column logistics.
The faction rewards disciplined play—ensuring the best troops are at the
front while the rest gather strength at the back. Victory through superior
organization rather than raw power or permanent removal.
