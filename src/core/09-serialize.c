/* 09-serialize.c - Game state JSON serialization implementation
 *
 * Implements JSON serialization for game state transmission. Uses cJSON library
 * for JSON construction. Player-specific views hide opponent hand contents while
 * exposing all public information needed for gameplay decisions.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "09-serialize.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/*                          Effect Serialization                              */
/* ========================================================================== */

/* {{{ serialize_effect */
cJSON* serialize_effect(Effect* effect) {
    if (!effect) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    cJSON_AddStringToObject(json, "type", effect_type_to_string(effect->type));
    cJSON_AddNumberToObject(json, "value", effect->value);

    if (effect->target_card_id) {
        cJSON_AddStringToObject(json, "target_card_id", effect->target_card_id);
    }

    return json;
}
/* }}} */

/* {{{ serialize_effect_array */
static cJSON* serialize_effect_array(Effect* effects, int count) {
    cJSON* array = cJSON_CreateArray();
    if (!array) return NULL;

    for (int i = 0; i < count; i++) {
        cJSON* effect_json = serialize_effect(&effects[i]);
        if (effect_json) {
            cJSON_AddItemToArray(array, effect_json);
        }
    }

    return array;
}
/* }}} */

/* ========================================================================== */
/*                          Card Serialization                                */
/* ========================================================================== */

/* {{{ serialize_card_type */
cJSON* serialize_card_type(CardType* type) {
    if (!type) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Identity */
    cJSON_AddStringToObject(json, "id", type->id ? type->id : "");
    cJSON_AddStringToObject(json, "name", type->name ? type->name : "");

    if (type->flavor) {
        cJSON_AddStringToObject(json, "flavor", type->flavor);
    }

    /* Classification */
    cJSON_AddNumberToObject(json, "cost", type->cost);
    cJSON_AddStringToObject(json, "faction", faction_to_string(type->faction));
    cJSON_AddStringToObject(json, "kind", card_kind_to_string(type->kind));

    /* Base stats */
    if (type->kind == CARD_KIND_BASE) {
        cJSON_AddNumberToObject(json, "defense", type->defense);
        cJSON_AddBoolToObject(json, "is_outpost", type->is_outpost);
    }

    /* Effects */
    if (type->effect_count > 0) {
        cJSON_AddItemToObject(json, "effects",
            serialize_effect_array(type->effects, type->effect_count));
    }

    if (type->ally_effect_count > 0) {
        cJSON_AddItemToObject(json, "ally_effects",
            serialize_effect_array(type->ally_effects, type->ally_effect_count));
    }

    if (type->scrap_effect_count > 0) {
        cJSON_AddItemToObject(json, "scrap_effects",
            serialize_effect_array(type->scrap_effects, type->scrap_effect_count));
    }

    /* Spawning */
    if (type->spawns_id) {
        cJSON_AddStringToObject(json, "spawns_id", type->spawns_id);
    }

    return json;
}
/* }}} */

/* {{{ serialize_card_instance */
cJSON* serialize_card_instance(CardInstance* card) {
    if (!card) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Instance identity */
    cJSON_AddStringToObject(json, "instance_id",
        card->instance_id ? card->instance_id : "");

    /* Type reference (just the ID, client should have type definitions) */
    if (card->type) {
        cJSON_AddStringToObject(json, "card_id",
            card->type->id ? card->type->id : "");
        cJSON_AddStringToObject(json, "name",
            card->type->name ? card->type->name : "");
        cJSON_AddStringToObject(json, "faction",
            faction_to_string(card->type->faction));
        cJSON_AddStringToObject(json, "kind",
            card_kind_to_string(card->type->kind));
        cJSON_AddNumberToObject(json, "cost", card->type->cost);

        /* Include base defense for bases */
        if (card->type->kind == CARD_KIND_BASE) {
            cJSON_AddNumberToObject(json, "defense", card->type->defense);
        }
    }

    /* Upgrades */
    if (card->attack_bonus != 0) {
        cJSON_AddNumberToObject(json, "attack_bonus", card->attack_bonus);
    }
    if (card->trade_bonus != 0) {
        cJSON_AddNumberToObject(json, "trade_bonus", card->trade_bonus);
    }
    if (card->authority_bonus != 0) {
        cJSON_AddNumberToObject(json, "authority_bonus", card->authority_bonus);
    }

    /* Visual state */
    cJSON_AddNumberToObject(json, "image_seed", card->image_seed);
    cJSON_AddBoolToObject(json, "needs_regen", card->needs_regen);

    /* Base-specific state */
    if (card->type && card->type->kind == CARD_KIND_BASE) {
        cJSON_AddStringToObject(json, "placement",
            base_placement_to_string(card->placement));
        cJSON_AddBoolToObject(json, "deployed", card->deployed);
        cJSON_AddNumberToObject(json, "damage_taken", card->damage_taken);
    }

    return json;
}
/* }}} */

