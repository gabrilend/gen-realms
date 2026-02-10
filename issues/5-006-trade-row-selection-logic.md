# 5-006: Trade Row Selection Logic

## Current Behavior
Trade row refills randomly from deck.

## Intended Behavior
LLM-guided trade row selection that:
- Considers current game state
- Balances faction representation
- Creates narrative opportunities
- Encourages strategic variety
- Maintains game balance

## Suggested Implementation Steps

1. Create selection prompt:
   ```c
   // {{{ selection prompt
   static const char* TRADE_ROW_PROMPT =
       "Given the game state:\n%s\n\n"
       "Available cards to add to trade row:\n%s\n\n"
       "Select the most narratively interesting card that:\n"
       "1. Balances faction representation\n"
       "2. Creates strategic options\n"
       "3. Fits the current battle narrative\n\n"
       "Respond with just the card name.";
   // }}}
   ```

2. Implement `trade_row_select()`:
   ```c
   // {{{ select card
   int trade_row_select(Game* game, CardType** candidates, int count) {
       char* state = world_state_to_prompt(game->world_state);
       char* cards = cards_list_to_string(candidates, count);

       char* prompt = prompt_format(TRADE_ROW_PROMPT, state, cards);
       LLMResponse* resp = llm_request(prompt, SYSTEM_NARRATOR);

       // Parse response to find matching card
       return find_card_by_name(candidates, count, resp->text);
   }
   // }}}
   ```

3. Implement faction balance tracking:
   ```c
   // {{{ faction balance
   typedef struct {
       int merchant_count;
       int wilds_count;
       int kingdom_count;
       int artificer_count;
   } FactionBalance;

   FactionBalance get_trade_row_balance(TradeRow* row);
   // }}}
   ```

4. Add fallback to random selection on LLM failure

5. Implement singleton encouragement (unique cards)

6. Write tests

## Related Documents
- 5-002-prompt-network-structure.md
- 1-004-trade-row-implementation.md

## Dependencies
- 5-001: LLM API Client
- 5-003: World State Prompt
- 1-004: Trade Row Implementation

## Selection Criteria

| Priority | Criterion |
|----------|-----------|
| 1 | Underrepresented faction |
| 2 | Narrative fit |
| 3 | Strategic variety |
| 4 | Cost distribution |

## Acceptance Criteria
- [ ] LLM selects trade row cards
- [ ] Faction balance considered
- [ ] Fallback to random works
- [ ] Selection is timely (not too slow)
- [ ] Results feel narratively cohesive
