# 1-008a: Auto-Draw Eligibility Detection

## Parent Issue
1-008: Auto-Draw Resolution System

## Current Behavior
No way to detect which cards have auto-draw effects.

## Intended Behavior
System to detect auto-draw eligible cards that:
- Scans hand for cards with draw effects
- Checks if draw effect is unspent (not used since last shuffle)
- Returns list of eligible cards for auto-draw
- Supports different draw effect types

## Suggested Implementation Steps

1. Define auto-draw effect criteria in `src/core/08-auto-draw.h`:
   ```c
   // {{{ auto-draw types
   typedef enum {
       AUTODRAW_ON_DRAW,      // triggers when card is drawn
       AUTODRAW_ON_PLAY,      // not auto-draw, requires play
       AUTODRAW_CONDITIONAL   // only if condition met
   } AutoDrawTrigger;

   typedef struct {
       CardInstance* card;
       Effect* draw_effect;
       AutoDrawTrigger trigger;
   } AutoDrawCandidate;
   // }}}
   ```

2. Add spent tracking to CardInstance (from 1-001):
   ```c
   // In CardInstance struct:
   // bool draw_effect_spent;  // reset on shuffle
   ```

3. Implement `autodraw_has_draw_effect()`:
   ```c
   // {{{ has draw effect
   bool autodraw_has_draw_effect(CardType* type) {
       for (int i = 0; i < type->primary_effect_count; i++) {
           if (type->primary_effects[i].type == EFFECT_DRAW_CARDS) {
               return true;
           }
       }
       return false;
   }
   // }}}
   ```

4. Implement `autodraw_is_eligible()`:
   ```c
   // {{{ is eligible
   bool autodraw_is_eligible(CardInstance* card) {
       // Must have draw effect
       if (!autodraw_has_draw_effect(card->type)) {
           return false;
       }

       // Must not be spent
       if (card->draw_effect_spent) {
           return false;
       }

       // Must be auto-draw trigger type
       // (determined by card definition)
       return card->type->auto_draw_trigger == AUTODRAW_ON_DRAW;
   }
   // }}}
   ```

5. Implement `autodraw_find_eligible()`:
   ```c
   // {{{ find eligible
   int autodraw_find_eligible(CardInstance** hand, int hand_count,
                               AutoDrawCandidate* out, int max_out) {
       int found = 0;
       for (int i = 0; i < hand_count && found < max_out; i++) {
           if (autodraw_is_eligible(hand[i])) {
               out[found].card = hand[i];
               out[found].draw_effect = autodraw_get_effect(hand[i]);
               out[found].trigger = hand[i]->type->auto_draw_trigger;
               found++;
           }
       }
       return found;
   }
   // }}}
   ```

6. Implement `autodraw_get_effect()` to find the draw effect

7. Write tests in `tests/test-autodraw-eligible.c`

## Related Documents
- 1-008-auto-draw-resolution-system.md (parent)
- 1-001-card-data-structure.md
- docs/02-game-mechanics.md

## Dependencies
- 1-001: Card Data Structure (CardInstance with spent flag)
- 1-007: Card Effect System (effect types)

## Acceptance Criteria
- [ ] Cards with draw effects detected
- [ ] Spent effects filtered out
- [ ] Eligible list built correctly
- [ ] Different trigger types distinguished
- [ ] Empty result when no eligible cards
