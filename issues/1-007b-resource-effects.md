# 1-007b: Resource Effects

## Parent Issue
1-007: Card Effect System

## Current Behavior
No resource modification effects exist.

## Intended Behavior
Effect handlers for basic resource modifications:
- Add trade (gold) to player's pool
- Add combat (attack) to player's pool
- Add/heal authority (health)
- Apply upgrade bonuses from CardInstance

## Suggested Implementation Steps

1. Implement `handle_add_trade()`:
   ```c
   // {{{ handle_add_trade
   static void handle_add_trade(Game* game, Player* player, Effect* effect, CardInstance* source) {
       int bonus = source ? source->trade_bonus : 0;
       player->trade_pool += effect->value + bonus;
   }
   // }}}
   ```

2. Implement `handle_add_combat()`:
   ```c
   // {{{ handle_add_combat
   static void handle_add_combat(Game* game, Player* player, Effect* effect, CardInstance* source) {
       int bonus = source ? source->attack_bonus : 0;
       player->combat_pool += effect->value + bonus;
   }
   // }}}
   ```

3. Implement `handle_add_authority()`:
   ```c
   // {{{ handle_add_authority
   static void handle_add_authority(Game* game, Player* player, Effect* effect, CardInstance* source) {
       int bonus = source ? source->authority_bonus : 0;
       player->authority += effect->value + bonus;
       // Note: authority can exceed starting value (no cap)
   }
   // }}}
   ```

4. Register handlers in dispatch table

5. Write tests in `tests/test-effects-resources.c`:
   - Test basic resource addition
   - Test with upgrade bonuses
   - Test multiple effects in sequence

## Related Documents
- 1-007a-effect-dispatch-infrastructure.md
- 1-003-player-state-management.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure
- 1-003: Player State Management (trade_pool, combat_pool, authority fields)

## Acceptance Criteria
- [ ] Trade effects add to trade pool
- [ ] Combat effects add to combat pool
- [ ] Authority effects modify authority
- [ ] Upgrade bonuses from CardInstance applied correctly
- [ ] Tests pass for all resource effects
