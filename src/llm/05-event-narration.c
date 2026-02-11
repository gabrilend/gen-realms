/*
 * 05-event-narration.c - Event Narration Implementation
 *
 * Generates dramatic narrative for game events with scaling intensity.
 * Uses prompt templates and world state context for consistent storytelling.
 */

#include "05-event-narration.h"
#include "04-force-description.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// {{{ Event Type Names
static const char* EVENT_TYPE_NAMES[GAME_EVENT_TYPE_COUNT] = {
    "card played",
    "card purchased",
    "attack on player",
    "attack on base",
    "base destroyed",
    "turn start",
    "turn end",
    "game over",
    "ally triggered",
    "card scrapped"
};
// }}}

// {{{ Intensity Words
static const char* INTENSITY_WORDS[] = {
    "quietly",      // LOW
    "dramatically", // MEDIUM
    "furiously",    // HIGH
    "magnificently" // EPIC
};
// }}}

// {{{ strdup_safe
static char* strdup_safe(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}
// }}}

// {{{ event_narration_create
NarrationEvent* event_narration_create(GameEventType type) {
    NarrationEvent* event = malloc(sizeof(NarrationEvent));
    if (event == NULL) {
        return NULL;
    }

    event->type = type;
    event->actor = NULL;
    event->target = NULL;
    event->card = NULL;
    event->base = NULL;
    event->damage = 0;
    event->cost = 0;
    event->turn = 0;
    event->intensity = INTENSITY_MEDIUM;

    return event;
}
// }}}

// {{{ event_narration_free
void event_narration_free(NarrationEvent* event) {
    // NarrationEvent doesn't own any of its pointers, just free the struct
    free(event);
}
// }}}

// {{{ event_type_to_string
const char* event_type_to_string(GameEventType type) {
    if (type < 0 || type >= GAME_EVENT_TYPE_COUNT) {
        return "unknown event";
    }
    return EVENT_TYPE_NAMES[type];
}
// }}}

// {{{ event_narration_get_intensity_word
const char* event_narration_get_intensity_word(NarrationIntensity intensity) {
    if (intensity < INTENSITY_LOW || intensity > INTENSITY_EPIC) {
        return "dramatically";
    }
    return INTENSITY_WORDS[intensity];
}
// }}}

// {{{ event_narration_calculate_intensity
NarrationIntensity event_narration_calculate_intensity(WorldState* state,
                                                        NarrationEvent* event) {
    if (event == NULL) {
        return INTENSITY_MEDIUM;
    }

    // Game over is always epic
    if (event->type == GAME_EVENT_GAME_OVER) {
        return INTENSITY_EPIC;
    }

    // Base destroyed is high intensity
    if (event->type == GAME_EVENT_BASE_DESTROYED) {
        return INTENSITY_HIGH;
    }

    // Check world state tension
    if (state != NULL && state->tension > 0.7f) {
        return INTENSITY_HIGH;
    }

    // Large damage is high intensity
    if (event->damage >= 10) {
        return INTENSITY_HIGH;
    }

    // Small damage or purchases are low intensity
    if (event->damage > 0 && event->damage <= 2) {
        return INTENSITY_LOW;
    }
    if (event->type == GAME_EVENT_CARD_PURCHASED && event->cost <= 2) {
        return INTENSITY_LOW;
    }

    // Turn transitions are low intensity
    if (event->type == GAME_EVENT_TURN_START || event->type == GAME_EVENT_TURN_END) {
        return INTENSITY_LOW;
    }

    return INTENSITY_MEDIUM;
}
// }}}

// {{{ event_narration_build_card_played
char* event_narration_build_card_played(Player* player, CardInstance* card) {
    if (player == NULL || card == NULL || card->type == NULL) {
        return strdup_safe("A card is played.");
    }

    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    const char* player_name = player->name != NULL ? player->name : "A commander";
    const char* card_name = card->type->name != NULL ? card->type->name : "a card";
    const FactionTheme* theme = force_desc_get_theme(card->type->faction);

    prompt_vars_add(vars, "player_name", player_name);
    prompt_vars_add(vars, "card_name", card_name);
    prompt_vars_add(vars, "card_faction", theme->name);

    char effect_str[128];
    snprintf(effect_str, sizeof(effect_str), "joins the battle");
    prompt_vars_add(vars, "card_effect", effect_str);

    char* prompt = prompt_build(PROMPT_CARD_PLAYED, vars);
    prompt_vars_free(vars);

    return prompt;
}
// }}}

