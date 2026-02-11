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
- [x] Game state serializes to valid JSON
- [x] Player-specific views hide appropriate info
- [x] Card instances include all display-relevant fields
- [x] Deserialization parses client actions
- [x] Round-trip tests pass

## Implementation Notes (2026-02-11)

### Files Created
- `src/core/09-serialize.h` - API declarations for serialization
- `src/core/09-serialize.c` - Implementation (~500 lines)
- `tests/test-serialize.c` - Comprehensive tests (113 tests, all passing)

### Key Functions Implemented
- `serialize_game_for_player()` - Player-specific view (hides opponent hands)
- `serialize_game_full()` - Full game state for debugging
- `serialize_card_instance()` - Card with all upgrades and visual state
- `serialize_card_type()` - Card type definitions
- `serialize_player_public()` - Opponent view (counts only)
- `serialize_player_private()` - Own player view (full hand, resources)
- `serialize_trade_row()` - Trade row with explorer
- `deserialize_action()` - Parse client input to Action struct
- `game_state_to_string()` / `game_state_to_string_pretty()` - Utility functions

### JSON Structure
The serialized game state follows the format specified in the issue, with:
- `turn`, `phase`, `active_player` for game state
- `you` object with full private player info
- `opponents` array with public info only
- `trade_row` with slots and explorer availability

### Dependencies
- Uses cJSON library (libs/cJSON.c) for JSON construction
- Integrates with all core modules (1-001 through 1-008)