/* {{{ serialize_card_array */
cJSON* serialize_card_array(CardInstance** cards, int count) {
    cJSON* array = cJSON_CreateArray();
    if (!array) return NULL;

    for (int i = 0; i < count; i++) {
        if (cards[i]) {
            cJSON* card_json = serialize_card_instance(cards[i]);
            if (card_json) {
                cJSON_AddItemToArray(array, card_json);
            }
        }
    }

    return array;
}
/* }}} */

/* ========================================================================== */
/*                          Player Serialization                              */
/* ========================================================================== */

/* {{{ serialize_bases */
static cJSON* serialize_bases(Player* player) {
    cJSON* bases = cJSON_CreateObject();
    if (!bases || !player || !player->deck) return bases;

    /* Frontier bases */
    cJSON_AddItemToObject(bases, "frontier",
        serialize_card_array(player->deck->frontier_bases,
                            player->deck->frontier_base_count));

    /* Interior bases */
    cJSON_AddItemToObject(bases, "interior",
        serialize_card_array(player->deck->interior_bases,
                            player->deck->interior_base_count));

    return bases;
}
/* }}} */

/* {{{ serialize_player_public */
cJSON* serialize_player_public(Player* player) {
    if (!player) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Identity */
    cJSON_AddNumberToObject(json, "id", player->id);
    cJSON_AddStringToObject(json, "name", player->name ? player->name : "");

    /* Health */
    cJSON_AddNumberToObject(json, "authority", player->authority);

    /* Deck flow tracker (visible to all per game rules) */
    cJSON_AddNumberToObject(json, "d10", player->d10);
    cJSON_AddNumberToObject(json, "d4", player->d4);

    /* Public counts (hide actual hand contents) */
    if (player->deck) {
        cJSON_AddNumberToObject(json, "hand_count", player->deck->hand_count);
        cJSON_AddNumberToObject(json, "deck_count", player->deck->draw_pile_count);
        cJSON_AddNumberToObject(json, "discard_count", player->deck->discard_count);
        cJSON_AddNumberToObject(json, "played_count", player->deck->played_count);

        /* Discard pile contents are public (visible to all) */
        cJSON_AddItemToObject(json, "discard",
            serialize_card_array(player->deck->discard, player->deck->discard_count));

        /* Played cards are public */
        if (player->deck->played_count > 0) {
            cJSON_AddItemToObject(json, "played",
                serialize_card_array(player->deck->played, player->deck->played_count));
        }
    }

    /* Bases are public */
    cJSON_AddItemToObject(json, "bases", serialize_bases(player));

    return json;
}
/* }}} */

/* {{{ serialize_player_private */
cJSON* serialize_player_private(Player* player) {
    if (!player) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Identity */
    cJSON_AddNumberToObject(json, "id", player->id);
    cJSON_AddStringToObject(json, "name", player->name ? player->name : "");

    /* Health */
    cJSON_AddNumberToObject(json, "authority", player->authority);

    /* Per-turn resources */
    cJSON_AddNumberToObject(json, "trade", player->trade);
    cJSON_AddNumberToObject(json, "combat", player->combat);

    /* Deck flow tracker */
    cJSON_AddNumberToObject(json, "d10", player->d10);
    cJSON_AddNumberToObject(json, "d4", player->d4);

    /* Full hand contents (private!) */
    if (player->deck) {
        cJSON_AddItemToObject(json, "hand",
            serialize_card_array(player->deck->hand, player->deck->hand_count));
        cJSON_AddNumberToObject(json, "deck_count", player->deck->draw_pile_count);
        cJSON_AddNumberToObject(json, "discard_count", player->deck->discard_count);

        /* Include discard pile contents (player can see their own) */
        cJSON_AddItemToObject(json, "discard",
            serialize_card_array(player->deck->discard, player->deck->discard_count));

        /* Played cards */
        cJSON_AddItemToObject(json, "played",
            serialize_card_array(player->deck->played, player->deck->played_count));
    }

    /* Bases */
    cJSON_AddItemToObject(json, "bases", serialize_bases(player));

    /* Faction tracking */
    cJSON* factions = cJSON_CreateArray();
    for (int i = 0; i < FACTION_COUNT; i++) {
        if (player->factions_played[i]) {
            cJSON_AddItemToArray(factions,
                cJSON_CreateString(faction_to_string((Faction)i)));
        }
    }
    cJSON_AddItemToObject(json, "factions_played", factions);

    return json;
}
/* }}} */

