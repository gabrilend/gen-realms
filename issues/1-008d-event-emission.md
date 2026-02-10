# 1-008d: Auto-Draw Event Emission

## Parent Issue
1-008: Auto-Draw Resolution System

## Current Behavior
No events emitted for auto-draw sequence.

## Intended Behavior
Event system integration that:
- Emits events for each auto-draw trigger
- Emits events for each card drawn
- Provides chain summary event
- Enables narrative generation for auto-draws
- Supports client display of auto-draw sequence

## Suggested Implementation Steps

1. Define auto-draw event types:
   ```c
   // {{{ event types
   typedef enum {
       EVENT_AUTODRAW_START,      // chain beginning
       EVENT_AUTODRAW_TRIGGER,    // specific card triggering
       EVENT_AUTODRAW_CARD,       // card drawn via auto-draw
       EVENT_AUTODRAW_COMPLETE,   // chain finished
   } AutoDrawEventType;

   typedef struct {
       AutoDrawEventType type;
       CardInstance* source;      // card that triggered
       CardInstance* drawn;       // card that was drawn (if applicable)
       int chain_iteration;
       int total_drawn;
   } AutoDrawEvent;
   // }}}
   ```

2. Implement event emission in chain resolution:
   ```c
   // {{{ emit events
   AutoDrawResult autodraw_resolve_chain(Game* game, Player* player) {
       // Emit chain start
       AutoDrawEvent start_event = {
           .type = EVENT_AUTODRAW_START,
           .chain_iteration = 0,
           .total_drawn = 0
       };
       game_emit_autodraw_event(game, &start_event);

       // ... chain resolution ...

       // Emit chain complete
       AutoDrawEvent complete_event = {
           .type = EVENT_AUTODRAW_COMPLETE,
           .chain_iteration = state.iterations,
           .total_drawn = state.cards_drawn
       };
       game_emit_autodraw_event(game, &complete_event);
   }
   // }}}
   ```

3. Emit trigger events:
   ```c
   // {{{ trigger event
   // In autodraw_execute_single():
   AutoDrawEvent trigger_event = {
       .type = EVENT_AUTODRAW_TRIGGER,
       .source = candidate->card,
       .chain_iteration = current_iteration
   };
   game_emit_autodraw_event(game, &trigger_event);
   // }}}
   ```

4. Emit card drawn events:
   ```c
   // {{{ card drawn event
   // For each card drawn:
   AutoDrawEvent card_event = {
       .type = EVENT_AUTODRAW_CARD,
       .source = candidate->card,
       .drawn = drawn_card,
       .chain_iteration = current_iteration
   };
   game_emit_autodraw_event(game, &card_event);
   // }}}
   ```

5. Implement event dispatcher:
   ```c
   // {{{ event dispatcher
   void game_emit_autodraw_event(Game* game, AutoDrawEvent* event) {
       // Notify registered listeners
       for (int i = 0; i < game->autodraw_listener_count; i++) {
           game->autodraw_listeners[i](game, event);
       }
   }
   // }}}
   ```

6. Add listener registration:
   ```c
   // {{{ listener registration
   typedef void (*AutoDrawListener)(Game* game, AutoDrawEvent* event);

   void game_register_autodraw_listener(Game* game, AutoDrawListener listener) {
       if (game->autodraw_listener_count < MAX_LISTENERS) {
           game->autodraw_listeners[game->autodraw_listener_count++] = listener;
       }
   }
   // }}}
   ```

7. Include events in protocol for clients

8. Write tests for event emission

## Related Documents
- 1-008b-chain-resolution.md
- 5-005-event-narration-prompts.md
- 2-005-protocol-implementation.md

## Dependencies
- 1-008b: Chain Resolution
- Game event system (if exists)

## Event Sequence Example

```
EVENT_AUTODRAW_START
  iteration: 0, total: 0

EVENT_AUTODRAW_TRIGGER
  source: Guild Courier
  iteration: 1

EVENT_AUTODRAW_CARD
  source: Guild Courier
  drawn: Merchant Scout
  iteration: 1

EVENT_AUTODRAW_COMPLETE
  iteration: 1, total: 1
```

## Protocol Integration

```json
{
  "type": "autodraw_sequence",
  "events": [
    {"type": "trigger", "card": "Guild Courier"},
    {"type": "draw", "source": "Guild Courier", "drawn": "Merchant Scout"}
  ],
  "total_drawn": 1
}
```

## Acceptance Criteria
- [ ] Start event emitted at chain begin
- [ ] Trigger events for each auto-draw card
- [ ] Card events for each drawn card
- [ ] Complete event with summary
- [ ] Listeners receive all events
- [ ] Events included in protocol messages
