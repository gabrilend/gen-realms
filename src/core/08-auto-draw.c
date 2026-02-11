/* 08-auto-draw.c - Auto-draw resolution system implementation
 *
 * Cards with draw effects (EFFECT_DRAW) trigger automatically when drawn
 * to hand at turn start. This prevents tedious play-draw-play loops by
 * resolving all draw chains before the main phase begins.
 */

/* Enable POSIX functions */
#define _POSIX_C_SOURCE 200809L

#include "08-auto-draw.h"
#include "05-game.h"
#include <stdlib.h>
#include <string.h>

/* Maximum listeners for auto-draw events */
#define MAX_AUTODRAW_LISTENERS 8

/* ========================================================================== */
/*                             Static State                                   */
/* ========================================================================== */

/* {{{ Listener registry
 * Registered callbacks for auto-draw events.
 */
static struct {
    AutoDrawListener listener;
    void* context;
} s_listeners[MAX_AUTODRAW_LISTENERS];
static int s_listener_count = 0;
/* }}} */

/* ========================================================================== */
/*                       Eligibility Detection (1-008a)                       */
/* ========================================================================== */

/* {{{ autodraw_has_draw_effect
 * Returns true if the card type has a primary draw effect.
 * Only checks primary effects (not ally/scrap) since those require
 * player action to trigger.
 */
bool autodraw_has_draw_effect(CardType* type) {
    if (!type || !type->effects) {
        return false;
    }

    for (int i = 0; i < type->effect_count; i++) {
        if (type->effects[i].type == EFFECT_DRAW) {
            return true;
        }
    }

    return false;
}
/* }}} */

/* {{{ autodraw_get_draw_effect
 * Returns the first draw effect from a card's primary effects.
 * Returns NULL if no draw effect exists.
 */
Effect* autodraw_get_draw_effect(CardInstance* card) {
    if (!card || !card->type || !card->type->effects) {
        return NULL;
    }

    for (int i = 0; i < card->type->effect_count; i++) {
        if (card->type->effects[i].type == EFFECT_DRAW) {
            return &card->type->effects[i];
        }
    }

    return NULL;
}
/* }}} */

/* {{{ autodraw_is_eligible
 * Returns true if a card is eligible for auto-draw.
 * Requirements:
 * - Card must have a draw effect
 * - Draw effect must not be spent (used since last shuffle)
 */
bool autodraw_is_eligible(CardInstance* card) {
    if (!card) {
        return false;
    }

    /* Must have draw effect */
    if (!autodraw_has_draw_effect(card->type)) {
        return false;
    }

    /* Must not be spent */
    if (card->draw_effect_spent) {
        return false;
    }

    return true;
}
/* }}} */

/* {{{ autodraw_find_eligible
 * Scans an array of cards (typically hand) for auto-draw eligible cards.
 * Returns the number of eligible cards found, up to max_out.
 */
int autodraw_find_eligible(CardInstance** hand, int hand_count,
                            AutoDrawCandidate* out, int max_out) {
    if (!hand || !out || max_out <= 0) {
        return 0;
    }

    int found = 0;
    for (int i = 0; i < hand_count && found < max_out; i++) {
        CardInstance* card = hand[i];
        if (!card) {
            continue;
        }

        if (autodraw_is_eligible(card)) {
            out[found].card = card;
            out[found].draw_effect = autodraw_get_draw_effect(card);

            /* Find effect index for reference */
            out[found].effect_index = -1;
            if (card->type && card->type->effects) {
                for (int j = 0; j < card->type->effect_count; j++) {
                    if (card->type->effects[j].type == EFFECT_DRAW) {
                        out[found].effect_index = j;
                        break;
                    }
                }
            }

            found++;
        }
    }

    return found;
}
/* }}} */

/* ========================================================================== */
/*                      Spent Flag Management (1-008c)                        */
/* ========================================================================== */

/* {{{ autodraw_mark_spent
 * Marks a card's draw effect as spent (already triggered this shuffle).
 */
void autodraw_mark_spent(CardInstance* card) {
    if (card) {
        card->draw_effect_spent = true;
    }
}
/* }}} */

/* {{{ autodraw_is_spent
 * Returns true if the card's draw effect has been spent.
 */
bool autodraw_is_spent(CardInstance* card) {
    if (!card) {
        return true;  /* Treat NULL as spent (no effect) */
    }
    return card->draw_effect_spent;
}
/* }}} */

/* {{{ autodraw_reset_spent_in_array
 * Resets spent flags for all cards in an array.
 * Called during shuffle to allow effects to trigger again.
 * Note: deck_shuffle() already does this, but this can be used
 * for other scenarios.
 */
void autodraw_reset_spent_in_array(CardInstance** cards, int count) {
    if (!cards) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (cards[i]) {
            cards[i]->draw_effect_spent = false;
        }
    }
}
/* }}} */

/* ========================================================================== */
/*                        Chain Resolution (1-008b)                           */
/* ========================================================================== */

/* {{{ autodraw_execute_single
 * Executes a single auto-draw effect.
 * Returns the number of cards actually drawn.
 * Marks the source card as spent before drawing.
 */
