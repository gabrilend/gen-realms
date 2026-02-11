# 3-002: Terminal Input System

## Current Behavior
No terminal input handling for SSH clients.

## Intended Behavior
An input system for terminal clients that:
- Parses command strings
- Translates to protocol messages
- Provides command completion hints
- Shows input validation feedback
- Supports command history

## Suggested Implementation Steps

1. Create `src/client/02-input.h` with type definitions
2. Create `src/client/02-input.c` with implementation
3. Define input commands:
   ```c
   typedef enum {
       CMD_PLAY,       // p N
       CMD_BUY,        // b N
       CMD_ATTACK,     // a [N]
       CMD_DRAW_ORDER, // d 3,1,5,2,4
       CMD_END_TURN,   // e
       CMD_HELP,       // h
       CMD_QUIT        // q
   } CommandType;
   ```
4. Implement `Command* input_parse(const char* line)`
5. Implement `char* input_to_json(Command* cmd)` - convert to protocol
6. Implement command validation with helpful errors
7. Add readline-style history (up/down arrows)
8. Implement tab completion for commands
9. Show available commands based on game phase
10. Write tests for parsing

## Related Documents
- docs/02-game-mechanics.md
- 3-001-terminal-renderer.md

## Dependencies
- 3-001: Terminal Renderer
- 2-005: Protocol Implementation

## Command Reference

```
Commands:
  p N        - Play card at index N from hand
  b N        - Buy card at slot N from trade row
  b w        - Buy wandering merchant
  a          - Attack opponent player
  a N        - Attack opponent base at index N
  d 3,1,5,2  - Set draw order (comma-separated indices)
  e          - End turn
  h          - Show help
  q          - Quit game
```

## Acceptance Criteria
- [ ] Commands parse correctly
- [ ] Invalid commands show helpful error
- [ ] JSON protocol messages generated
- [ ] History navigation works
- [ ] Phase-appropriate commands shown
