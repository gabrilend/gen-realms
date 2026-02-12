# 2-008: Hidden Information Handling

## Status: COMPLETE

## Current Behavior
ViewPerspective enum (VIEW_SELF, VIEW_OPPONENT, VIEW_SPECTATOR) controls
what information is visible during game state serialization. Opponent hand
contents are hidden, showing only count. Spectators can see all hands.

## Original Behavior (before fix)
Gamestate serialization didn't consider player perspective.

## Intended Behavior
Proper hidden information handling that:
- Hides opponent hand contents
- Shows only public opponent info
- Reveals appropriate info to spectators
- Prevents information leakage in protocol
- Handles "reveal" events correctly

## Suggested Implementation Steps

1. Extend `src/core/09-serialize.c` with perspective handling
2. Define visibility rules:
   ```c
   typedef enum {
       VIEW_SELF,      // Full info for this player
       VIEW_OPPONENT,  // Limited info (hand count, not contents)
       VIEW_SPECTATOR  // Can see all hands
   } ViewPerspective;
   ```
3. Implement `cJSON* serialize_player_for_view(Player* p, ViewPerspective view)`
4. For opponent view, include:
   - Authority, d10, d4
   - Hand count (not contents)
   - Base info (public)
   - Deck/discard counts
5. For spectator view, include all info
6. Ensure action validation server-side (can't target unknown cards)
7. Test for information leakage

## Related Documents
- docs/04-architecture-c-server.md
- docs/02-game-mechanics.md (hidden information)
- 1-012-gamestate-serialization.md

## Dependencies
- 1-012: Gamestate Serialization

## Hidden vs Public Info

| Information | Self | Opponent | Spectator |
|-------------|------|----------|-----------|
| Hand contents | Yes | No | Yes |
| Hand count | Yes | Yes | Yes |
| Authority | Yes | Yes | Yes |
| d10/d4 | Yes | Yes | Yes |
| Bases | Yes | Yes | Yes |
| Deck contents | No | No | No |
| Deck count | Yes | Yes | Yes |
| Discard pile | Yes | Yes | Yes |

## Acceptance Criteria
- [x] Opponent hand contents never sent
- [x] Hand counts visible to all
- [x] Spectators see everything
- [x] Server validates actions without leaking
- [x] Test confirms no information leakage

## Implementation Notes

### Files Modified
- `src/core/09-serialize.h` - Added ViewPerspective enum and new function declarations
- `src/core/09-serialize.c` - Added serialize_player_for_view, serialize_game_for_spectator,
  updated serialize_player_public to include d10/d4 and discard pile

### Files Created
- `tests/test-hidden-info.c` - 10 unit tests validating hidden info handling

### Key Functions Added
- `serialize_player_for_view(Player*, ViewPerspective)` - Perspective-aware player serialization
- `serialize_game_for_spectator(Game*)` - Full visibility for spectators

### Test Coverage
1. Private player view includes hand contents
2. Public player view hides hand contents
3. Public player view includes d10/d4
4. Public player view includes discard pile
5. Game serialization hides opponent hand
6. Spectator view shows all hands
7. No hand info leaks in opponent JSON string
8. ViewPerspective VIEW_OPPONENT hides hand
9. ViewPerspective VIEW_SELF shows hand
10. ViewPerspective VIEW_SPECTATOR shows hand