/* {{{ serialize_player_for_view */
cJSON* serialize_player_for_view(Player* player, ViewPerspective view) {
    if (!player) return NULL;

    switch (view) {
        case VIEW_SELF:
        case VIEW_SPECTATOR:
            /* Full info for self and spectators */
            return serialize_player_private(player);

        case VIEW_OPPONENT:
        default:
            /* Limited info for opponents */
            return serialize_player_public(player);
    }
}
/* }}} */

/* ========================================================================== */
/*                          Trade Row Serialization                           */
/* ========================================================================== */

/* {{{ serialize_trade_row */
cJSON* serialize_trade_row(TradeRow* row) {
    if (!row) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Slots */
    cJSON* slots = cJSON_CreateArray();
    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i]) {
            cJSON* slot = serialize_card_instance(row->slots[i]);
            cJSON_AddNumberToObject(slot, "slot", i);
            cJSON_AddItemToArray(slots, slot);
        } else {
            cJSON_AddItemToArray(slots, cJSON_CreateNull());
        }
    }
    cJSON_AddItemToObject(json, "slots", slots);

    /* Explorer info */
    if (row->explorer_type) {
        cJSON* explorer = cJSON_CreateObject();
        cJSON_AddStringToObject(explorer, "card_id", row->explorer_type->id);
        cJSON_AddStringToObject(explorer, "name", row->explorer_type->name);
        cJSON_AddNumberToObject(explorer, "cost", EXPLORER_COST);
        cJSON_AddBoolToObject(explorer, "available", true);
        cJSON_AddItemToObject(json, "explorer", explorer);
    }

    /* Deck remaining */
    cJSON_AddNumberToObject(json, "deck_remaining", row->trade_deck_count);

    return json;
}
/* }}} */

/* ========================================================================== */
/*                          Game State Serialization                          */
/* ========================================================================== */

/* {{{ serialize_game_for_player */
cJSON* serialize_game_for_player(Game* game, int player_id) {
    if (!game) return NULL;
    if (player_id < 0 || player_id >= game->player_count) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Turn info */
    cJSON_AddNumberToObject(json, "turn", game->turn_number);
    cJSON_AddStringToObject(json, "phase", game_phase_to_string(game->phase));
    cJSON_AddNumberToObject(json, "active_player", game->active_player);

    /* Am I the active player? */
    cJSON_AddBoolToObject(json, "is_your_turn", game->active_player == player_id);

    /* Game end state */
    cJSON_AddBoolToObject(json, "game_over", game->game_over);
    if (game->game_over) {
        cJSON_AddNumberToObject(json, "winner", game->winner);
    }

    /* "you" - full private info for requesting player */
    Player* self = game->players[player_id];
    cJSON_AddItemToObject(json, "you", serialize_player_private(self));

    /* "opponents" - public info only for other players */
    cJSON* opponents = cJSON_CreateArray();
    for (int i = 0; i < game->player_count; i++) {
        if (i != player_id && game->players[i]) {
            cJSON* opp = serialize_player_public(game->players[i]);
            cJSON_AddItemToArray(opponents, opp);
        }
    }
    cJSON_AddItemToObject(json, "opponents", opponents);

    /* Trade row */
    cJSON_AddItemToObject(json, "trade_row", serialize_trade_row(game->trade_row));

    return json;
}
/* }}} */

/* {{{ serialize_game_full */
cJSON* serialize_game_full(Game* game) {
    if (!game) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Turn info */
    cJSON_AddNumberToObject(json, "turn", game->turn_number);
    cJSON_AddStringToObject(json, "phase", game_phase_to_string(game->phase));
    cJSON_AddNumberToObject(json, "active_player", game->active_player);
    cJSON_AddNumberToObject(json, "player_count", game->player_count);

    /* Game end state */
    cJSON_AddBoolToObject(json, "game_over", game->game_over);
    if (game->game_over) {
        cJSON_AddNumberToObject(json, "winner", game->winner);
    }

    /* All players with full info */
    cJSON* players = cJSON_CreateArray();
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i]) {
            cJSON_AddItemToArray(players, serialize_player_private(game->players[i]));
        }
    }
    cJSON_AddItemToObject(json, "players", players);

    /* Trade row */
    cJSON_AddItemToObject(json, "trade_row", serialize_trade_row(game->trade_row));

    return json;
}
/* }}} */

