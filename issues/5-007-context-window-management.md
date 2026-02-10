# 3-007: Event Hook System

## Current Behavior
Game events occur without triggering narrative generation.

## Intended Behavior
An event system that triggers LLM narration on game events:
- Hooks into game engine at key points
- Emits events with relevant data
- LLM responds with narrative text
- Non-blocking (game doesn't wait for slow LLM)
- Configurable which events trigger narration

## Suggested Implementation Steps

1. Create `src/events.lua` with event emitter
2. Define event types:
   ```lua
   local events = {
     TURN_START = "turn_start",
     CARD_PLAYED = "card_played",
     CARD_PURCHASED = "card_purchased",
     ATTACK = "attack",
     BASE_DESTROYED = "base_destroyed",
     ALLY_TRIGGERED = "ally_triggered",
     TURN_END = "turn_end",
     GAME_OVER = "game_over"
   }
   ```
3. Implement `Events.emit(event_type, data)` - fire event
4. Implement `Events.on(event_type, handler)` - register listener
5. Create LLM listener that generates narrative
6. Implement event queue for batch processing
7. Add async/callback support for non-blocking
8. Integrate hooks into game engine
9. Add configuration for event filtering
10. Write tests for event flow

## Related Documents
- 3-004-narrative-generation-prompt.md
- 1-005-turn-loop-structure.md

## Dependencies
- 3-004: Narrative Generation Prompt
- Phase 1 complete (game engine)

## Event Flow Example

```
1. Player plays "Dire Bear"
2. Game engine calls Effects.execute_card()
3. Effects module emits CARD_PLAYED event:
   { card: dire_bear, player: player1, effects: [...] }
4. LLM listener receives event
5. LLM generates: "From the depths of the Thornwood..."
6. Narrative added to display queue
7. Game continues without waiting
```

## Acceptance Criteria
- [ ] Events emit at correct game points
- [ ] LLM listener receives events
- [ ] Narrative generated for each event type
- [ ] Game does not block on LLM
- [ ] Events can be filtered by config
