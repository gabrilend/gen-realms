# 3-006: Singleton Encouragement Logic

## Current Behavior
Trade row selection is random or purely narrative.

## Intended Behavior
The LLM DM biases trade row toward cards matching player decks:
- Track which cards players have acquired
- Increase probability of matching faction cards
- Encourage "unified, semi-singleton decks" per vision
- Balance narrative with this mechanical bias
- Make faction commitment rewarding

## Suggested Implementation Steps

1. Create `src/llm/singleton-bias.lua`
2. Implement deck analysis:
   ```lua
   local function analyze_deck(player)
     local factions = {}
     for _, card in ipairs(player.deck.all_cards) do
       factions[card.faction] = (factions[card.faction] or 0) + 1
     end
     return factions
   end
   ```
3. Implement `Bias.calculate_weights(player, available_cards)`
4. Weight formula: `base_weight * (1 + faction_match_bonus)`
5. Integrate with trade row selection prompt:
   - Include faction analysis in prompt
   - Suggest weighted probabilities to LLM
6. Implement fallback weighted random selection
7. Add configuration for bias strength
8. Write tests for bias distribution

## Related Documents
- notes/vision (singleton encouragement)
- 3-003-trade-row-selection-prompt.md
- 1-004-trade-row-implementation.md

## Dependencies
- 3-003: Trade Row Selection Prompt

## Example Bias Calculation

```
Player 1 deck: 8 Wilds, 4 Kingdom, 2 Neutral

Available cards:
- Dire Bear (Wilds): base 1.0 * 1.8 = 1.8 weight
- Trading Post (Merchant): base 1.0 * 1.0 = 1.0 weight
- Knight Commander (Kingdom): base 1.0 * 1.4 = 1.4 weight

Faction match bonus = 0.1 * (cards_of_faction / total_cards)
Wilds: 8/14 = 0.57 * 1.4 = 0.8 bonus
```

## Acceptance Criteria
- [ ] Deck composition analyzed correctly
- [ ] Bias weights calculated
- [ ] LLM receives bias information
- [ ] Faction-focused decks see matching cards
- [ ] Bias is tunable
