# 1-012: Gamestate Serialization

## Current Behavior
Game state exists only in C structs, cannot be transmitted over network.

## Intended Behavior
A JSON serialization system that:
- Converts Game struct to JSON for protocol transmission
- Handles player-specific views (hide opponent's hand)
- Includes all display-relevant information
- Supports partial updates (delta sync)
- Works with cJSON library

## Suggested Implementation Steps

1. Create `src/core/09-serialize.h` with function declarations
2. Create `src/core/09-serialize.c` with implementation
3. Implement `cJSON* serialize_game_for_player(Game* game, int player_id)`:
   ```c
   // Returns JSON with:
   // - Full info for requesting player
   // - Hidden info for opponents (hand_count, not hand contents)
   // - Full trade row
   // - Narrative text
   // - Turn/phase info
   ```
4. Implement `cJSON* serialize_card_instance(CardInstance* card)`
5. Implement `cJSON* serialize_player_public(Player* player)`
6. Implement `cJSON* serialize_player_private(Player* player)`
7. Implement `cJSON* serialize_trade_row(TradeRow* row)`
8. Implement `Action* deserialize_action(cJSON* json)` for client input
9. Write tests in `tests/test-serialize.c`

## Related Documents
- docs/04-architecture-c-server.md (protocol format)
- 1-005-turn-loop-structure.md

## Dependencies
- 1-005: Turn Loop (Game struct must exist)
- cJSON library

## JSON Output Example

```json
{
  "turn": 12,
  "phase": "main",
  "active_player": 0,
  "you": {
    "id": 0,
    "authority": 35,
    "d10": 7,
    "d4": 1,
    "trade": 3,
    "combat": 5,
    "hand": [
      {"card_id": "dire_bear", "instance_id": "abc123", ...}
    ],
    "bases": [...],
    "deck_count": 12,
    "discard_count": 5
  },
  "opponent": {
    "id": 1,
    "authority": 28,
    "hand_count": 4,
    "bases": [...],
    "deck_count": 8,
    "discard_count": 7
  },
  "trade_row": [...],
  "narrative": "The dire bear emerges..."
}
```

## Acceptance Criteria
- [ ] Game state serializes to valid JSON
- [ ] Player-specific views hide appropriate info
- [ ] Card instances include all display-relevant fields
- [ ] Deserialization parses client actions
- [ ] Round-trip tests pass
