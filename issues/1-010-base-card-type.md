# 1-010: Base Card Type

## Current Behavior
No base card type exists. All cards are treated as ships that leave play.

## Intended Behavior
C-based base card handling that:
- Stays in play across turns (until destroyed)
- Has defense value (must be attacked to destroy)
- May be an "outpost" (must be destroyed before player)
- Effects trigger each turn (not just when played)
- Effects activate the turn AFTER being played (deployment delay)
- Tracked separately from played ships in Deck struct
- Tracks current damage taken

## Suggested Implementation Steps

1. CardType already has base properties from 1-001:
   ```c
   // In CardType:
   int defense;       // health for bases
   bool is_outpost;   // must destroy before player
   ```
2. Add to CardInstance for tracking:
   ```c
   // Add to CardInstance:
   bool deployed;     // true after first full turn
   int damage_taken;  // for bases, tracks damage
   ```
3. Deck already has `in_play_bases` array from 1-002
4. Implement `void deck_play_base(Deck* deck, CardInstance* base)`
5. Implement `void deck_remove_base(Deck* deck, CardInstance* base)`
6. Modify `deck_end_turn()` to NOT discard bases
7. Implement deployment delay in turn start:
   ```c
   void game_deploy_bases(Game* game, Player* player) {
       for each base in player->deck->in_play_bases {
           if (!base->deployed) {
               base->deployed = true;
           } else {
               // trigger base effects
           }
       }
   }
   ```
8. Integrate with combat (outpost priority check)
9. Write tests in `tests/test-base.c`

## Related Documents
- docs/02-game-mechanics.md (base rules)
- docs/04-architecture-c-server.md
- 1-006-basic-combat-resolution.md

## Dependencies
- 1-001: Card Data Structure (base properties in CardType)
- 1-002: Deck Management (base tracking array)
- 1-006: Combat (base destruction)

## Acceptance Criteria
- [ ] Bases stay in play after turn ends
- [ ] Bases do not trigger effects on first turn (deployment delay)
- [ ] Deployed bases trigger effects each turn
- [ ] Outposts block direct player attacks
- [ ] Destroyed bases go to owner's discard pile
- [ ] Base damage tracking works correctly
