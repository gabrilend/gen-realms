/*
 * 03-world-state.c - World State Implementation
 *
 * Tracks game narrative context for LLM prompt generation.
 * Updates automatically based on game events to maintain
 * coherent narrative across the session.
 */

#include "03-world-state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// {{{ Faction Names
// Narrative-friendly faction names for LLM context.
static const char* FACTION_NAMES[FACTION_COUNT] = {
    "the Neutral forces",
    "the Merchant Guilds",
    "the forces of the Wilds",
    "the High Kingdom",
    "the Artificer Order"
};
// }}}

// {{{ Initial Descriptions
static const char* INITIAL_BATTLEFIELD =
    "The contested realm of Symbeline stretches before two rival commanders, "
    "each seeking dominion over these mystical lands.";

static const char* INITIAL_FORCES =
    "A small band of scouts and vipers, awaiting orders.";
// }}}

// {{{ strdup_safe
static char* strdup_safe(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}
// }}}

// {{{ game_event_create
static GameEvent* game_event_create(const char* event_type,
                                     const char* description,
                                     int player_id, int turn) {
    GameEvent* event = malloc(sizeof(GameEvent));
    if (event == NULL) {
        return NULL;
    }

    event->event_type = strdup_safe(event_type);
    event->description = strdup_safe(description);
    event->player_id = player_id;
    event->turn = turn;

    return event;
}
// }}}

// {{{ game_event_free
static void game_event_free(GameEvent* event) {
    if (event == NULL) {
        return;
    }

    free(event->event_type);
    free(event->description);
    free(event);
}
// }}}

// {{{ world_state_create
WorldState* world_state_create(void) {
    WorldState* state = malloc(sizeof(WorldState));
    if (state == NULL) {
        return NULL;
    }

    state->battlefield_description = strdup_safe(INITIAL_BATTLEFIELD);
    state->turn_number = 0;
    state->last_update_turn = 0;
    state->dominant_faction = FACTION_NEUTRAL;
    state->event_count = 0;
    state->event_cursor = 0;
    state->tension = 0.0f;

    // Initialize faction counts
    for (int i = 0; i < FACTION_COUNT; i++) {
        state->faction_card_counts[i] = 0;
    }

    // Initialize player forces
    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->player_forces[i] = NULL;
    }

    // Initialize event array
    for (int i = 0; i < WORLD_STATE_MAX_EVENTS; i++) {
        state->events[i] = NULL;
    }

    return state;
}
// }}}

// {{{ world_state_free
void world_state_free(WorldState* state) {
    if (state == NULL) {
        return;
    }

    free(state->battlefield_description);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        free(state->player_forces[i]);
    }

    for (int i = 0; i < WORLD_STATE_MAX_EVENTS; i++) {
        game_event_free(state->events[i]);
    }

    free(state);
}
// }}}

// {{{ world_state_init_from_game
void world_state_init_from_game(WorldState* state, Game* game) {
    if (state == NULL || game == NULL) {
        return;
    }

    state->turn_number = game->turn_number;
    state->last_update_turn = game->turn_number;

    // Initialize force descriptions for each player
    for (int i = 0; i < game->player_count && i < MAX_PLAYERS; i++) {
        free(state->player_forces[i]);
        state->player_forces[i] = strdup_safe(INITIAL_FORCES);
    }

    // Reset faction counts
    for (int i = 0; i < FACTION_COUNT; i++) {
        state->faction_card_counts[i] = 0;
    }

    state->tension = 0.0f;
}
// }}}

// {{{ world_state_calculate_tension
float world_state_calculate_tension(WorldState* state, Game* game) {
    if (state == NULL || game == NULL || game->player_count < 2) {
        return 0.0f;
    }

    // Factors affecting tension:
    // 1. Authority difference (closer = more tense)
    // 2. Low authority (either player close to losing)
    // 3. Turn count (longer games = more dramatic)

    Player* p1 = game->players[0];
    Player* p2 = game->players[1];

    if (p1 == NULL || p2 == NULL) {
        return 0.0f;
    }

    int auth1 = p1->authority;
    int auth2 = p2->authority;
    int starting = PLAYER_STARTING_AUTHORITY;

    // Authority closeness (0-0.4)
    int diff = abs(auth1 - auth2);
    float closeness = 0.4f * (1.0f - (float)diff / starting);

    // Low authority (0-0.4)
    int min_auth = (auth1 < auth2) ? auth1 : auth2;
    float danger = 0.4f * (1.0f - (float)min_auth / starting);

    // Game length (0-0.2)
    float length_factor = 0.2f * fminf((float)game->turn_number / 20.0f, 1.0f);

    float tension = closeness + danger + length_factor;
    if (tension > 1.0f) tension = 1.0f;
    if (tension < 0.0f) tension = 0.0f;

    state->tension = tension;
    return tension;
}
// }}}

