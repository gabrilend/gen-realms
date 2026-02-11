/*
 * 05-event-narration.h - Event Narration Prompts
 *
 * Generates narrative descriptions for game events including card plays,
 * purchases, attacks, base destruction, and game endings.
 * Scales narrative intensity based on game tension and damage values.
 */

#ifndef LLM_EVENT_NARRATION_H
#define LLM_EVENT_NARRATION_H

#include "../core/05-game.h"
#include "02-prompts.h"
#include "03-world-state.h"
#include <stdbool.h>

// {{{ GameEventType
// Types of game events that can be narrated.
typedef enum {
    GAME_EVENT_CARD_PLAYED,      // A card was played from hand
    GAME_EVENT_CARD_PURCHASED,   // A card was bought from trade row
    GAME_EVENT_ATTACK_PLAYER,    // Direct attack on opponent
    GAME_EVENT_ATTACK_BASE,      // Attack on opponent's base
    GAME_EVENT_BASE_DESTROYED,   // A base was destroyed
    GAME_EVENT_TURN_START,       // A new turn begins
    GAME_EVENT_TURN_END,         // Turn has ended
    GAME_EVENT_GAME_OVER,        // Game has ended
    GAME_EVENT_ALLY_TRIGGERED,   // Ally ability activated
    GAME_EVENT_SCRAP,            // Card was scrapped
    GAME_EVENT_TYPE_COUNT        // Sentinel
} GameEventType;
// }}}

// {{{ NarrationIntensity
// How dramatic the narration should be.
typedef enum {
    INTENSITY_LOW,       // Brief, understated
    INTENSITY_MEDIUM,    // Standard dramatic
    INTENSITY_HIGH,      // Very dramatic, climactic
    INTENSITY_EPIC       // Maximum drama for game-ending moments
} NarrationIntensity;
// }}}

// {{{ NarrationEvent
// Represents a game event to be narrated.
typedef struct {
    GameEventType type;

    // Context
    Player* actor;           // Player causing the event
    Player* target;          // Target player (if applicable)
    CardInstance* card;      // Card involved (if applicable)
    CardInstance* base;      // Base involved (if applicable)

    // Values
    int damage;              // Damage dealt (for attacks)
    int cost;                // Cost paid (for purchases)
    int turn;                // Turn number

    // State
    NarrationIntensity intensity;
} NarrationEvent;
// }}}

// {{{ event_narration_create
// Creates a new narration event.
NarrationEvent* event_narration_create(GameEventType type);
// }}}

// {{{ event_narration_free
// Frees a narration event.
void event_narration_free(NarrationEvent* event);
// }}}

// {{{ event_narration_build
// Builds a narrative prompt for an event.
// Caller must free returned string.
char* event_narration_build(NarrationEvent* event);
// }}}

// {{{ event_narration_build_card_played
// Builds narration for a card being played.
// Caller must free returned string.
char* event_narration_build_card_played(Player* player, CardInstance* card);
// }}}

// {{{ event_narration_build_purchase
// Builds narration for a card purchase.
// Caller must free returned string.
char* event_narration_build_purchase(Player* player, CardInstance* card, int cost);
// }}}

// {{{ event_narration_build_attack
// Builds narration for an attack on a player.
// Caller must free returned string.
char* event_narration_build_attack(Player* attacker, Player* defender,
                                    int damage, int remaining_auth);
// }}}

// {{{ event_narration_build_base_attack
// Builds narration for an attack on a base.
// Caller must free returned string.
char* event_narration_build_base_attack(Player* attacker, Player* defender,
                                         CardInstance* base, int damage);
// }}}

// {{{ event_narration_build_base_destroyed
// Builds narration for a base destruction.
// Caller must free returned string.
char* event_narration_build_base_destroyed(Player* owner, CardInstance* base);
// }}}

// {{{ event_narration_build_turn_start
// Builds a brief turn transition narration.
// Caller must free returned string.
char* event_narration_build_turn_start(Player* player, int turn);
// }}}

// {{{ event_narration_build_turn_end
// Builds a turn end summary.
// Caller must free returned string.
char* event_narration_build_turn_end(Player* player, int trade_spent,
                                      int combat_dealt, int cards_played);
// }}}

// {{{ event_narration_build_game_over
// Builds epic game over narration.
// Caller must free returned string.
char* event_narration_build_game_over(Player* winner, Player* loser,
                                       int final_authority);
// }}}

// {{{ event_narration_calculate_intensity
// Calculates appropriate intensity based on game state.
NarrationIntensity event_narration_calculate_intensity(WorldState* state,
                                                        NarrationEvent* event);
// }}}

// {{{ event_narration_get_intensity_word
// Returns a word describing the intensity level.
const char* event_narration_get_intensity_word(NarrationIntensity intensity);
// }}}

// {{{ event_type_to_string
// Returns string name of event type.
const char* event_type_to_string(GameEventType type);
// }}}

#endif /* LLM_EVENT_NARRATION_H */
