# 1-005: Turn Loop Structure

## Current Behavior
No turn structure exists. Game has no flow or phase progression.

## Intended Behavior
A C-based turn loop that manages:
- Turn start (trigger auto-draws, reset resources)
- Draw phase (with draw order choice, then auto-draws)
- Main phase (play cards, buy cards, attack)
- End phase (discard hand and played cards)
- Turn transition (switch active player)
- Game end detection (player authority <= 0)

## Suggested Implementation Steps

1. Create `src/core/05-game.h` with type definitions
2. Create `src/core/05-game.c` with implementation
3. Define game state structure:
   ```c
   typedef enum {
       PHASE_DRAW_ORDER,  // waiting for draw order selection
       PHASE_DRAW,        // executing draws
       PHASE_MAIN,        // player actions
       PHASE_END          // cleanup
   } GamePhase;

   typedef struct {
       Player* players[4];       // up to 4 players
       int player_count;
       int active_player;        // index of current player
       TradeRow* trade_row;
       int turn_number;
       GamePhase phase;
       bool game_over;
       int winner;               // player index, -1 if none
       CardType** card_database; // all card types
       int card_count;
   } Game;
   ```
4. Implement `Game* game_create(const char** names, int player_count)`
5. Implement `void game_free(Game* game)`
6. Implement `void game_start_turn(Game* game)` - reset resources
7. Implement `void game_submit_draw_order(Game* game, int* order, int count)`
8. Implement `void game_execute_draws(Game* game)` - draw + auto-draws
9. Implement `bool game_process_action(Game* game, Action* action)` - main phase
10. Implement `void game_end_turn(Game* game)` - cleanup and transition
11. Implement `bool game_check_over(Game* game)` - detect winner
12. Write tests in `tests/test-game.c`

## Related Documents
- docs/02-game-mechanics.md (draw order choice)
- docs/04-architecture-c-server.md
- 1-003-player-state-management.md
- 1-008-auto-draw-resolution-system.md

## Dependencies
- 1-002: Deck Management System
- 1-003: Player State Management
- 1-004: Trade Row Implementation

## Acceptance Criteria
- [ ] Game struct compiles without errors
- [ ] Game initializes with 2-4 players
- [ ] Turn phases execute in correct order
- [ ] Draw order selection works
- [ ] Active player switches after end phase
- [ ] Game ends when player authority reaches 0
- [ ] Winner correctly identified
- [ ] State can be serialized for network transmission
