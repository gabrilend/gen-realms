/* 07-effects.c - Effect execution system implementation
 *
 * Implements the effect dispatch table and handler functions. Effects are
 * the core mechanic for card abilities - each card can have primary effects,
 * ally effects (triggered by faction matches), and scrap effects.
 *
 * The dispatch table approach allows O(1) routing from EffectType to handler,
 * and makes it easy to add new effect types without modifying execution logic.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "07-effects.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                           Static State                                     */
/* ========================================================================== */

/* Event callbacks for narrative hooks */
static EffectCallback s_callbacks[MAX_EFFECT_CALLBACKS];
static int s_callback_count = 0;

/* Effect context per player (indexed by player ID) */
static EffectContext s_contexts[MAX_PLAYERS];

/* Dispatch table initialization flag */
static bool s_initialized = false;

/* ========================================================================== */
/*                          Forward Declarations                              */
/* ========================================================================== */

/* Handler functions for each effect type */
static void handle_trade(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_combat(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_authority(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_draw(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_discard(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_scrap_trade_row(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_scrap_hand(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_top_deck(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_d10_up(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_d10_down(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_destroy_base(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_copy_ship(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_acquire_free(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_upgrade_attack(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_upgrade_trade(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_upgrade_auth(Game* game, Player* player, Effect* effect, CardInstance* source);
static void handle_spawn(Game* game, Player* player, Effect* effect, CardInstance* source);

/* ========================================================================== */
/*                           Dispatch Table                                   */
/* ========================================================================== */

/* {{{ Dispatch table
 * Maps EffectType enum values to handler functions.
 * Using designated initializers for clarity and safety.
 */
static EffectHandler s_dispatch_table[EFFECT_TYPE_COUNT] = {
    [EFFECT_TRADE]          = handle_trade,
    [EFFECT_COMBAT]         = handle_combat,
    [EFFECT_AUTHORITY]      = handle_authority,
    [EFFECT_DRAW]           = handle_draw,
    [EFFECT_DISCARD]        = handle_discard,
    [EFFECT_SCRAP_TRADE_ROW] = handle_scrap_trade_row,
    [EFFECT_SCRAP_HAND]     = handle_scrap_hand,
    [EFFECT_TOP_DECK]       = handle_top_deck,
    [EFFECT_D10_UP]         = handle_d10_up,
    [EFFECT_D10_DOWN]       = handle_d10_down,
    [EFFECT_DESTROY_BASE]   = handle_destroy_base,
    [EFFECT_COPY_SHIP]      = handle_copy_ship,
    [EFFECT_ACQUIRE_FREE]   = handle_acquire_free,
    [EFFECT_UPGRADE_ATTACK] = handle_upgrade_attack,
    [EFFECT_UPGRADE_TRADE]  = handle_upgrade_trade,
    [EFFECT_UPGRADE_AUTH]   = handle_upgrade_auth,
    [EFFECT_SPAWN]          = handle_spawn,
};
/* }}} */

/* ========================================================================== */
/*                           Initialization                                   */
/* ========================================================================== */

/* {{{ effects_init
 * Initializes the effect system. Clears contexts and callbacks.
 */
void effects_init(void) {
    /* Clear all contexts */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        s_contexts[i].next_ship_free = false;
        s_contexts[i].free_ship_max_cost = 0;
        s_contexts[i].next_ship_to_top = false;
        s_contexts[i].pending_draws = 0;
    }

    /* Clear callbacks */
    s_callback_count = 0;
    for (int i = 0; i < MAX_EFFECT_CALLBACKS; i++) {
        s_callbacks[i].callback = NULL;
        s_callbacks[i].context = NULL;
    }

    s_initialized = true;
}
/* }}} */

/* ========================================================================== */
/*                          Effect Execution                                  */
/* ========================================================================== */

/* {{{ fire_callbacks
 * Fires all registered callbacks after an effect executes.
 */
static void fire_callbacks(Game* game, Player* player,
                           CardInstance* source, Effect* effect) {
    for (int i = 0; i < s_callback_count; i++) {
        if (s_callbacks[i].callback) {
            s_callbacks[i].callback(game, player, source, effect,
                                    s_callbacks[i].context);
        }
    }
}
/* }}} */

/* {{{ effects_execute
 * Executes a single effect. Routes to the appropriate handler via dispatch table.
 */
void effects_execute(Game* game, Player* player,
                     Effect* effect, CardInstance* source) {
    if (!game || !player || !effect) {
        return;
    }

    /* Ensure initialization */
    if (!s_initialized) {
        effects_init();
    }

    /* Validate effect type */
    if (effect->type < 0 || effect->type >= EFFECT_TYPE_COUNT) {
        return;
    }

    /* Get handler from dispatch table */
    EffectHandler handler = s_dispatch_table[effect->type];
    if (!handler) {
        /* No handler registered for this effect type */
        return;
    }

    /* Execute the effect */
    handler(game, player, effect, source);

    /* Fire event callbacks for narrative hooks */
    fire_callbacks(game, player, source, effect);
}
/* }}} */

/* {{{ effects_execute_all
 * Executes an array of effects in order.
 */
void effects_execute_all(Game* game, Player* player,
                         Effect* effects, int count, CardInstance* source) {
    if (!game || !player || !effects || count <= 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        effects_execute(game, player, &effects[i], source);
    }
}
/* }}} */

/* {{{ effects_execute_card
 * Executes all primary effects of a card, then checks ally abilities.
 * Called when a card is played from hand.
 */
void effects_execute_card(Game* game, Player* player, CardInstance* card) {
    if (!game || !player || !card || !card->type) {
        return;
    }

    CardType* type = card->type;

    /* Execute primary effects */
    if (type->effects && type->effect_count > 0) {
        effects_execute_all(game, player, type->effects,
                           type->effect_count, card);
    }

    /* Check and execute ally abilities */
    if (type->ally_effects && type->ally_effect_count > 0) {
        /* Check if another card of same faction was already played */
        if (player_has_faction_ally(player, type->faction)) {
            effects_execute_all(game, player, type->ally_effects,
                               type->ally_effect_count, card);
        }
    }

    /* Mark this faction as played for future ally checks */
    player_mark_faction_played(player, type->faction);
}
/* }}} */

/* {{{ effects_execute_ally
 * Re-checks ally abilities for all played cards.
 * Called when a new card is played that might enable ally effects.
 */
void effects_execute_ally(Game* game, Player* player, CardInstance* card) {
    if (!game || !player || !card || !card->type) {
        return;
    }

    /* This card's ally effects should trigger if there's already
     * a card of the same faction in play */
    CardType* type = card->type;
    if (type->ally_effects && type->ally_effect_count > 0) {
        if (player_has_faction_ally(player, type->faction)) {
            effects_execute_all(game, player, type->ally_effects,
                               type->ally_effect_count, card);
        }
    }
}
/* }}} */

/* {{{ effects_execute_scrap
 * Executes the scrap effects of a card when it is scrapped.
 * Called when a player chooses to scrap a card from hand or play.
 */
void effects_execute_scrap(Game* game, Player* player, CardInstance* card) {
    if (!game || !player || !card || !card->type) {
        return;
    }

    CardType* type = card->type;

    /* Execute scrap effects */
    if (type->scrap_effects && type->scrap_effect_count > 0) {
        effects_execute_all(game, player, type->scrap_effects,
                           type->scrap_effect_count, card);
    }
}
/* }}} */

/* ========================================================================== */
/*                          Event Callbacks                                   */
/* ========================================================================== */

/* {{{ effects_register_callback
 * Registers a callback to be fired after each effect execution.
 * Used for Phase 5 narrative generation hooks.
 */
void effects_register_callback(EffectEventFunc callback, void* context) {
    if (!callback || s_callback_count >= MAX_EFFECT_CALLBACKS) {
        return;
    }

    s_callbacks[s_callback_count].callback = callback;
    s_callbacks[s_callback_count].context = context;
    s_callback_count++;
}
/* }}} */

/* {{{ effects_unregister_callback
 * Removes a previously registered callback.
 */
void effects_unregister_callback(EffectEventFunc callback) {
    for (int i = 0; i < s_callback_count; i++) {
        if (s_callbacks[i].callback == callback) {
            /* Shift remaining callbacks down */
            for (int j = i; j < s_callback_count - 1; j++) {
                s_callbacks[j] = s_callbacks[j + 1];
            }
            s_callback_count--;
            return;
        }
    }
}
/* }}} */

/* {{{ effects_clear_callbacks
 * Removes all registered callbacks.
 */
void effects_clear_callbacks(void) {
    s_callback_count = 0;
    for (int i = 0; i < MAX_EFFECT_CALLBACKS; i++) {
        s_callbacks[i].callback = NULL;
        s_callbacks[i].context = NULL;
    }
}
/* }}} */

/* ========================================================================== */
/*                          Context Management                                */
/* ========================================================================== */

/* {{{ effects_get_context
 * Returns the effect context for a player.
 */
EffectContext* effects_get_context(Player* player) {
    if (!player || player->id < 0 || player->id >= MAX_PLAYERS) {
        return NULL;
    }
    return &s_contexts[player->id];
}
/* }}} */

/* {{{ effects_reset_context
 * Resets a player's effect context. Called at start of turn.
 */
void effects_reset_context(Player* player) {
    if (!player || player->id < 0 || player->id >= MAX_PLAYERS) {
        return;
    }

    EffectContext* ctx = &s_contexts[player->id];
    ctx->next_ship_free = false;
    ctx->free_ship_max_cost = 0;
    ctx->next_ship_to_top = false;
    ctx->pending_draws = 0;
}
/* }}} */

/* ========================================================================== */
/*                          Effect Handlers                                   */
/* ========================================================================== */

/* {{{ handle_trade
 * Adds trade (gold/coins) to the player's pool for this turn.
 * Includes upgrade bonus from CardInstance.
 */
static void handle_trade(Game* game, Player* player,
                         Effect* effect, CardInstance* source) {
    int bonus = source ? source->trade_bonus : 0;
    player_add_trade(player, effect->value + bonus);
}
/* }}} */

/* {{{ handle_combat
 * Adds combat (attack power) to the player's pool for this turn.
 * Includes upgrade bonus from CardInstance.
 */
static void handle_combat(Game* game, Player* player,
                          Effect* effect, CardInstance* source) {
    int bonus = source ? source->attack_bonus : 0;
    player_add_combat(player, effect->value + bonus);
}
/* }}} */

/* {{{ handle_authority
 * Adds authority (health) to the player.
 * Includes upgrade bonus from CardInstance.
 */
static void handle_authority(Game* game, Player* player,
                             Effect* effect, CardInstance* source) {
    int bonus = source ? source->authority_bonus : 0;
    player_add_authority(player, effect->value + bonus);
}
/* }}} */

/* {{{ handle_draw
 * Draws cards from the player's deck to their hand.
 * Simple immediate draw (auto-draw resolution is in 1-008).
 */
static void handle_draw(Game* game, Player* player,
                        Effect* effect, CardInstance* source) {
    player_draw_cards(player, effect->value);
}
/* }}} */

/* {{{ handle_discard
 * Forces the opponent to discard cards.
 * Creates a pending action for the opponent to choose which cards to discard.
 */
static void handle_discard(Game* game, Player* player,
                           Effect* effect, CardInstance* source) {
    if (!game || !player || !effect || effect->value <= 0) {
        return;
    }

    /* Get opponent - in 2-player game, it's always the other player */
    Player* opponent = game_get_opponent(game, 0);
    if (!opponent) {
        return;
    }

    /* Create pending discard action for opponent */
    game_request_discard(game, opponent->id, effect->value);
}
/* }}} */

/* {{{ handle_scrap_trade_row
 * Allows player to scrap a card from the trade row.
 * Creates a pending action for the player to choose which card to scrap.
 */
static void handle_scrap_trade_row(Game* game, Player* player,
                                   Effect* effect, CardInstance* source) {
    if (!game || !player || !effect) {
        return;
    }

    int count = effect->value > 0 ? effect->value : 1;

    /* Create pending scrap trade row action */
    game_request_scrap_trade_row(game, player->id, count);
}
/* }}} */

/* {{{ handle_scrap_hand
 * Allows player to scrap a card from their hand or discard pile.
 * Creates a pending action for the player to choose which card to scrap.
 * Note: Scrapping decrements d10 (handled in resolution).
 */
static void handle_scrap_hand(Game* game, Player* player,
                              Effect* effect, CardInstance* source) {
    if (!game || !player || !effect) {
        return;
    }

    int count = effect->value > 0 ? effect->value : 1;

    /* Create pending scrap hand/discard action */
    game_request_scrap_hand_discard(game, player->id, count);
}
/* }}} */

/* {{{ handle_top_deck
 * Allows player to put a card from discard on top of their deck.
 * Creates a pending action for the player to choose which card.
 */
static void handle_top_deck(Game* game, Player* player,
                            Effect* effect, CardInstance* source) {
    if (!game || !player || !effect) {
        return;
    }

    int count = effect->value > 0 ? effect->value : 1;

    /* Create pending top deck action */
    game_request_top_deck(game, player->id, count);
}
/* }}} */

/* {{{ handle_d10_up
 * Directly increments the d10 deck flow tracker.
 * Some cards provide this as a bonus effect.
 */
static void handle_d10_up(Game* game, Player* player,
                          Effect* effect, CardInstance* source) {
    for (int i = 0; i < effect->value; i++) {
        player_d10_increment(player);
    }
}
/* }}} */

/* {{{ handle_d10_down
 * Directly decrements the d10 deck flow tracker.
 * Rare effect, usually from scrapping.
 */
static void handle_d10_down(Game* game, Player* player,
                            Effect* effect, CardInstance* source) {
    for (int i = 0; i < effect->value; i++) {
        player_d10_decrement(player);
    }
}
/* }}} */

/* {{{ handle_destroy_base
 * Allows player to destroy an opponent's base without combat.
 */
static void handle_destroy_base(Game* game, Player* player,
                                Effect* effect, CardInstance* source) {
    /* TODO: Implement base destroy choice (requires action queue) */
}
/* }}} */

/* {{{ handle_copy_ship
 * Allows player to copy another ship's effects.
 */
static void handle_copy_ship(Game* game, Player* player,
                             Effect* effect, CardInstance* source) {
    /* TODO: Implement ship copy choice (requires action queue) */
}
/* }}} */

/* {{{ handle_acquire_free
 * Sets flag for next ship to be acquired for free.
 */
static void handle_acquire_free(Game* game, Player* player,
                                Effect* effect, CardInstance* source) {
    EffectContext* ctx = effects_get_context(player);
    if (ctx) {
        ctx->next_ship_free = true;
        ctx->free_ship_max_cost = effect->value;  /* 0 = any cost */
    }
}
/* }}} */

/* {{{ handle_upgrade_attack
 * Permanently upgrades a card's attack bonus.
 */
static void handle_upgrade_attack(Game* game, Player* player,
                                  Effect* effect, CardInstance* source) {
    /* TODO: Implement upgrade target choice (requires action queue) */
    /* When target is selected, call:
     * card_instance_apply_upgrade(target, EFFECT_UPGRADE_ATTACK, effect->value);
     */
}
/* }}} */

/* {{{ handle_upgrade_trade
 * Permanently upgrades a card's trade bonus.
 */
static void handle_upgrade_trade(Game* game, Player* player,
                                 Effect* effect, CardInstance* source) {
    /* TODO: Implement upgrade target choice (requires action queue) */
}
/* }}} */

/* {{{ handle_upgrade_auth
 * Permanently upgrades a card's authority bonus.
 */
static void handle_upgrade_auth(Game* game, Player* player,
                                Effect* effect, CardInstance* source) {
    /* TODO: Implement upgrade target choice (requires action queue) */
}
/* }}} */

/* {{{ handle_spawn
 * Spawns a unit from a base.
 * The effect's target_card_id specifies which unit type to create.
 * The new unit goes to the player's discard pile.
 */
static void handle_spawn(Game* game, Player* player,
                         Effect* effect, CardInstance* source) {
    (void)source;  /* Base context not needed, effect has target */

    if (!game || !player || !effect || !effect->target_card_id) {
        return;
    }

    /* Look up the unit type to spawn */
    CardType* unit_type = game_find_card_type(game, effect->target_card_id);
    if (!unit_type) {
        return;  /* Unknown card type */
    }

    /* Create the unit instance(s) */
    int count = effect->value > 0 ? effect->value : 1;
    for (int i = 0; i < count; i++) {
        CardInstance* unit = card_instance_create(unit_type);
        if (unit) {
            deck_add_to_discard(player->deck, unit);
        }
    }
}
/* }}} */
