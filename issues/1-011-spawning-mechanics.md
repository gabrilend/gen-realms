# 1-011: Spawning Mechanics

## Current Behavior
No spawning exists. Cards cannot create other cards.

## Intended Behavior
A C-based spawning system where:
- Some bases spawn unit cards each turn
- Spawned cards go to discard pile (drawn after shuffle)
- Unit cards typically have draw or scrap-for-bonus effects
- Scrapped units thematically "regroup" at origin base
- Spawn effect is part of base's turn-start trigger
- Spawned cards are defined in base's spawns_id property
- Spawned units are new CardInstances (get their own images)

## Suggested Implementation Steps

1. CardType already has spawns_id from 1-001:
   ```c
   // In CardType:
   char* spawns_id;  // card type ID to spawn, or NULL
   ```
2. Define unit card types in card database (cards meant to be spawned)
3. Implement spawn in effect system:
   ```c
   void effect_spawn(Game* game, Player* player, CardInstance* base) {
       if (base->type->spawns_id == NULL) return;

       CardType* unit_type = game_find_card_type(game, base->type->spawns_id);
       CardInstance* unit = card_instance_create(unit_type);

       deck_add_to_discard(player->deck, unit);
       // emit spawn event for narrative
   }
   ```
4. Call spawn during base turn-start effect resolution (after deployed check)
5. Spawned CardInstances get new image_seed (unique art)
6. Write tests in `tests/test-spawn.c`

## Related Documents
- docs/02-game-mechanics.md (spawning rules)
- docs/04-architecture-c-server.md
- 1-010-base-card-type.md

## Dependencies
- 1-001: Card Data Structure (spawns_id property, CardInstance)
- 1-007: Card Effect System (spawn as effect type)
- 1-010: Base Card Type (deployment delay check)

## Acceptance Criteria
- [ ] Bases with spawns_id create unit CardInstances
- [ ] Units appear in owner's discard pile
- [ ] Units can be drawn and played
- [ ] Units can be scrapped for their bonus
- [ ] Spawn only triggers after base is deployed
- [ ] Each spawned unit gets unique instance_id and image_seed