int autodraw_execute_single(Game* game, Player* player,
                             AutoDrawCandidate* candidate,
                             int current_iteration) {
    if (!game || !player || !candidate || !candidate->card ||
        !candidate->draw_effect || !player->deck) {
        return 0;
    }

    /* Mark as spent FIRST to prevent re-triggering during chain */
    autodraw_mark_spent(candidate->card);

    /* Get draw count from effect */
    int draw_count = candidate->draw_effect->value;
    if (draw_count <= 0) {
        draw_count = 1;  /* Default to 1 if not specified */
    }

    int actually_drawn = 0;

    for (int i = 0; i < draw_count; i++) {
        /* Draw from deck to hand */
        CardInstance* drawn = deck_draw_top(player->deck);
        if (!drawn) {
            break;  /* No more cards available */
        }

        actually_drawn++;

        /* Emit card drawn event */
        AutoDrawEvent card_event = {
            .type = AUTODRAW_EVENT_CARD,
            .source = candidate->card,
            .drawn = drawn,
            .chain_iteration = current_iteration,
            .total_drawn = 0  /* Will be updated by caller */
        };
        autodraw_emit_event(game, player, &card_event);
    }

    return actually_drawn;
}
/* }}} */

/* {{{ autodraw_resolve_chain
 * Resolves all auto-draw effects in the player's hand.
 * Chains can occur when newly drawn cards also have draw effects.
 * Returns result code indicating success or reason for stopping.
 */
AutoDrawResult autodraw_resolve_chain(Game* game, Player* player) {
    if (!game || !player || !player->deck) {
        return AUTODRAW_ERROR_INVALID;
    }

    AutoDrawState state = {
        .iterations = 0,
        .total_drawn = 0,
        .awaiting_input = false
    };

    /* Emit chain start event */
    AutoDrawEvent start_event = {
        .type = AUTODRAW_EVENT_START,
        .source = NULL,
        .drawn = NULL,
        .chain_iteration = 0,
        .total_drawn = 0
    };
    autodraw_emit_event(game, player, &start_event);

    bool found_new;
    do {
        found_new = false;
        state.iterations++;

        /* Safety check - prevent infinite loops */
        if (state.iterations > AUTODRAW_MAX_ITERATIONS) {
            /* Emit complete event with current totals */
            AutoDrawEvent error_event = {
                .type = AUTODRAW_EVENT_COMPLETE,
                .source = NULL,
                .drawn = NULL,
                .chain_iteration = state.iterations,
                .total_drawn = state.total_drawn
            };
            autodraw_emit_event(game, player, &error_event);
            return AUTODRAW_ERROR_MAX_ITER;
        }

        /* Find eligible cards in current hand */
        AutoDrawCandidate candidates[AUTODRAW_MAX_CANDIDATES];
        int eligible_count = autodraw_find_eligible(
            player->deck->hand, player->deck->hand_count,
            candidates, AUTODRAW_MAX_CANDIDATES);

        /* Execute each eligible card's draw effect */
        for (int i = 0; i < eligible_count; i++) {
            /* Emit trigger event */
            AutoDrawEvent trigger_event = {
                .type = AUTODRAW_EVENT_TRIGGER,
                .source = candidates[i].card,
                .drawn = NULL,
                .chain_iteration = state.iterations,
                .total_drawn = state.total_drawn
            };
            autodraw_emit_event(game, player, &trigger_event);

            /* Execute the draw */
            int drawn = autodraw_execute_single(game, player,
                                                 &candidates[i],
                                                 state.iterations);

            if (drawn > 0) {
                found_new = true;
                state.total_drawn += drawn;
            }
        }

    } while (found_new);

    /* Emit chain complete event */
    AutoDrawEvent complete_event = {
        .type = AUTODRAW_EVENT_COMPLETE,
        .source = NULL,
        .drawn = NULL,
        .chain_iteration = state.iterations,
        .total_drawn = state.total_drawn
    };
    autodraw_emit_event(game, player, &complete_event);

    if (state.total_drawn == 0) {
        return AUTODRAW_NO_ELIGIBLE;
    }

    return AUTODRAW_OK;
}
/* }}} */

/* ========================================================================== */
/*                          Event Emission (1-008d)                           */
/* ========================================================================== */

/* {{{ autodraw_register_listener
 * Registers a callback to receive auto-draw events.
 * Used for narrative generation and client display.
 */
void autodraw_register_listener(AutoDrawListener listener, void* context) {
    if (!listener || s_listener_count >= MAX_AUTODRAW_LISTENERS) {
        return;
    }

    /* Check for duplicate */
    for (int i = 0; i < s_listener_count; i++) {
        if (s_listeners[i].listener == listener) {
            return;  /* Already registered */
        }
    }

    s_listeners[s_listener_count].listener = listener;
    s_listeners[s_listener_count].context = context;
    s_listener_count++;
}
/* }}} */

/* {{{ autodraw_unregister_listener
 * Removes a previously registered listener.
 */
void autodraw_unregister_listener(AutoDrawListener listener) {
    if (!listener) {
        return;
    }

    for (int i = 0; i < s_listener_count; i++) {
        if (s_listeners[i].listener == listener) {
            /* Shift remaining listeners down */
            for (int j = i; j < s_listener_count - 1; j++) {
                s_listeners[j] = s_listeners[j + 1];
            }
            s_listener_count--;
            return;
        }
    }
}
/* }}} */

/* {{{ autodraw_clear_listeners
 * Removes all registered listeners.
 */
void autodraw_clear_listeners(void) {
    s_listener_count = 0;
    memset(s_listeners, 0, sizeof(s_listeners));
}
/* }}} */

/* {{{ autodraw_emit_event
 * Emits an auto-draw event to all registered listeners.
 */
void autodraw_emit_event(Game* game, Player* player, AutoDrawEvent* event) {
    if (!event) {
        return;
    }

    for (int i = 0; i < s_listener_count; i++) {
        if (s_listeners[i].listener) {
            s_listeners[i].listener(game, player, event,
                                     s_listeners[i].context);
        }
    }
}
/* }}} */
