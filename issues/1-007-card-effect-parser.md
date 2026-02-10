# 1-007: Card Effect System

## Current Behavior
No effect execution exists. Card data cannot translate to game actions.

## Intended Behavior
A C-based effect system using dispatch tables that:
- Reads effect definitions from card data
- Executes effects in correct order
- Supports all effect types (trade, combat, authority, draw, scrap, upgrade, etc.)
- Handles conditional effects (ally abilities)
- Handles choice effects ("choose one")
- Provides hooks for effect execution events (narrative, visuals)
- Supports card upgrades that modify effect values

## Suggested Implementation Steps

1. Create `src/core/07-effects.h` with type definitions
2. Create `src/core/07-effects.c` with implementation
3. Define effect types enum:
   ```c
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
       EFFECT_UPGRADE_CARD,       // for blacksmith-style effects
       EFFECT_SPAWN_UNIT          // for base spawning
   } EffectType;

   typedef struct {
       EffectType type;
       int value;                 // amount for numeric effects
       char* target_card_id;      // for spawn/upgrade effects
   } Effect;

   // Event callback for narrative/visual hooks
   typedef void (*EffectEventFunc)(Game* game, CardInstance* card,
                                    Effect* effect, void* context);
   ```
4. Create dispatch table of effect handlers
5. Implement `void effects_execute(Effect* effect, Game* game, Player* player)`
6. Implement `void effects_execute_card(CardInstance* card, Game* game, Player* p)`
7. Implement `void effects_check_ally(CardInstance* card, Game* game, Player* p)`
8. Implement `void effects_execute_scrap(CardInstance* card, Game* game, Player* p)`
9. Implement upgrade bonus application (from CardInstance bonuses)
10. Add event callback registration for LLM/visual hooks
11. Write tests in `tests/test-effects.c`

## Related Documents
- docs/02-game-mechanics.md (upgrade system)
- docs/04-architecture-c-server.md
- 1-001-card-data-structure.md

## Dependencies
- 1-001: Card Data Structure (effects stored in cards, upgrades in instances)
- 1-003: Player State Management (effects modify player)
- 1-002: Deck Management (draw effects)

## Acceptance Criteria
- [ ] Effect dispatch table compiles and works
- [ ] All basic effect types execute correctly
- [ ] Ally abilities trigger when faction matches
- [ ] Scrap abilities work correctly
- [ ] Complex cards with multiple effects work
- [ ] Upgrade bonuses from CardInstance are applied
- [ ] Event callbacks fire for narrative integration
