# 3-003: Trade Row Selection Prompt

## Current Behavior
Trade row fills randomly from the deck.

## Intended Behavior
The LLM DM influences which cards appear in the trade row:
- Receives game state context
- Selects narratively appropriate cards
- Biases toward cards that fit the emerging story
- Still maintains game balance
- Returns structured card selections

## Suggested Implementation Steps

1. Create `src/llm/prompts/trade-row.lua`
2. Design the system prompt:
   ```
   You are the Dungeon Master for Symbeline Realms.
   Your role is to select cards for the trade row that
   advance the narrative while maintaining game balance.

   Consider:
   - The current story arc
   - Each player's faction preferences
   - Dramatic tension
   - Card availability in deck
   ```
3. Design the user prompt template with game state injection
4. Implement `TradeRowPrompt.generate(game_state)` - build prompt
5. Implement `TradeRowPrompt.parse_response(response)` - extract selections
6. Add validation (selected cards must exist in deck)
7. Implement fallback to random if LLM fails
8. Write tests for prompt generation and parsing

## Related Documents
- 3-001-llm-api-integration-module.md
- 3-002-game-state-serialization.md
- 1-004-trade-row-implementation.md

## Dependencies
- 3-001: LLM API Integration
- 3-002: Game State Serialization

## Prompt Example

```
System: You are the Dungeon Master for Symbeline Realms...

User: The trade row needs 2 new cards.

Current story: Lady Morgaine has been building a beast army
while Lord Theron relies on merchant wealth.

Available cards in deck: [list of 30 remaining cards]

Player 1 deck composition: 60% Wilds, 30% Kingdom
Player 2 deck composition: 50% Merchant, 40% Artificer

Select 2 cards that would create interesting choices.
Respond with card IDs only, one per line.
```

## Acceptance Criteria
- [ ] Prompt includes relevant game context
- [ ] LLM returns valid card selections
- [ ] Selections parse correctly
- [ ] Fallback works when LLM unavailable
- [ ] Narrative influence is noticeable
