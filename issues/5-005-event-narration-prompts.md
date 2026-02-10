# 5-005: Event Narration Prompts

## Current Behavior
No narrative generated for game events.

## Intended Behavior
Event-specific prompts that generate narrative for:
- Card plays
- Purchases
- Attacks
- Base destruction
- Turn transitions
- Game end

## Suggested Implementation Steps

1. Define event types and templates:
   ```c
   // {{{ event templates
   static const char* EVENT_TEMPLATES[] = {
       [EVENT_CARD_PLAYED] =
           "Narrate %s playing the %s. Effect: %s. One sentence.",
       [EVENT_CARD_PURCHASED] =
           "Describe %s acquiring the %s from the markets. Brief.",
       [EVENT_ATTACK_PLAYER] =
           "Describe an attack dealing %d damage to %s. Dramatic.",
       [EVENT_ATTACK_BASE] =
           "Narrate the assault on %s's %s (defense %d). Vivid.",
       [EVENT_BASE_DESTROYED] =
           "Describe the fall of %s. Dramatic conclusion.",
       [EVENT_TURN_START] =
           "Brief transition: Turn %d begins for %s.",
       [EVENT_GAME_OVER] =
           "Narrate %s's victory over %s. Epic conclusion."
   };
   // }}}
   ```

2. Implement event narration builder:
   ```c
   // {{{ build event narration
   char* event_narration_build(GameEvent* event) {
       const char* template = EVENT_TEMPLATES[event->type];

       switch (event->type) {
           case EVENT_CARD_PLAYED:
               return prompt_format(template,
                   event->player->name,
                   event->card->type->name,
                   effect_to_string(event->card->type->effects));
           // ... other cases
       }
   }
   // }}}
   ```

3. Implement attack narration with intensity scaling

4. Implement turn transition narration

5. Implement game over narration

6. Write tests for each event type

## Related Documents
- 5-002-prompt-network-structure.md
- 1-008d-event-emission.md

## Dependencies
- 5-002: Prompt Network Structure
- Game event system

## Event Examples

```
EVENT_CARD_PLAYED:
"Lady Morgaine summons the Dire Bear to the battlefield,
its savage strength adding five points of combat to her
growing army."

EVENT_ATTACK_PLAYER:
"Lord Theron's forces strike directly at Lady Morgaine,
her authority crumbling by seven points under the
relentless assault."

EVENT_BASE_DESTROYED:
"With a thunderous crash, the ancient Watchtower
collapses, its defenders scattered to the winds."
```

## Acceptance Criteria
- [ ] All event types have templates
- [ ] Narration quality is good
- [ ] Attack intensity scales with damage
- [ ] Turn transitions are brief
- [ ] Game over is epic
