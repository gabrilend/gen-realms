# 1-007a: Effect Dispatch Infrastructure

## Parent Issue
1-007: Card Effect System

## Current Behavior
No effect execution infrastructure exists.

## Intended Behavior
The foundational effect system infrastructure that:
- Defines all effect type enums
- Defines Effect struct with type, value, and target fields
- Creates dispatch table mapping effect types to handler functions
- Defines handler function signature
- Provides effect execution entry point
- Supports event callbacks for narrative/visual hooks

## Suggested Implementation Steps

1. Create `src/core/07-effects.h` with type definitions:
   ```c
   // {{{ EffectType enum
   typedef enum {
       EFFECT_ADD_TRADE,
       EFFECT_ADD_COMBAT,
       EFFECT_ADD_AUTHORITY,
       EFFECT_DRAW_CARDS,
       EFFECT_OPPONENT_DISCARD,
       EFFECT_SCRAP_TRADE_ROW,
       EFFECT_SCRAP_HAND_DISCARD,
       EFFECT_DESTROY_BASE,
       EFFECT_COPY_SHIP,
       EFFECT_NEXT_SHIP_FREE,
       EFFECT_NEXT_SHIP_TOP,
       EFFECT_UPGRADE_CARD,
       EFFECT_SPAWN_UNIT,
       EFFECT_COUNT  // for array sizing
   } EffectType;
   // }}}

   // {{{ Effect struct
   typedef struct {
       EffectType type;
       int value;
       char* target_card_id;
   } Effect;
   // }}}

   // {{{ Handler function pointer type
   typedef void (*EffectHandler)(Game* game, Player* player, Effect* effect, CardInstance* source);
   // }}}

   // {{{ Event callback type
   typedef void (*EffectEventFunc)(Game* game, CardInstance* card, Effect* effect, void* context);
   // }}}
   ```

2. Create `src/core/07-effects.c` with dispatch table:
   ```c
   // {{{ dispatch table
   static EffectHandler effect_handlers[EFFECT_COUNT] = {
       [EFFECT_ADD_TRADE] = handle_add_trade,
       [EFFECT_ADD_COMBAT] = handle_add_combat,
       // ... etc, initially NULL for unimplemented
   };
   // }}}
   ```

3. Implement `void effects_init(void)` - initialize dispatch table

4. Implement `void effects_execute(Effect* effect, Game* game, Player* player, CardInstance* source)`:
   ```c
   // {{{ effects_execute
   void effects_execute(Effect* effect, Game* game, Player* player, CardInstance* source) {
       if (effect->type >= EFFECT_COUNT) return;
       EffectHandler handler = effect_handlers[effect->type];
       if (handler) {
           handler(game, player, effect, source);
           // fire event callback if registered
       }
   }
   // }}}
   ```

5. Implement event callback registration:
   - `void effects_register_callback(EffectEventFunc callback, void* context)`
   - Store in static array of callbacks

6. Write basic tests in `tests/test-effects-infra.c`

## Related Documents
- docs/04-architecture-c-server.md
- 1-007-card-effect-parser.md (parent)

## Dependencies
- 1-001: Card Data Structure (CardInstance definition)
- 1-003: Player State Management (Player struct)

## Acceptance Criteria
- [ ] EffectType enum defined with all types
- [ ] Effect struct defined
- [ ] Dispatch table compiles
- [ ] effects_execute routes to correct handler
- [ ] Event callback system works
- [ ] Stub handlers for all effect types exist
