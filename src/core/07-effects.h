/* 07-effects.h - Effect execution system type definitions
 *
 * Implements the effect dispatch system that routes EffectType values to
 * handler functions. Effects are triggered when cards are played, ally
 * conditions are met, or cards are scrapped. The system supports event
 * callbacks for narrative hooks (Phase 5 LLM integration).
 */

#ifndef SYMBELINE_EFFECTS_H
#define SYMBELINE_EFFECTS_H

#include "01-card.h"
#include "02-deck.h"
#include "03-player.h"
#include "05-game.h"
#include <stdbool.h>

/* Maximum number of event callbacks that can be registered */
#define MAX_EFFECT_CALLBACKS 8

/* ========================================================================== */
/*                              Type Definitions                              */
/* ========================================================================== */

/* {{{ EffectContext
 * Additional context passed to effect handlers for stateful effects.
 */
typedef struct {
    bool next_ship_free;        /* Next purchased ship costs 0 */
    int free_ship_max_cost;     /* Max cost for free ship (0 = any) */
    bool next_ship_to_top;      /* Next purchase goes to deck top */
    int pending_draws;          /* Draws waiting for auto-draw resolution */
} EffectContext;
/* }}} */

/* {{{ EffectHandler
 * Function pointer type for effect handler functions.
 * Each effect type has a corresponding handler.
 */
typedef void (*EffectHandler)(Game* game, Player* player,
                               Effect* effect, CardInstance* source);
/* }}} */

/* {{{ EffectEventFunc
 * Callback for effect events (used for narrative hooks in Phase 5).
 * Called after an effect successfully executes.
 */
typedef void (*EffectEventFunc)(Game* game, Player* player,
                                 CardInstance* source, Effect* effect,
                                 void* context);
/* }}} */

/* {{{ EffectCallback
 * Registered callback with its context.
 */
typedef struct {
    EffectEventFunc callback;
    void* context;
} EffectCallback;
/* }}} */

/* ========================================================================== */
/*                            Function Prototypes                             */
/* ========================================================================== */

/* {{{ Initialization */
void effects_init(void);
/* }}} */

/* {{{ Effect execution */
void effects_execute(Game* game, Player* player,
                     Effect* effect, CardInstance* source);
void effects_execute_all(Game* game, Player* player,
                         Effect* effects, int count, CardInstance* source);
void effects_execute_card(Game* game, Player* player,
                          CardInstance* card);
void effects_execute_ally(Game* game, Player* player,
                          CardInstance* card);
void effects_execute_scrap(Game* game, Player* player,
                           CardInstance* card);
/* }}} */

/* {{{ Event callbacks */
void effects_register_callback(EffectEventFunc callback, void* context);
void effects_unregister_callback(EffectEventFunc callback);
void effects_clear_callbacks(void);
/* }}} */

/* {{{ Context management */
EffectContext* effects_get_context(Player* player);
void effects_reset_context(Player* player);
/* }}} */

#endif /* SYMBELINE_EFFECTS_H */
