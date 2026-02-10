# 2-010: Phase 2 Demo

## Current Behavior
Phase 1 demo exists with placeholder cards.

## Intended Behavior
An enhanced demo that:
- Uses the complete card database
- All four factions playable
- Faction strategies emerge through play
- Balance is testable through repeated games
- Card art placeholders display
- Win/loss statistics tracked

## Suggested Implementation Steps

1. Update `issues/completed/demos/phase-2-demo.lua`
2. Integrate card loader with game initialization
3. Build trade deck from all faction cards
4. Display faction colors in TUI
5. Show card art (ASCII mode)
6. Add faction indicator to cards in hand
7. Track game statistics:
   - Games played
   - Wins per player
   - Faction usage
   - Average game length
8. Update simple AI to consider factions
9. Create match history log
10. Update run script

## Related Documents
- 1-012-phase-1-demo.md
- All Phase 2 issues

## Dependencies
- All Phase 2 issues (2-001 through 2-009)

## Demo Features

```
=== Symbeline Realms - Phase 2 Demo ===

Your Hand:
[1] Guild Courier (Merchant) +2T, Ally: Draw
[2] Wolf Scout (Wilds) +2C, Ally: +2C
[3] Village Scout +1T
[4] Village Scout +1T
[5] Hedge Knight +1C

Trade Row:
[A] Dire Bear (Wilds) Cost: 4 | +5C, Ally: Draw
[B] Trading Post (Merchant) Cost: 3 | Base, +2T/turn
[C] Construct Apprentice (Artificer) Cost: 1 | +1T, Scrap: +2C
[D] Knight Commander (Kingdom) Cost: 4 | +4C+2A, Ally: Discard
[E] Sellsword (Neutral) Cost: 3 | +3C, Scrap: +3T
[W] Wandering Merchant Cost: 2 | Always Available

Stats: Games: 5 | You: 3 wins | AI: 2 wins
```

## Acceptance Criteria
- [ ] Complete card database in use
- [ ] All factions appear in trade row
- [ ] Faction colors/indicators visible
- [ ] Games feel strategically different
- [ ] Statistics tracking works
- [ ] Demo showcases Phase 2 content
