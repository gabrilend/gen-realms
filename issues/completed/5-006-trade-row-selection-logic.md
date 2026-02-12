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
- [x] LLM selects trade row cards
- [x] Faction balance considered
- [x] Fallback to random works
- [x] Selection is timely (not too slow)
- [x] Results feel narratively cohesive

## Implementation Notes

Created `src/llm/08-trade-select.h` and `src/llm/08-trade-select.c`:

### Core Data Structures
- `FactionBalance` - Tracks faction counts in trade row
- `SelectionScore` - Scoring factors (faction_need, singleton, cost_variety, narrative_fit)
- `CandidateCard` - Card with computed score
- `TradeSelectContext` - Game state + LLM config for callback

### Key Functions

**Context Management:**
- `trade_select_context_create()` - Create selection context
- `trade_select_context_free()` - Free context (doesn't free game/llm)

**Faction Balance:**
- `trade_select_get_balance()` - Get faction counts in trade row
- `trade_select_get_deck_balance()` - Get faction counts in remaining deck
- `trade_select_underrepresented()` - Find most underrepresented faction

**Candidate Scoring:**
- `trade_select_gather_candidates()` - Get top N cards from deck
- `trade_select_score_candidate()` - Score a card by all criteria
- `trade_select_score_faction_need()` - Score based on underrepresentation
- `trade_select_score_singleton()` - Score based on purchase history
- `trade_select_score_cost_variety()` - Score based on cost distribution

**LLM Integration:**
- `trade_select_build_prompt()` - Build LLM selection prompt
- `trade_select_parse_response()` - Parse LLM response to find card
- `trade_select_best_candidate()` - Select highest-scoring candidate

**DM Hook:**
- `trade_select_dm_callback()` - Install in TradeRow for LLM selection

### Scoring Weights
```c
#define WEIGHT_FACTION_NEED  0.30f  /* Underrepresented factions */
#define WEIGHT_SINGLETON     0.25f  /* Less-bought cards */
#define WEIGHT_COST_VARIETY  0.20f  /* Fill cost gaps */
#define WEIGHT_NARRATIVE     0.25f  /* LLM narrative fit */
```

### Unit Tests
26 tests in `tests/test-trade-select.c`:
- Context create/free
- Balance calculation
- Underrepresented faction detection
- Candidate gathering
- Scoring (faction, singleton, cost, complete)
- Prompt building
- Response parsing (case-insensitive)
- Best candidate selection
- DM callback (with and without LLM)

## Completion Date
2026-02-11