// {{{ event_narration_build_purchase
char* event_narration_build_purchase(Player* player, CardInstance* card, int cost) {
    if (player == NULL || card == NULL || card->type == NULL) {
        return strdup_safe("A card is acquired.");
    }

    const char* player_name = player->name != NULL ? player->name : "A commander";
    const char* card_name = card->type->name != NULL ? card->type->name : "a card";
    const FactionTheme* theme = force_desc_get_theme(card->type->faction);

    size_t buffer_size = 512;
    char* prompt = malloc(buffer_size);
    if (prompt == NULL) {
        return NULL;
    }

    snprintf(prompt, buffer_size,
             "%s acquires the %s from the trade row for %d trade. "
             "The %s %s strengthens their forces. Narrate this briefly.",
             player_name, card_name, cost,
             theme->adjectives[0], theme->nouns[0]);

    return prompt;
}
// }}}

// {{{ event_narration_build_attack
char* event_narration_build_attack(Player* attacker, Player* defender,
                                    int damage, int remaining_auth) {
    if (attacker == NULL || defender == NULL) {
        return strdup_safe("An attack is launched.");
    }

    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    const char* attacker_name = attacker->name != NULL ? attacker->name : "The attacker";
    const char* defender_name = defender->name != NULL ? defender->name : "their opponent";

    prompt_vars_add(vars, "attacker", attacker_name);
    prompt_vars_add(vars, "defender", defender_name);

    char damage_str[16];
    snprintf(damage_str, sizeof(damage_str), "%d", damage);
    prompt_vars_add(vars, "damage", damage_str);

    char remaining_str[16];
    snprintf(remaining_str, sizeof(remaining_str), "%d", remaining_auth);
    prompt_vars_add(vars, "remaining_authority", remaining_str);

    char* prompt = prompt_build(PROMPT_ATTACK, vars);
    prompt_vars_free(vars);

    return prompt;
}
// }}}

// {{{ event_narration_build_base_attack
char* event_narration_build_base_attack(Player* attacker, Player* defender,
                                         CardInstance* base, int damage) {
    if (attacker == NULL || defender == NULL || base == NULL || base->type == NULL) {
        return strdup_safe("A base is attacked.");
    }

    const char* attacker_name = attacker->name != NULL ? attacker->name : "The attacker";
    const char* defender_name = defender->name != NULL ? defender->name : "their opponent";
    const char* base_name = base->type->name != NULL ? base->type->name : "a base";

    int remaining_defense = base->type->defense - base->damage_taken - damage;
    if (remaining_defense < 0) remaining_defense = 0;

    size_t buffer_size = 512;
    char* prompt = malloc(buffer_size);
    if (prompt == NULL) {
        return NULL;
    }

    snprintf(prompt, buffer_size,
             "%s's forces assault %s's %s, dealing %d damage. "
             "The %s has %d defense remaining. "
             "Narrate this attack vividly.",
             attacker_name, defender_name, base_name, damage,
             base_name, remaining_defense);

    return prompt;
}
// }}}

// {{{ event_narration_build_base_destroyed
char* event_narration_build_base_destroyed(Player* owner, CardInstance* base) {
    if (base == NULL || base->type == NULL) {
        return strdup_safe("A base falls.");
    }

    const char* owner_name = owner != NULL && owner->name != NULL
                             ? owner->name : "a commander";
    const char* base_name = base->type->name != NULL ? base->type->name : "the base";
    const FactionTheme* theme = force_desc_get_theme(base->type->faction);

    size_t buffer_size = 512;
    char* prompt = malloc(buffer_size);
    if (prompt == NULL) {
        return NULL;
    }

    snprintf(prompt, buffer_size,
             "The %s %s belonging to %s has been destroyed! "
             "Narrate its dramatic fall using %s imagery. "
             "Make it memorable in two sentences.",
             theme->adjectives[0], base_name, owner_name,
             theme->name);

    return prompt;
}
// }}}

