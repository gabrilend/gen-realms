# 1-004: Trade Row Implementation

## Current Behavior
No trade row exists. Players cannot acquire new cards.

## Intended Behavior
A C-based trade row system that:
- Displays 5 cards available for purchase
- Allows purchasing cards with trade currency
- Removes purchased cards and replaces from trade deck
- Includes the Explorer card (always available, costs 2)
- Tracks the trade deck (remaining unpurchased cards)
- Provides hook point for LLM DM to influence card selection (phase 5)
- Supports singleton encouragement bias

## Suggested Implementation Steps

1. Create `src/core/04-trade-row.h` with type definitions
2. Create `src/core/04-trade-row.c` with implementation
3. Define trade row structure:
   ```c
   // Function pointer for DM override (set in phase 5)
   typedef CardType* (*DMSelectFunc)(TradeRow* row, void* context);

   typedef struct {
       CardInstance* slots[5];      // 5 cards available (NULL for empty)
       CardType** trade_deck;       // remaining card types to draw from
       int trade_deck_count;
       int trade_deck_capacity;
       CardType* explorer_type;     // always-available explorer
       DMSelectFunc dm_select;      // hook for LLM selection (NULL = random)
       void* dm_context;            // context for DM function
   } TradeRow;
   ```
4. Implement `TradeRow* trade_row_create(CardType** all_cards, int count)`
5. Implement `void trade_row_free(TradeRow* row)`
6. Implement `void trade_row_fill_slots(TradeRow* row)` - uses dm_select if set
7. Implement `bool trade_row_can_buy(TradeRow* row, int slot, Player* p)`
8. Implement `CardInstance* trade_row_buy(TradeRow* row, int slot, Player* p)`
9. Implement `CardInstance* trade_row_buy_explorer(TradeRow* row, Player* p)`
10. Implement `void trade_row_set_dm(TradeRow* row, DMSelectFunc fn, void* ctx)`
11. Write tests in `tests/test-trade-row.c`

## Related Documents
- docs/02-game-mechanics.md
- docs/04-architecture-c-server.md
- 1-003-player-state-management.md

## Dependencies
- 1-001: Card Data Structure
- 1-003: Player State Management (for trade currency check)

## Acceptance Criteria
- [ ] Trade row struct compiles without errors
- [ ] Initializes with 5 cards from shuffled deck
- [ ] Purchase deducts trade and adds card instance to player discard
- [ ] Empty slot refills from trade deck
- [ ] Explorer always available for purchase
- [ ] Cannot buy card player cannot afford
- [ ] DM hook point works when set
- [ ] Falls back to random selection when DM hook is NULL
