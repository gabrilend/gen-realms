/* 08-auto-draw.h - Auto-draw resolution system type definitions
 *
 * Implements automatic draw effect resolution at turn start. Cards with
 * draw effects trigger automatically when drawn (before main phase),
 * allowing chains of draws to resolve without tedious play-draw loops.
 * Effects are marked "spent" until the next deck shuffle.
 */

#ifndef SYMBELINE_AUTODRAW_H
#define SYMBELINE_AUTODRAW_H

#include "01-card.h"
#include "02-deck.h"
#include "03-player.h"
#include <stdbool.h>

/* Include 05-game.h for Game type - this header is designed to be included
 * after the core type headers are defined.
 */
#include "05-game.h"

/* Maximum candidates to process in one eligibility scan */
#define AUTODRAW_MAX_CANDIDATES 16

/* Maximum chain iterations before forced stop (safety) */
#define AUTODRAW_MAX_ITERATIONS 20

/* ========================================================================== */
/*                               Enumerations                                 */
/* ========================================================================== */

/* {{{ AutoDrawResult
 * Result codes from auto-draw resolution.
 */
typedef enum {
    AUTODRAW_OK = 0,            /* Chain resolved successfully */
    AUTODRAW_NO_ELIGIBLE,       /* No cards needed processing */
    AUTODRAW_AWAITING_INPUT,    /* Waiting for draw order choice */
    AUTODRAW_ERROR_MAX_ITER,    /* Hit max iterations (safety) */
    AUTODRAW_ERROR_INVALID      /* Invalid parameters */
} AutoDrawResult;
/* }}} */

/* {{{ AutoDrawEventType
 * Types of events emitted during auto-draw resolution.
 */
typedef enum {
    AUTODRAW_EVENT_START = 0,   /* Chain beginning */
    AUTODRAW_EVENT_TRIGGER,     /* Specific card triggering */
    AUTODRAW_EVENT_CARD,        /* Card drawn via auto-draw */
    AUTODRAW_EVENT_COMPLETE     /* Chain finished */
} AutoDrawEventType;
/* }}} */

/* ========================================================================== */
/*                                Structures                                  */
/* ========================================================================== */

/* {{{ AutoDrawCandidate
 * A card eligible for auto-draw with its draw effect.
 */
typedef struct {
    CardInstance* card;         /* The card with draw effect */
    Effect* draw_effect;        /* Pointer to the draw effect */
    int effect_index;           /* Index in card's effects array */
} AutoDrawCandidate;
/* }}} */

/* {{{ AutoDrawEvent
 * Event data emitted during auto-draw resolution.
 */
typedef struct {
    AutoDrawEventType type;
    CardInstance* source;       /* Card that triggered (for TRIGGER/CARD) */
    CardInstance* drawn;        /* Card that was drawn (for CARD) */
    int chain_iteration;
    int total_drawn;
} AutoDrawEvent;
/* }}} */

/* {{{ AutoDrawListener
 * Callback function for auto-draw events.
 */
typedef void (*AutoDrawListener)(Game* game, Player* player,
                                  AutoDrawEvent* event, void* context);
/* }}} */

/* {{{ AutoDrawState
 * Tracks state during chain resolution.
 */
typedef struct {
    int iterations;             /* Current iteration count */
    int total_drawn;            /* Total cards drawn this chain */
    bool awaiting_input;        /* Paused for draw order choice */
} AutoDrawState;
/* }}} */

/* ========================================================================== */
/*                            Function Prototypes                             */
/* ========================================================================== */

/* {{{ Eligibility detection (1-008a) */
bool autodraw_has_draw_effect(CardType* type);
bool autodraw_is_eligible(CardInstance* card);
Effect* autodraw_get_draw_effect(CardInstance* card);
int autodraw_find_eligible(CardInstance** hand, int hand_count,
                            AutoDrawCandidate* out, int max_out);
/* }}} */

/* {{{ Spent flag management (1-008c) */
void autodraw_mark_spent(CardInstance* card);
bool autodraw_is_spent(CardInstance* card);
void autodraw_reset_spent_in_array(CardInstance** cards, int count);
/* }}} */

/* {{{ Chain resolution (1-008b) */
AutoDrawResult autodraw_resolve_chain(Game* game, Player* player);
int autodraw_execute_single(Game* game, Player* player,
                             AutoDrawCandidate* candidate,
                             int current_iteration);
/* }}} */

/* {{{ Event emission (1-008d) */
void autodraw_register_listener(AutoDrawListener listener, void* context);
void autodraw_unregister_listener(AutoDrawListener listener);
void autodraw_clear_listeners(void);
void autodraw_emit_event(Game* game, Player* player, AutoDrawEvent* event);
/* }}} */

#endif /* SYMBELINE_AUTODRAW_H */
