# 1-003: Player State Management

## Current Behavior
No player state tracking exists. Cannot track health, resources, or turn state.

## Intended Behavior
A C struct-based player state system that tracks:
- Authority (health, starts at 50)
- Trade (coins available this turn, resets each turn)
- Combat (attack power available this turn, resets each turn)
- Deck Flow d10 (5-0-9 cycle, tracks buy/scrap momentum)
- Deck Flow d4 (bonus cards drawn per turn)
- Factions played this turn (for ally ability triggers)
- Reference to player's deck state
- Connection info (for network identification)

## Suggested Implementation Steps

1. Create `src/core/03-player.h` with type definitions
2. Create `src/core/03-player.c` with implementation
3. Define player state structure:
   ```c
   typedef struct {
       int id;
       char* name;
       int authority;
       int trade;
       int combat;
       int d10;
       int d4;
       bool factions_played[5];  // indexed by Faction enum
       Deck* deck;
       int connection_id;        // for network tracking
   } Player;
   ```
4. Implement `Player* player_create(const char* name, int id)`
5. Implement `void player_free(Player* player)`
6. Implement `void player_add_trade(Player* p, int amount)`
7. Implement `void player_add_combat(Player* p, int amount)`
8. Implement `void player_add_authority(Player* p, int amount)`
9. Implement `void player_take_damage(Player* p, int amount)`
10. Implement `void player_reset_turn(Player* p)` - clear trade/combat/factions
11. Implement `bool player_is_alive(Player* p)` - check authority > 0
12. Implement `int player_get_hand_size(Player* p)` - returns 5 + d4 (min 1)
13. Write tests in `tests/test-player.c`

## Related Documents
- docs/02-game-mechanics.md (d10/d4 rules)
- docs/04-architecture-c-server.md
- 1-002-deck-management-system.md

## Dependencies
- 1-002: Deck Management System (player references deck)

## Acceptance Criteria
- [x] Player struct compiles without errors
- [x] New players start with correct default values (50 auth, d10=5, d4=0)
- [x] Resource modifications work correctly
- [x] Turn reset clears temporary resources but not authority
- [x] Player death detected when authority reaches 0
- [x] Hand size calculation respects d4 with minimum of 1
- [x] Memory management is correct

## Implementation Notes (2026-02-10)
- Created src/core/03-player.h with Player struct
- Created src/core/03-player.c with resource/turn management
- d10 overflow (9->0) increments d4, underflow (0->9) decrements d4
- player_get_hand_size() returns BASE + d4 with min 1
- Faction tracking for ally ability triggers
- Convenience wrappers for deck operations
- 34 tests passing in tests/test-core.c
