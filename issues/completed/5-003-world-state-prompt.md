# 5-003: World State Prompt

## Status: COMPLETED

## Current Behavior
World state system maintains persistent narrative context for LLM prompts.

## Intended Behavior
A world state prompt that:
- Maintains persistent narrative context
- Summarizes game progress
- Tracks faction dominance
- Describes battlefield conditions
- Updates as game progresses

## Suggested Implementation Steps

1. Create world state structure:
   ```c
   // {{{ world state
   typedef struct {
       char* battlefield_description;
       char* player1_forces;
       char* player2_forces;
       char* recent_events[10];
       int event_count;
       int turn_number;
   } WorldState;
   // }}}
   ```

2. Implement `world_state_init()`:
   ```c
   // {{{ init
   WorldState* world_state_init(Game* game) {
       WorldState* ws = malloc(sizeof(WorldState));
       ws->battlefield_description =
           "The contested realm of Symbeline stretches before "
           "two rival commanders, each seeking dominion.";
       ws->turn_number = 1;
       return ws;
   }
   // }}}
   ```

3. Implement `world_state_update()`:
   ```c
   // {{{ update
   void world_state_update(WorldState* ws, Game* game) {
       // Update force descriptions based on played cards
       // Track faction balance
       // Update battlefield conditions
   }
   // }}}
   ```

4. Implement `world_state_to_prompt()`:
   ```c
   // {{{ to prompt
   char* world_state_to_prompt(WorldState* ws) {
       return prompt_build(PROMPT_WORLD_STATE, &(PromptVars){
           .battlefield = ws->battlefield_description,
           .player1 = ws->player1_forces,
           .player2 = ws->player2_forces,
           .turn = ws->turn_number
       });
   }
   // }}}
   ```

5. Add faction tracking (which faction is dominant)

6. Add event summarization (last N events)

7. Write tests

## Related Documents
- 5-002-prompt-network-structure.md
- docs/02-game-mechanics.md

## Dependencies
- 5-002: Prompt Network Structure

## Example Output

```
The battle for Symbeline enters its twelfth turn.

Lady Morgaine commands the combined might of the Wilds and
Kingdom factions. Her forces include:
- A pack of three dire beasts
- Two Knight Commanders
- The fortified Trading Post

Lord Theron marshals the Merchant Guilds and Artificer Order:
- Three mechanical constructs
- A network of trade caravans
- The ancient Watchtower outpost

The battlefield shows signs of prolonged conflict, with
fallen constructs and beast remains scattered across the
contested ground.
```

## Acceptance Criteria
- [x] World state initializes correctly
- [x] State updates on game events
- [x] Faction balance tracked
- [x] Recent events summarized
- [x] Prompt output is narrative quality

## Implementation Notes

### Files Created
- `src/llm/03-world-state.h` - WorldState structure and function prototypes
- `src/llm/03-world-state.c` - World state management implementation
- `tests/test-world-state.c` - Unit tests (12 tests, all passing)

### Key Features
- WorldState struct with battlefield description, player forces, events
- Circular buffer for event history (last 10 events)
- Tension calculation based on authority, game length, and authority closeness
- Dynamic battlefield description based on tension level
- Integration with PromptVars for PROMPT_WORLD_STATE building
- Faction name lookup for narrative use

### Tension Calculation
Factors affecting tension (0.0 - 1.0):
- Authority closeness (0-0.4): How close player authorities are
- Low authority danger (0-0.4): How close either player is to losing
- Game length (0-0.2): Longer games = more dramatic

### Test Coverage
- State creation and initialization
- Event recording and circular buffer
- Tension calculation (low and high scenarios)
- State updates and battlefield description changes
- PromptVars population
- Full context building

### Note
Uses mock Game structures in tests for standalone compilation.
Full integration tested when linked with core game modules.

Completed: 2026-02-10