// {{{ world_state_update
void world_state_update(WorldState* state, Game* game) {
    if (state == NULL || game == NULL) {
        return;
    }

    state->turn_number = game->turn_number;

    // Update tension
    world_state_calculate_tension(state, game);

    // Update dominant faction based on cards played
    int max_count = 0;
    Faction dominant = FACTION_NEUTRAL;
    for (int i = 1; i < FACTION_COUNT; i++) {  // Skip neutral
        if (state->faction_card_counts[i] > max_count) {
            max_count = state->faction_card_counts[i];
            dominant = (Faction)i;
        }
    }
    state->dominant_faction = dominant;

    // Update battlefield description based on tension
    free(state->battlefield_description);
    if (state->tension > 0.8f) {
        state->battlefield_description = strdup_safe(
            "The battlefield is scarred from prolonged conflict. "
            "The air crackles with tension as both sides prepare for "
            "what may be the decisive clash.");
    } else if (state->tension > 0.5f) {
        state->battlefield_description = strdup_safe(
            "Signs of battle mark the contested ground. "
            "Neither commander has yet secured the upper hand, "
            "but the struggle intensifies with each turn.");
    } else if (state->tension > 0.2f) {
        state->battlefield_description = strdup_safe(
            "The battle lines are drawn as both forces marshal their strength. "
            "Skirmishes break out as commanders test each other's defenses.");
    } else {
        state->battlefield_description = strdup_safe(
            "The contested realm of Symbeline stretches before two rival commanders, "
            "each seeking dominion over these mystical lands.");
    }

    state->last_update_turn = game->turn_number;
}
// }}}

// {{{ world_state_record_event
void world_state_record_event(WorldState* state, const char* event_type,
                               const char* description, int player_id, int turn) {
    if (state == NULL || event_type == NULL || description == NULL) {
        return;
    }

    // Free old event at cursor position
    game_event_free(state->events[state->event_cursor]);

    // Create new event
    state->events[state->event_cursor] = game_event_create(
        event_type, description, player_id, turn);

    // Advance cursor (circular buffer)
    state->event_cursor = (state->event_cursor + 1) % WORLD_STATE_MAX_EVENTS;

    if (state->event_count < WORLD_STATE_MAX_EVENTS) {
        state->event_count++;
    }
}
// }}}

// {{{ world_state_get_recent_events
char* world_state_get_recent_events(WorldState* state, int max_events) {
    if (state == NULL || state->event_count == 0) {
        return strdup_safe("No significant events yet.");
    }

    if (max_events > state->event_count) {
        max_events = state->event_count;
    }

    // Calculate buffer size
    size_t total_size = 256;  // Base size for formatting
    for (int i = 0; i < max_events; i++) {
        int idx = (state->event_cursor - 1 - i + WORLD_STATE_MAX_EVENTS) %
                  WORLD_STATE_MAX_EVENTS;
        if (state->events[idx] != NULL && state->events[idx]->description != NULL) {
            total_size += strlen(state->events[idx]->description) + 50;
        }
    }

    char* result = malloc(total_size);
    if (result == NULL) {
        return NULL;
    }
    result[0] = '\0';

    strcat(result, "Recent events:\n");

    for (int i = 0; i < max_events; i++) {
        int idx = (state->event_cursor - 1 - i + WORLD_STATE_MAX_EVENTS) %
                  WORLD_STATE_MAX_EVENTS;
        GameEvent* event = state->events[idx];

        if (event != NULL && event->description != NULL) {
            char line[256];
            snprintf(line, sizeof(line), "- Turn %d: %s\n",
                     event->turn, event->description);
            strcat(result, line);
        }
    }

    return result;
}
// }}}

// {{{ world_state_to_prompt_vars
void world_state_to_prompt_vars(WorldState* state, Game* game, PromptVars* vars) {
    if (state == NULL || game == NULL || vars == NULL) {
        return;
    }

    char buffer[64];

    // Turn number
    snprintf(buffer, sizeof(buffer), "%d", game->turn_number);
    prompt_vars_add(vars, "turn", buffer);

    // Player 1
    if (game->player_count >= 1 && game->players[0] != NULL) {
        Player* p1 = game->players[0];
        prompt_vars_add(vars, "player1_name",
                        p1->name != NULL ? p1->name : "Player 1");
        snprintf(buffer, sizeof(buffer), "%d", p1->authority);
        prompt_vars_add(vars, "player1_authority", buffer);
    }

    // Player 2
    if (game->player_count >= 2 && game->players[1] != NULL) {
        Player* p2 = game->players[1];
        prompt_vars_add(vars, "player2_name",
                        p2->name != NULL ? p2->name : "Player 2");
        snprintf(buffer, sizeof(buffer), "%d", p2->authority);
        prompt_vars_add(vars, "player2_authority", buffer);
    }

    // Phase
    prompt_vars_add(vars, "phase", game_phase_to_string(game->phase));
}
// }}}

// {{{ world_state_build_context
char* world_state_build_context(WorldState* state, Game* game) {
    if (state == NULL || game == NULL) {
        return NULL;
    }

    // Build prompt vars
    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    world_state_to_prompt_vars(state, game, vars);

    // Build world state prompt
    char* world_prompt = prompt_build(PROMPT_WORLD_STATE, vars);
    prompt_vars_free(vars);

    if (world_prompt == NULL) {
        return NULL;
    }

    // Get recent events
    char* events = world_state_get_recent_events(state, 5);

    // Calculate total size
    size_t total = strlen(world_prompt) + strlen(state->battlefield_description) +
                   (events != NULL ? strlen(events) : 0) + 256;

    char* context = malloc(total);
    if (context == NULL) {
        free(world_prompt);
        free(events);
        return NULL;
    }

    // Build context string
    snprintf(context, total,
             "%s\n\n"
             "Battlefield: %s\n\n"
             "%s",
             world_prompt,
             state->battlefield_description,
             events != NULL ? events : "");

    free(world_prompt);
    free(events);

    return context;
}
// }}}

// {{{ world_state_get_faction_name
const char* world_state_get_faction_name(Faction faction) {
    if (faction < 0 || faction >= FACTION_COUNT) {
        return "unknown forces";
    }
    return FACTION_NAMES[faction];
}
// }}}
