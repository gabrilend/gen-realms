# 6-006b: Event-to-Region Mapping

## Current Behavior
No mapping between game events and canvas regions.

## Intended Behavior
Map game events to appropriate canvas regions:
- Define event-to-region rules
- Consider player side for placement
- Handle special cases (game start, game over)
- Generate appropriate prompts per region

## Suggested Implementation Steps

1. Define event-region mapping:
   ```c
   // {{{ event mapping
   typedef struct {
       GameEventType event_type;
       CanvasRegion region_p1;  // Region for player 1
       CanvasRegion region_p2;  // Region for player 2
       int base_priority;
       const char* prompt_template;
   } EventRegionRule;

   static const EventRegionRule EVENT_RULES[] = {
       {
           EVENT_GAME_START,
           REGION_SKY, REGION_SKY,
           100,
           "Fantasy battlefield sky, dramatic clouds, two armies facing"
       },
       {
           EVENT_CARD_PLAYED,
           REGION_P1_FORCES, REGION_P2_FORCES,
           60,
           "%s, %s faction, fantasy card art"
       },
       {
           EVENT_BASE_PLAYED,
           REGION_P1_BASE, REGION_P2_BASE,
           80,
           "%s stronghold, %s faction architecture, fantasy fortress"
       },
       {
           EVENT_ATTACK,
           REGION_CENTER, REGION_CENTER,
           90,
           "Battle clash, combat action, magical energy, dynamic"
       },
       {
           EVENT_GAME_OVER,
           REGION_CENTER, REGION_CENTER,
           100,
           "Victory scene, triumphant %s, battlefield aftermath"
       }
   };
   // }}}
   ```

2. Implement `map_event_to_region()`:
   ```c
   // {{{ map event
   CanvasRegion map_event_to_region(GameEvent* event) {
       for (int i = 0; i < ARRAY_SIZE(EVENT_RULES); i++) {
           if (EVENT_RULES[i].event_type == event->type) {
               return event->player == 0
                   ? EVENT_RULES[i].region_p1
                   : EVENT_RULES[i].region_p2;
           }
       }
       return REGION_CENTER;  // Default
   }
   // }}}
   ```

3. Implement `get_event_priority()`:
   ```c
   // {{{ get priority
   int get_event_priority(GameEvent* event) {
       for (int i = 0; i < ARRAY_SIZE(EVENT_RULES); i++) {
           if (EVENT_RULES[i].event_type == event->type) {
               return EVENT_RULES[i].base_priority;
           }
       }
       return 50;  // Default medium priority
   }
   // }}}
   ```

4. Implement `build_region_prompt()`:
   ```c
   // {{{ build prompt
   char* build_region_prompt(GameEvent* event, WorldState* ws) {
       const char* template = NULL;
       for (int i = 0; i < ARRAY_SIZE(EVENT_RULES); i++) {
           if (EVENT_RULES[i].event_type == event->type) {
               template = EVENT_RULES[i].prompt_template;
               break;
           }
       }

       if (!template) return strdup("fantasy battlefield scene");

       // Format with event-specific details
       switch (event->type) {
           case EVENT_CARD_PLAYED:
               return format_string(template,
                   event->card->type->name,
                   faction_name(event->card->type->faction));

           case EVENT_GAME_OVER:
               return format_string(template,
                   ws->player_names[event->winner]);

           default:
               return strdup(template);
       }
   }
   // }}}
   ```

5. Handle region conflicts (same region, different events)

6. Add special handling for multi-region events

7. Write mapping tests

## Related Documents
- 6-006a-region-selector-structure.md
- 6-006-inpainting-region-selection.md (parent issue)
- 1-007-card-effect-system.md

## Dependencies
- 6-006a: Region Selector Structure
- 1-007: Card Effect System (GameEvent types)

## Event-Region Table

| Event | Player 1 Region | Player 2 Region | Priority |
|-------|-----------------|-----------------|----------|
| Game Start | SKY | SKY | 100 |
| Card Played | P1_FORCES | P2_FORCES | 60 |
| Base Played | P1_BASE | P2_BASE | 80 |
| Attack | CENTER | CENTER | 90 |
| Game Over | CENTER | CENTER | 100 |

## Acceptance Criteria
- [ ] All event types have region mappings
- [ ] Player side determines left/right
- [ ] Priorities assigned correctly
- [ ] Prompts build with event details
- [ ] Conflicts handled gracefully
