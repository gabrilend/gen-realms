# 1-007d: Special Effects

## Parent Issue
1-007: Card Effect System

## Current Behavior
No special/complex effects exist.

## Intended Behavior
Effect handlers for special mechanics:
- Copy ship (play a ship as if it were another)
- Destroy base (attack and remove opponent's base)
- Next ship costs 0 (free purchase)
- Next ship goes to top of deck (instead of discard)

## Suggested Implementation Steps

1. Implement `handle_copy_ship()`:
   ```c
   // {{{ handle_copy_ship
   static void handle_copy_ship(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Player chooses a ship in play (theirs or trade row)
       // Execute that ship's effects as if played
       game_request_copy_target(game, player);
   }
   // }}}
   ```

2. Implement `handle_destroy_base()`:
   ```c
   // {{{ handle_destroy_base
   static void handle_destroy_base(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Player chooses opponent's base to destroy
       // Base is scrapped (removed from game)
       game_request_destroy_base_target(game, player);
   }
   // }}}
   ```

3. Implement `handle_next_ship_free()`:
   ```c
   // {{{ handle_next_ship_free
   static void handle_next_ship_free(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Set flag on player state
       player->next_ship_free = true;
       player->free_ship_max_cost = effect->value; // 0 = any cost
   }
   // }}}
   ```

4. Implement `handle_next_ship_top()`:
   ```c
   // {{{ handle_next_ship_top
   static void handle_next_ship_top(Game* game, Player* player, Effect* effect, CardInstance* source) {
       // Set flag - next purchased ship goes to deck top
       player->next_ship_to_top = true;
   }
   // }}}
   ```

5. Modify purchase logic to check these flags

6. Write tests in `tests/test-effects-special.c`

## Related Documents
- 1-007a-effect-dispatch-infrastructure.md
- 1-004-trade-row-implementation.md
- 1-010-base-card-type.md

## Dependencies
- 1-007a: Effect Dispatch Infrastructure
- 1-004: Trade Row (purchase modification)
- 1-010: Base Card Type (base destruction)

## Acceptance Criteria
- [ ] Copy ship executes target ship's effects
- [ ] Destroy base removes opponent's base
- [ ] Next-ship-free flag reduces cost to 0
- [ ] Next-ship-top flag puts purchase on deck
- [ ] Flags reset after use