// {{{ event_narration_build_turn_start
char* event_narration_build_turn_start(Player* player, int turn) {
    if (player == NULL) {
        return strdup_safe("A new turn begins.");
    }

    const char* player_name = player->name != NULL ? player->name : "A commander";

    size_t buffer_size = 256;
    char* prompt = malloc(buffer_size);
    if (prompt == NULL) {
        return NULL;
    }

    snprintf(prompt, buffer_size,
             "Turn %d begins. %s prepares their next move. "
             "One brief transitional sentence.",
             turn, player_name);

    return prompt;
}
// }}}

// {{{ event_narration_build_turn_end
char* event_narration_build_turn_end(Player* player, int trade_spent,
                                      int combat_dealt, int cards_played) {
    if (player == NULL) {
        return strdup_safe("The turn ends.");
    }

    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    const char* player_name = player->name != NULL ? player->name : "The commander";
    prompt_vars_add(vars, "player_name", player_name);

    char trade_str[16];
    snprintf(trade_str, sizeof(trade_str), "%d", trade_spent);
    prompt_vars_add(vars, "trade_made", trade_str);

    char combat_str[16];
    snprintf(combat_str, sizeof(combat_str), "%d", combat_dealt);
    prompt_vars_add(vars, "combat_dealt", combat_str);

    char cards_str[16];
    snprintf(cards_str, sizeof(cards_str), "%d", cards_played);
    prompt_vars_add(vars, "cards_played", cards_str);

    char* prompt = prompt_build(PROMPT_TURN_SUMMARY, vars);
    prompt_vars_free(vars);

    return prompt;
}
// }}}

// {{{ event_narration_build_game_over
char* event_narration_build_game_over(Player* winner, Player* loser,
                                       int final_authority) {
    if (winner == NULL || loser == NULL) {
        return strdup_safe("The battle concludes.");
    }

    const char* winner_name = winner->name != NULL ? winner->name : "The victor";
    const char* loser_name = loser->name != NULL ? loser->name : "their opponent";

    size_t buffer_size = 512;
    char* prompt = malloc(buffer_size);
    if (prompt == NULL) {
        return NULL;
    }

    snprintf(prompt, buffer_size,
             "VICTORY! %s triumphs over %s with %d authority remaining! "
             "The battle for Symbeline is decided. "
             "Narrate this epic conclusion in three dramatic sentences. "
             "Describe the victor's glory and the defeated's fall.",
             winner_name, loser_name, final_authority);

    return prompt;
}
// }}}

// {{{ event_narration_build
char* event_narration_build(NarrationEvent* event) {
    if (event == NULL) {
        return strdup_safe("Something happens.");
    }

    switch (event->type) {
        case GAME_EVENT_CARD_PLAYED:
            return event_narration_build_card_played(event->actor, event->card);

        case GAME_EVENT_CARD_PURCHASED:
            return event_narration_build_purchase(event->actor, event->card,
                                                   event->cost);

        case GAME_EVENT_ATTACK_PLAYER:
            return event_narration_build_attack(event->actor, event->target,
                                                 event->damage,
                                                 event->target != NULL
                                                     ? event->target->authority
                                                     : 0);

        case GAME_EVENT_ATTACK_BASE:
            return event_narration_build_base_attack(event->actor, event->target,
                                                      event->base, event->damage);

        case GAME_EVENT_BASE_DESTROYED:
            return event_narration_build_base_destroyed(event->target, event->base);

        case GAME_EVENT_TURN_START:
            return event_narration_build_turn_start(event->actor, event->turn);

        case GAME_EVENT_TURN_END:
            return event_narration_build_turn_end(event->actor, event->cost,
                                                   event->damage, event->turn);

        case GAME_EVENT_GAME_OVER:
            return event_narration_build_game_over(event->actor, event->target,
                                                    event->damage);

        default:
            return strdup_safe("An event occurs.");
    }
}
// }}}
