# 2-008: Hidden Information Handling

## Current Behavior
Gamestate serialization doesn't consider player perspective.

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
- [ ] Opponent hand contents never sent
- [ ] Hand counts visible to all
- [ ] Spectators see everything
- [ ] Server validates actions without leaking
- [ ] Test confirms no information leakage
