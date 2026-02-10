# 4-010: Phase 4 Demo

## Current Behavior
No playable card content exists.

## Intended Behavior
A demonstration showing all card content loaded and playable:
- All factions with complete card sets
- Starting decks work correctly
- Upgrade effects apply properly
- Balance validator runs without errors
- Cards display with all metadata

## Suggested Implementation Steps

1. Create `run-phase4-demo.sh` in project root
2. Load all faction JSON files
3. Display faction statistics:
   - Number of cards per faction
   - Cost distribution
   - Effect types
4. Initialize a game with full card content
5. Run balance validator on all cards
6. Display example cards from each faction
7. Show upgrade effect demonstrations
8. Verify spawning abilities work

## Related Documents
- docs/03-roadmap.md
- 4-001 through 4-009

## Dependencies
- All previous Phase 4 issues (4-001 through 4-009)
- Phase 1 complete (game engine)

## Demo Output

```
=== SYMBELINE REALMS: PHASE 4 DEMO ===

Loading card content...

FACTION SUMMARY:
┌─────────────────┬───────┬──────────┬───────────┐
│ Faction         │ Cards │ Avg Cost │ Theme     │
├─────────────────┼───────┼──────────┼───────────┤
│ Merchant Guilds │    15 │      3.2 │ Trade/Econ│
│ The Wilds       │    15 │      2.8 │ Combat    │
│ High Kingdom    │    15 │      4.1 │ Authority │
│ Artificer Order │    15 │      3.5 │ Synergy   │
│ Neutral         │    10 │      2.5 │ Utility   │
│ Starting        │    10 │      0.0 │ Basic     │
└─────────────────┴───────┴──────────┴───────────┘

BALANCE VALIDATOR:
✓ All cards within cost guidelines
✓ Faction abilities properly distributed
✓ Upgrade effects balanced
✓ Spawning costs appropriate

SAMPLE CARDS:
[Merchant] Guild Courier - 2g - +2 Trade, Draw 1
[Wilds] Dire Bear - 5g - +5 Combat, Ally: +3 Combat
[Kingdom] Knight Commander - 4g - +3 Authority, Upgrade: +1 Combat
[Artificer] Battle Golem - 3g - +2 Combat, Auto-draw: Artif card

UPGRADE DEMO:
Original: Scout +1 Trade
After Blacksmith: Scout +1 Trade, +1 Combat (permanent)

Demo complete. All card content functional.
```

## Acceptance Criteria
- [ ] All faction JSON files load correctly
- [ ] Balance validator passes
- [ ] Starting deck initializes
- [ ] Upgrade effects can be applied
- [ ] Spawning abilities trigger
- [ ] All cards display proper metadata
