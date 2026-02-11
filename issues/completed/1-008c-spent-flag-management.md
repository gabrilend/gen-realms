# 1-008c: Spent Flag Management

## Parent Issue
1-008: Auto-Draw Resolution System

## Current Behavior
No tracking of spent auto-draw effects.

## Intended Behavior
Spent flag system that:
- Marks draw effects as spent when triggered
- Persists spent state through hand/discard/deck movement
- Resets all spent flags on deck shuffle
- Handles edge cases (scrap, copy effects)

## Suggested Implementation Steps

1. Ensure CardInstance has spent flag (from 1-001):
   ```c
   // In CardInstance struct:
   typedef struct CardInstance {
       // ... other fields ...
       bool draw_effect_spent;
   } CardInstance;
   ```

2. Implement `autodraw_mark_spent()`:
   ```c
   // {{{ mark spent
   void autodraw_mark_spent(CardInstance* card) {
       card->draw_effect_spent = true;
   }
   // }}}
   ```

3. Implement `autodraw_is_spent()`:
   ```c
   // {{{ is spent
   bool autodraw_is_spent(CardInstance* card) {
       return card->draw_effect_spent;
   }
   // }}}
   ```

4. Modify `deck_shuffle()` to reset spent flags:
   ```c
   // {{{ shuffle with reset
   void deck_shuffle(Deck* deck) {
       // Reset spent flags on all cards being shuffled
       for (int i = 0; i < deck->discard_count; i++) {
           deck->discard[i]->draw_effect_spent = false;
       }

       // Move discard to deck
       // ... existing shuffle logic ...

       // Shuffle
       // ... existing shuffle logic ...
   }
   // }}}
   ```

5. Handle scrap (card removed from game):
   ```c
   // {{{ scrap handling
   // When a card is scrapped, spent flag is irrelevant
   // as card is removed from game entirely
   void game_scrap_card(Game* game, CardInstance* card) {
       // Remove from current zone
       // Card is destroyed, no flag reset needed
   }
   // }}}
   ```

6. Handle copy effects:
   ```c
   // {{{ copy handling
   // When copying a card's effects, DO NOT copy spent state
   // The copy is a fresh execution
   void effects_copy_card(Game* game, Player* player,
                          CardInstance* source, CardInstance* target) {
       // Execute target's effects (not source's spent state)
       effects_execute_card(target, game, player);
   }
   // }}}
   ```

7. Add debug logging for spent state changes

8. Write tests in `tests/test-autodraw-spent.c`

## Related Documents
- 1-008a-eligibility-detection.md
- 1-002-deck-management-system.md

## Dependencies
- 1-001: Card Data Structure (CardInstance definition)
- 1-002: Deck Management (shuffle function)

## State Diagram

```
Card Created → spent = false
     │
     ▼
Card Drawn (with auto-draw)
     │
     ▼ (auto-draw triggers)
spent = true ◄──────────────┐
     │                       │
     ▼                       │
Card Played                  │
     │                       │
     ▼                       │
Card to Discard              │
     │                       │ (spent preserved)
     ▼                       │
Deck Shuffled ───────────────┘
     │        (spent reset)
     ▼
spent = false
```

## Acceptance Criteria
- [ ] Spent flag set when auto-draw triggers
- [ ] Spent state persists through zones
- [ ] Shuffle resets all spent flags
- [ ] Scrapped cards handled correctly
- [ ] Copy effects don't inherit spent state
