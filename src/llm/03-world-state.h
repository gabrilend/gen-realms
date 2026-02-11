/*
 * 03-world-state.h - World State for LLM Context
 *
 * Maintains persistent narrative context for LLM prompts.
 * Tracks game progress, faction dominance, battlefield conditions,
 * and recent events to provide coherent narrative generation.
 */

#ifndef LLM_WORLD_STATE_H
#define LLM_WORLD_STATE_H

#include "../core/05-game.h"
#include "02-prompts.h"
#include <stdbool.h>

/* Maximum number of recent events to track */
#define WORLD_STATE_MAX_EVENTS 10

/* Maximum description length */
#define WORLD_STATE_MAX_DESC 512

// {{{ GameEvent
// Represents a significant game event for narrative tracking.
typedef struct {
    char* description;      // Brief event description
    int turn;               // Turn when event occurred
    int player_id;          // Player who triggered event (-1 if neutral)
    char* event_type;       // Type: "attack", "buy", "play", "base_destroyed"
} GameEvent;
// }}}

// {{{ WorldState
// Persistent narrative state tracking for LLM context.
typedef struct {
    // Battlefield narrative
    char* battlefield_description;      // Current battlefield state
    char* player_forces[MAX_PLAYERS];   // Description of each player's forces

    // Turn tracking
    int turn_number;
    int last_update_turn;               // Last turn state was updated

    // Faction balance (tracks which factions are dominant)
    int faction_card_counts[FACTION_COUNT];  // Cards played per faction
    Faction dominant_faction;                // Current dominant faction

    // Event history
    GameEvent* events[WORLD_STATE_MAX_EVENTS];
    int event_count;
    int event_cursor;                   // Circular buffer cursor

    // Game tension (0.0 = calm, 1.0 = climactic)
    float tension;
} WorldState;
// }}}

// {{{ world_state_create
// Creates a new WorldState with default initial descriptions.
WorldState* world_state_create(void);
// }}}

// {{{ world_state_free
// Frees all memory associated with state.
void world_state_free(WorldState* state);
// }}}

// {{{ world_state_init_from_game
// Initializes world state from a Game struct.
// Should be called when game starts.
void world_state_init_from_game(WorldState* state, Game* game);
// }}}

// {{{ world_state_update
// Updates world state based on current game state.
// Called after each significant game action.
void world_state_update(WorldState* state, Game* game);
// }}}

// {{{ world_state_record_event
// Records a significant game event for narrative context.
void world_state_record_event(WorldState* state, const char* event_type,
                               const char* description, int player_id, int turn);
// }}}

// {{{ world_state_get_recent_events
// Returns a string summarizing recent events.
// Caller must free returned string.
char* world_state_get_recent_events(WorldState* state, int max_events);
// }}}

// {{{ world_state_to_prompt_vars
// Populates a PromptVars with world state data.
// For use with prompt_build(PROMPT_WORLD_STATE, vars).
void world_state_to_prompt_vars(WorldState* state, Game* game, PromptVars* vars);
// }}}

// {{{ world_state_build_context
// Builds complete LLM context from world state.
// Caller must free returned string.
char* world_state_build_context(WorldState* state, Game* game);
// }}}

// {{{ world_state_calculate_tension
// Calculates current game tension level (0.0 - 1.0).
// Higher tension = more dramatic narrative encouraged.
float world_state_calculate_tension(WorldState* state, Game* game);
// }}}

// {{{ world_state_get_faction_name
// Returns the narrative name for a faction.
const char* world_state_get_faction_name(Faction faction);
// }}}

#endif /* LLM_WORLD_STATE_H */
