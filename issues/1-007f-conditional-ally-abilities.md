# 1-007f: Conditional and Ally Abilities

## Parent Issue
1-007: Card Effect System

## Current Behavior
No conditional or ally ability resolution exists.

## Intended Behavior
Systems for handling conditional effects:
- Ally abilities (trigger when faction matches another card in play)
- Scrap abilities (optional: scrap this card for bonus effect)
- Choice effects ("choose one" from multiple options)

## Suggested Implementation Steps

1. Add ability tracking to CardType:
   ```c
   // {{{ CardType ability fields
   typedef struct CardType {
       // ... existing fields ...
       Effect* primary_effects;      // always execute
       int primary_effect_count;
       Effect* ally_effects;         // execute if ally present
       int ally_effect_count;
       Effect* scrap_effects;        // execute if scrapped
       int scrap_effect_count;
       Faction faction;              // for ally checking
   } CardType;
   // }}}
   ```

2. Implement ally checking:
   ```c
   // {{{ effects_check_ally
   bool effects_check_ally(Game* game, Player* player, Faction faction) {
       // Check if any other card in play shares faction
       for (int i = 0; i < player->in_play_count; i++) {
           CardInstance* card = player->in_play[i];
           if (card->type->faction == faction) {
               return true;
           }
       }
       return false;
   }
   // }}}
   ```

3. Implement `effects_execute_card()`:
   ```c
   // {{{ effects_execute_card
   void effects_execute_card(CardInstance* card, Game* game, Player* player) {
       CardType* type = card->type;

       // Execute primary effects
       for (int i = 0; i < type->primary_effect_count; i++) {
           effects_execute(&type->primary_effects[i], game, player, card);
       }

       // Check and execute ally effects
       if (type->ally_effect_count > 0) {
           if (effects_check_ally(game, player, type->faction)) {
               for (int i = 0; i < type->ally_effect_count; i++) {
                   effects_execute(&type->ally_effects[i], game, player, card);
               }
           }
       }
   }
   // }}}
   ```

4. Implement scrap ability execution:
   ```c
   // {{{ effects_execute_scrap
   void effects_execute_scrap(CardInstance* card, Game* game, Player* player) {
       CardType* type = card->type;

       // Execute scrap effects
       for (int i = 0; i < type->scrap_effect_count; i++) {
           effects_execute(&type->scrap_effects[i], game, player, card);
       }

       // Remove card from game (not to discard)
       game_scrap_card(game, card);
   }
   // }}}
   ```

5. Implement choice effects:
   ```c
   // {{{ handle choice effects
   // Choice effects are marked with EFFECT_CHOICE type
   // value = number of options, target_card_id = serialized options
   static void handle_choice(Game* game, Player* player, Effect* effect, CardInstance* source) {
       game_request_effect_choice(game, player, effect);
   }
   // }}}
   ```

6. Write tests in `tests/test-effects-conditional.c`:
   - Test ally ability triggers with matching faction
   - Test ally ability does not trigger without match
   - Test scrap abilities execute and remove card
   - Test choice effects present options

## Related Documents
- 1-007a-effect-dispatch-infrastructure.md
- docs/02-game-mechanics.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure
- 1-001: Card Data Structure (faction field)

## Acceptance Criteria
- [x] Ally abilities trigger with matching faction in play
- [x] Ally abilities do not trigger without match
- [x] Scrap abilities execute when card is scrapped
- [ ] Scrapped cards removed from game (not to discard) - needs game_scrap_card()
- [ ] Choice effects present options to player - needs action queue
- [ ] Chosen effect executes correctly - needs action queue

## Implementation Notes (2026-02-10)
- effects_execute_card() checks ally conditions via player_has_faction_ally()
- Faction is marked via player_mark_faction_played() after executing primary effects
- Second card of same faction triggers ally effects
- effects_execute_scrap() runs scrap effects before card removal
- Choice effects deferred to action queue implementation
- Tests verify ally ability triggering logic
