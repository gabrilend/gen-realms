/*
 * 04-force-description.h - Force Description Prompts
 *
 * Generates thematic descriptions for player forces, cards, and bases.
 * Uses faction-specific themes for consistent narrative style.
 * Includes caching to avoid regenerating unchanged descriptions.
 */

#ifndef LLM_FORCE_DESCRIPTION_H
#define LLM_FORCE_DESCRIPTION_H

#include "../core/05-game.h"
#include "02-prompts.h"
#include <stdbool.h>

/* Maximum cached descriptions per player */
#define FORCE_DESC_CACHE_SIZE 16

// {{{ FactionTheme
// Theme keywords and descriptors for each faction.
typedef struct {
    Faction faction;
    const char* name;           // Faction display name
    const char* adjectives[4];  // Thematic adjectives
    const char* nouns[4];       // Thematic nouns/imagery
    const char* verbs[4];       // Thematic action words
} FactionTheme;
// }}}

// {{{ CachedDescription
// Cached description for a card or base.
typedef struct {
    char* card_id;              // Card instance ID
    char* description;          // Generated description
    int turn_generated;         // Turn when generated
} CachedDescription;
// }}}

// {{{ ForceDescCache
// Cache for force descriptions to avoid regeneration.
typedef struct {
    CachedDescription* entries[FORCE_DESC_CACHE_SIZE];
    int count;
    int cursor;                 // For circular replacement
} ForceDescCache;
// }}}

// {{{ force_desc_cache_create
// Creates a new description cache.
ForceDescCache* force_desc_cache_create(void);
// }}}

// {{{ force_desc_cache_free
// Frees the cache and all cached descriptions.
void force_desc_cache_free(ForceDescCache* cache);
// }}}

// {{{ force_desc_cache_get
// Retrieves a cached description by card ID.
// Returns NULL if not cached.
const char* force_desc_cache_get(ForceDescCache* cache, const char* card_id);
// }}}

// {{{ force_desc_cache_set
// Caches a description for a card ID.
void force_desc_cache_set(ForceDescCache* cache, const char* card_id,
                           const char* description, int turn);
// }}}

// {{{ force_desc_get_theme
// Gets the theme for a faction.
const FactionTheme* force_desc_get_theme(Faction faction);
// }}}

// {{{ force_desc_build_player_forces
// Builds a description prompt for a player's current forces.
// Caller must free returned string.
char* force_desc_build_player_forces(Player* player, Game* game);
// }}}

// {{{ force_desc_build_card_played
// Builds a description prompt for a card being played.
// Caller must free returned string.
char* force_desc_build_card_played(CardInstance* card, Player* player);
// }}}

// {{{ force_desc_build_base
// Builds a description prompt for a base in play.
// Caller must free returned string.
char* force_desc_build_base(CardInstance* base, BasePlacement placement);
// }}}

// {{{ force_desc_build_attack
// Builds a description prompt for an attack.
// Caller must free returned string.
char* force_desc_build_attack(Player* attacker, Player* defender, int damage);
// }}}

// {{{ force_desc_get_faction_adjective
// Returns a random thematic adjective for a faction.
const char* force_desc_get_faction_adjective(Faction faction);
// }}}

// {{{ force_desc_get_faction_noun
// Returns a random thematic noun for a faction.
const char* force_desc_get_faction_noun(Faction faction);
// }}}

// {{{ force_desc_summarize_forces
// Creates a text summary of cards in play for a player.
// Caller must free returned string.
char* force_desc_summarize_forces(Player* player);
// }}}

// {{{ force_desc_get_dominant_faction
// Determines the dominant faction for a player based on cards played.
Faction force_desc_get_dominant_faction(Player* player);
// }}}

#endif /* LLM_FORCE_DESCRIPTION_H */
