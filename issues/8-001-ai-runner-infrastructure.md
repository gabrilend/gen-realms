# 8-001: AI Runner Infrastructure

## Current Behavior
No AI system exists. Games can only be played by human players.

## Intended Behavior
A core framework that enables AI agents to play the game by:
- Enumerating all legal actions from a given game state
- Evaluating action outcomes through simulation
- Executing chosen actions through the game API
- Managing turn flow and timing

## Suggested Implementation Steps

1. Create `src/ai/01-runner.h` with AI runner interface:
   ```c
   typedef struct AIRunner AIRunner;
   typedef struct AIAction AIAction;

   /* Action types the AI can take */
   typedef enum {
       AI_ACTION_PLAY_CARD,
       AI_ACTION_BUY_CARD,
       AI_ACTION_BUY_EXPLORER,
       AI_ACTION_ATTACK_PLAYER,
       AI_ACTION_ATTACK_BASE,
       AI_ACTION_SCRAP_HAND,
       AI_ACTION_END_TURN,
       AI_ACTION_RESOLVE_PENDING,
       AI_ACTION_SET_DRAW_ORDER
   } AIActionType;

   /* Create/destroy AI runner */
   AIRunner* ai_runner_create(Game* game, int player_id);
   void ai_runner_destroy(AIRunner* runner);

   /* Action enumeration */
   int ai_runner_enumerate_actions(AIRunner* runner, AIAction** out_actions);
   void ai_runner_free_actions(AIAction* actions, int count);

   /* Action execution */
   bool ai_runner_execute_action(AIRunner* runner, AIAction* action);

   /* Turn management */
   bool ai_runner_is_my_turn(AIRunner* runner);
   void ai_runner_think(AIRunner* runner); /* Called each frame */
   ```

2. Create `src/ai/01-runner.c` implementing:
   - Action enumeration by scanning game state
   - Legal move validation
   - Action execution wrappers around game API

3. Implement action enumeration for each action type:
   - `enumerate_play_actions()` - playable cards in hand
   - `enumerate_buy_actions()` - affordable cards in trade row
   - `enumerate_attack_actions()` - valid attack targets
   - `enumerate_pending_actions()` - pending action resolutions

4. Create simulation helper:
   ```c
   /* Clone game state for lookahead */
   Game* ai_runner_clone_game(AIRunner* runner);
   void ai_runner_free_clone(Game* clone);
   ```

5. Add timing/budget system:
   ```c
   /* Computation budget management */
   void ai_runner_set_time_budget_ms(AIRunner* runner, int ms);
   bool ai_runner_budget_remaining(AIRunner* runner);
   ```

6. Write unit tests:
   - Action enumeration completeness
   - Action execution correctness
   - Game state cloning fidelity

## Related Documents
- docs/03-roadmap.md (Phase 8)
- src/core/05-game.h (game API)
- issues/phase-8-progress.md

## Dependencies
- 1-005: Turn Loop Structure (turn detection)
- 1-007: Card Effect System (action effects)

## Acceptance Criteria
- [ ] AIRunner can enumerate all legal actions
- [ ] Actions execute correctly through game API
- [ ] Game state can be cloned for simulation
- [ ] Time budget system limits computation
- [ ] Unit tests cover all action types

## Notes
The AI runner is intentionally "dumb" - it only enumerates and executes.
All strategic intelligence comes from higher-level components (8-002 onwards).
This separation allows different AI strategies to share the same runner.