/* {{{ serialize_game_for_spectator */
cJSON* serialize_game_for_spectator(Game* game) {
    if (!game) return NULL;

    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;

    /* Turn info */
    cJSON_AddNumberToObject(json, "turn", game->turn_number);
    cJSON_AddStringToObject(json, "phase", game_phase_to_string(game->phase));
    cJSON_AddNumberToObject(json, "active_player", game->active_player);
    cJSON_AddNumberToObject(json, "player_count", game->player_count);

    /* Spectator flag */
    cJSON_AddBoolToObject(json, "is_spectator", true);

    /* Game end state */
    cJSON_AddBoolToObject(json, "game_over", game->game_over);
    if (game->game_over) {
        cJSON_AddNumberToObject(json, "winner", game->winner);
    }

    /* All players with full info (spectators see everything) */
    cJSON* players = cJSON_CreateArray();
    for (int i = 0; i < game->player_count; i++) {
        if (game->players[i]) {
            cJSON* player_json = serialize_player_for_view(game->players[i], VIEW_SPECTATOR);
            cJSON_AddItemToArray(players, player_json);
        }
    }
    cJSON_AddItemToObject(json, "players", players);

    /* Trade row */
    cJSON_AddItemToObject(json, "trade_row", serialize_trade_row(game->trade_row));

    return json;
}
/* }}} */

/* ========================================================================== */
/*                          Action Deserialization                            */
/* ========================================================================== */

/* {{{ parse_action_type */
static ActionType parse_action_type(const char* type_str) {
    if (!type_str) return ACTION_END_TURN;

    if (strcmp(type_str, "play_card") == 0) return ACTION_PLAY_CARD;
    if (strcmp(type_str, "buy_card") == 0) return ACTION_BUY_CARD;
    if (strcmp(type_str, "buy_explorer") == 0) return ACTION_BUY_EXPLORER;
    if (strcmp(type_str, "attack_player") == 0) return ACTION_ATTACK_PLAYER;
    if (strcmp(type_str, "attack_base") == 0) return ACTION_ATTACK_BASE;
    if (strcmp(type_str, "scrap_hand") == 0) return ACTION_SCRAP_HAND;
    if (strcmp(type_str, "scrap_discard") == 0) return ACTION_SCRAP_DISCARD;
    if (strcmp(type_str, "scrap_trade_row") == 0) return ACTION_SCRAP_TRADE_ROW;
    if (strcmp(type_str, "end_turn") == 0) return ACTION_END_TURN;

    return ACTION_END_TURN;  /* Default fallback */
}
/* }}} */

/* {{{ deserialize_action */
Action* deserialize_action(cJSON* json) {
    if (!json || !cJSON_IsObject(json)) return NULL;

    Action* action = action_create(ACTION_END_TURN);
    if (!action) return NULL;

    /* Type (required) */
    cJSON* type_json = cJSON_GetObjectItem(json, "type");
    if (cJSON_IsString(type_json)) {
        action->type = parse_action_type(type_json->valuestring);
    }

    /* Slot (for trade row operations) */
    cJSON* slot_json = cJSON_GetObjectItem(json, "slot");
    if (cJSON_IsNumber(slot_json)) {
        action->slot = (int)slot_json->valuedouble;
    }

    /* Card ID (for play/scrap operations) */
    cJSON* card_json = cJSON_GetObjectItem(json, "card_id");
    if (cJSON_IsString(card_json)) {
        action->card_instance_id = strdup(card_json->valuestring);
    }

    /* Target player (for attacks) */
    cJSON* target_json = cJSON_GetObjectItem(json, "target");
    if (cJSON_IsNumber(target_json)) {
        action->target_player = (int)target_json->valuedouble;
    }

    /* Amount (for attacks) */
    cJSON* amount_json = cJSON_GetObjectItem(json, "amount");
    if (cJSON_IsNumber(amount_json)) {
        action->amount = (int)amount_json->valuedouble;
    }

    return action;
}
/* }}} */

/* ========================================================================== */
/*                            Utility Functions                               */
/* ========================================================================== */

/* {{{ game_state_to_string */
char* game_state_to_string(Game* game, int player_id) {
    cJSON* json = serialize_game_for_player(game, player_id);
    if (!json) return NULL;

    char* str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    return str;
}
/* }}} */

/* {{{ game_state_to_string_pretty */
char* game_state_to_string_pretty(Game* game, int player_id) {
    cJSON* json = serialize_game_for_player(game, player_id);
    if (!json) return NULL;

    char* str = cJSON_Print(json);
    cJSON_Delete(json);

    return str;
}
/* }}} */
