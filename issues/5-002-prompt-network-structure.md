# 3-002: Game State Serialization

## Current Behavior
Game state exists as Lua tables but cannot be presented to LLM.

## Intended Behavior
A serialization system that converts game state to LLM-readable format:
- Human-readable text representation
- Includes relevant context (not raw data dumps)
- Summarizes card information appropriately
- Tracks narrative-relevant events
- Configurable detail level

## Suggested Implementation Steps

1. Create `src/llm/serializer.lua`
2. Implement `Serializer.player_summary(player)`:
   ```
   "Sir Aldric has 42 Authority. Their deck favors the Merchant Guilds,
   with a strong trade engine. They hold 5 cards and have 3 gold, 2 attack."
   ```
3. Implement `Serializer.trade_row_summary(trade_row)`:
   ```
   "The trade row offers: a Dire Bear from the Wilds (4 gold),
   a Trading Post of the Merchants (3 gold)..."
   ```
4. Implement `Serializer.turn_summary(game)`:
   ```
   "Turn 7. Sir Aldric plays Guild Courier and Trade Caravan,
   generating 5 gold. They purchase the Merchant Prince."
   ```
5. Implement `Serializer.game_context(game)` - full state summary
6. Add faction flavor to descriptions
7. Implement event log for narrative continuity
8. Write tests for various game states

## Related Documents
- 3-001-llm-api-integration-module.md
- 1-005-turn-loop-structure.md

## Dependencies
- Phase 1 complete (game state exists)
- Phase 2 complete (cards have flavor)

## Example Output

```
=== The Battle for Symbeline ===

Turn 12. The tide of war shifts.

Lady Morgaine (Wilds/Kingdom) - 28 Authority
- Has amassed a pack of dire beasts and noble knights
- Currently holds 6 cards, 4 attack power ready

Lord Theron (Merchant/Artificer) - 35 Authority
- Commands a fleet of trading vessels and construct golems
- His Trading Post generates steady income

The markets offer:
- Primal Titan (7g) - A towering force of nature
- Arcane Workshop (3g) - Tools for the discerning artificer
- Sellsword (3g) - A blade for hire
```

## Acceptance Criteria
- [ ] Player state converts to readable text
- [ ] Trade row describes cards with flavor
- [ ] Turn events summarize actions
- [ ] Faction themes reflected in text
- [ ] Detail level is configurable
