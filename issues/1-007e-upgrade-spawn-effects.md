# 1-007e: Upgrade and Spawn Effects

## Parent Issue
1-007: Card Effect System

## Current Behavior
No upgrade or spawn effects exist.

## Intended Behavior
Effect handlers for the Symbeline-specific mechanics:
- Upgrade card (permanently modify a CardInstance)
- Spawn unit (bases creating units during combat)

## Suggested Implementation Steps

1. Implement `handle_upgrade_card()`:
   ```c
   // {{{ handle_upgrade_card
   static void handle_upgrade_card(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Player chooses a card to upgrade
       // Modification is permanent (stored in CardInstance)
       // target_card_id in effect specifies upgrade type

       game_request_upgrade_target(game, player, effect);
       // When target selected:
       // - Parse upgrade type from effect->target_card_id
       // - Apply bonus to CardInstance (attack_bonus, trade_bonus, etc.)
       // - Set needs_regen flag for new art
   }
   // }}}
   ```

2. Define upgrade types:
   ```c
   // {{{ upgrade types
   typedef enum {
       UPGRADE_ATTACK,      // +1 attack bonus
       UPGRADE_TRADE,       // +1 trade bonus
       UPGRADE_AUTHORITY,   // +1 authority bonus
       UPGRADE_DRAW,        // gains draw effect
   } UpgradeType;
   // }}}
   ```

3. Implement upgrade application:
   ```c
   // {{{ apply_upgrade
   void apply_upgrade(CardInstance* card, UpgradeType type, int value) {
       switch (type) {
           case UPGRADE_ATTACK:
               card->attack_bonus += value;
               break;
           case UPGRADE_TRADE:
               card->trade_bonus += value;
               break;
           // ... etc
       }
       card->needs_regen = true;  // trigger new art generation
   }
   // }}}
   ```

4. Implement `handle_spawn_unit()`:
   ```c
   // {{{ handle_spawn_unit
   static void handle_spawn_unit(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Create a temporary unit from card definition
       // target_card_id specifies which unit type to spawn
       CardType* unit_type = cards_get_type(effect->target_card_id);
       if (!unit_type) return;

       // Create temporary instance (not added to deck)
       CardInstance* spawned = cardinstance_create_temp(unit_type);

       // Add to player's in-play area for this turn
       player_add_temp_unit(player, spawned);

       // Execute the spawned unit's effects
       effects_execute_card(spawned, game, player);
   }
   // }}}
   ```

5. Track temporary units (cleaned up at end of turn)

6. Write tests in `tests/test-effects-upgrade-spawn.c`

## Related Documents
- docs/02-game-mechanics.md (upgrade system)
- 1-007a-effect-dispatch-infrastructure.md
- 1-010-base-card-type.md
- 1-011-spawning-mechanics.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure
- 1-001: Card Data Structure (CardInstance bonuses)
- 1-010: Base Card Type (bases have spawn effects)
- 1-011: Spawning Mechanics (spawn timing)

## Acceptance Criteria
- [ ] Upgrade effects modify CardInstance permanently
- [ ] Upgrade bonuses persist across shuffles
- [ ] needs_regen flag set on upgraded cards
- [ ] Spawn effects create temporary units
- [ ] Spawned units execute their effects
- [ ] Temporary units cleaned up at turn end
