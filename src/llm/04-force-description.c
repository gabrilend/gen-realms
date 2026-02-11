/*
 * 04-force-description.c - Force Description Implementation
 *
 * Generates narrative descriptions for player forces using faction themes.
 * Caches descriptions to avoid regenerating unchanged content.
 */

#include "04-force-description.h"
#include "02-prompts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// {{{ Faction Themes
static const FactionTheme FACTION_THEMES[FACTION_COUNT] = {
    // FACTION_NEUTRAL
    {
        .faction = FACTION_NEUTRAL,
        .name = "Neutral",
        .adjectives = {"versatile", "adaptable", "reliable", "steady"},
        .nouns = {"scouts", "explorers", "mercenaries", "wanderers"},
        .verbs = {"survey", "scout", "roam", "traverse"}
    },
    // FACTION_MERCHANT
    {
        .faction = FACTION_MERCHANT,
        .name = "Merchant Guilds",
        .adjectives = {"gilded", "prosperous", "cunning", "wealthy"},
        .nouns = {"caravans", "trade ships", "gold", "coffers"},
        .verbs = {"bargain", "acquire", "invest", "profit"}
    },
    // FACTION_WILDS
    {
        .faction = FACTION_WILDS,
        .name = "The Wilds",
        .adjectives = {"primal", "savage", "untamed", "ferocious"},
        .nouns = {"beasts", "dire wolves", "thornwood", "claws"},
        .verbs = {"hunt", "stalk", "devour", "rampage"}
    },
    // FACTION_KINGDOM
    {
        .faction = FACTION_KINGDOM,
        .name = "High Kingdom",
        .adjectives = {"noble", "valiant", "gleaming", "honorable"},
        .nouns = {"knights", "banners", "castles", "armor"},
        .verbs = {"defend", "charge", "rally", "proclaim"}
    },
    // FACTION_ARTIFICER
    {
        .faction = FACTION_ARTIFICER,
        .name = "Artificer Order",
        .adjectives = {"arcane", "mechanical", "intricate", "pulsing"},
        .nouns = {"constructs", "workshops", "gears", "crystals"},
        .verbs = {"forge", "assemble", "energize", "transmute"}
    }
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

// {{{ random_index
// Returns a random index in range [0, max).
static int random_index(int max) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    return rand() % max;
}
// }}}

// {{{ force_desc_cache_create
ForceDescCache* force_desc_cache_create(void) {
    ForceDescCache* cache = malloc(sizeof(ForceDescCache));
    if (cache == NULL) {
        return NULL;
    }

    cache->count = 0;
    cache->cursor = 0;

    for (int i = 0; i < FORCE_DESC_CACHE_SIZE; i++) {
        cache->entries[i] = NULL;
    }

    return cache;
}
// }}}

// {{{ force_desc_cache_free
void force_desc_cache_free(ForceDescCache* cache) {
    if (cache == NULL) {
        return;
    }

    for (int i = 0; i < FORCE_DESC_CACHE_SIZE; i++) {
        if (cache->entries[i] != NULL) {
            free(cache->entries[i]->card_id);
            free(cache->entries[i]->description);
            free(cache->entries[i]);
        }
    }

    free(cache);
}
// }}}

// {{{ force_desc_cache_get
const char* force_desc_cache_get(ForceDescCache* cache, const char* card_id) {
    if (cache == NULL || card_id == NULL) {
        return NULL;
    }

    for (int i = 0; i < FORCE_DESC_CACHE_SIZE; i++) {
        if (cache->entries[i] != NULL &&
            cache->entries[i]->card_id != NULL &&
            strcmp(cache->entries[i]->card_id, card_id) == 0) {
            return cache->entries[i]->description;
        }
    }

    return NULL;
}
// }}}

// {{{ force_desc_cache_set
void force_desc_cache_set(ForceDescCache* cache, const char* card_id,
                           const char* description, int turn) {
    if (cache == NULL || card_id == NULL || description == NULL) {
        return;
    }

    // Check if already cached
    for (int i = 0; i < FORCE_DESC_CACHE_SIZE; i++) {
        if (cache->entries[i] != NULL &&
            cache->entries[i]->card_id != NULL &&
            strcmp(cache->entries[i]->card_id, card_id) == 0) {
            // Update existing entry
            free(cache->entries[i]->description);
            cache->entries[i]->description = strdup_safe(description);
            cache->entries[i]->turn_generated = turn;
            return;
        }
    }

    // Free existing entry at cursor position
    if (cache->entries[cache->cursor] != NULL) {
        free(cache->entries[cache->cursor]->card_id);
        free(cache->entries[cache->cursor]->description);
        free(cache->entries[cache->cursor]);
    }

    // Create new entry
    CachedDescription* entry = malloc(sizeof(CachedDescription));
    entry->card_id = strdup_safe(card_id);
    entry->description = strdup_safe(description);
    entry->turn_generated = turn;

    cache->entries[cache->cursor] = entry;
    cache->cursor = (cache->cursor + 1) % FORCE_DESC_CACHE_SIZE;

    if (cache->count < FORCE_DESC_CACHE_SIZE) {
        cache->count++;
    }
}
// }}}

// {{{ force_desc_get_theme
const FactionTheme* force_desc_get_theme(Faction faction) {
    if (faction < 0 || faction >= FACTION_COUNT) {
        return &FACTION_THEMES[FACTION_NEUTRAL];
    }
    return &FACTION_THEMES[faction];
}
// }}}

// {{{ force_desc_get_faction_adjective
const char* force_desc_get_faction_adjective(Faction faction) {
    const FactionTheme* theme = force_desc_get_theme(faction);
    return theme->adjectives[random_index(4)];
}
// }}}

// {{{ force_desc_get_faction_noun
const char* force_desc_get_faction_noun(Faction faction) {
    const FactionTheme* theme = force_desc_get_theme(faction);
    return theme->nouns[random_index(4)];
}
// }}}

// {{{ force_desc_get_dominant_faction
Faction force_desc_get_dominant_faction(Player* player) {
    if (player == NULL) {
        return FACTION_NEUTRAL;
    }

    // Count factions played this turn
    int max_count = 0;
    Faction dominant = FACTION_NEUTRAL;

    for (int i = 1; i < FACTION_COUNT; i++) {
        if (player->factions_played[i]) {
            // Simple: just pick the first faction played
            // In a more complex implementation, we'd track card counts
            if (dominant == FACTION_NEUTRAL) {
                dominant = (Faction)i;
            }
        }
    }

    return dominant;
}
// }}}

// {{{ force_desc_summarize_forces
char* force_desc_summarize_forces(Player* player) {
    if (player == NULL) {
        return strdup_safe("an unknown force");
    }

    // Build a summary of forces based on factions played
    size_t buffer_size = 256;
    char* summary = malloc(buffer_size);
    if (summary == NULL) {
        return NULL;
    }

    summary[0] = '\0';
    int faction_count = 0;

    for (int i = 1; i < FACTION_COUNT; i++) {
        if (player->factions_played[i]) {
            const FactionTheme* theme = force_desc_get_theme((Faction)i);
            if (faction_count > 0) {
                strcat(summary, " and ");
            }
            strcat(summary, theme->nouns[0]);
            strcat(summary, " of ");
            strcat(summary, theme->name);
            faction_count++;
        }
    }

    if (faction_count == 0) {
        strcpy(summary, "scouts and vipers");
    }

    return summary;
}
// }}}

// {{{ force_desc_build_player_forces
char* force_desc_build_player_forces(Player* player, Game* game) {
    if (player == NULL) {
        return NULL;
    }

    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    // Player name
    prompt_vars_add(vars, "player_name",
                    player->name != NULL ? player->name : "Unknown Commander");

    // Faction
    Faction dominant = force_desc_get_dominant_faction(player);
    const FactionTheme* theme = force_desc_get_theme(dominant);
    prompt_vars_add(vars, "faction", theme->name);

    // Bases in play (simplified - count from deck zones)
    char bases_str[16];
    snprintf(bases_str, sizeof(bases_str), "%d", 0);  // Would need deck access
    prompt_vars_add(vars, "bases_in_play", bases_str);

    // Cards in hand
    char hand_str[16];
    snprintf(hand_str, sizeof(hand_str), "%d",
             player->deck != NULL ? player->deck->hand_count : 0);
    prompt_vars_add(vars, "cards_in_hand", hand_str);

    char* prompt = prompt_build(PROMPT_FORCE_DESCRIPTION, vars);
    prompt_vars_free(vars);

    return prompt;
}
// }}}

// {{{ force_desc_build_card_played
char* force_desc_build_card_played(CardInstance* card, Player* player) {
    if (card == NULL || card->type == NULL) {
        return NULL;
    }

    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    // Player name
    const char* player_name = (player != NULL && player->name != NULL)
                              ? player->name : "A commander";
    prompt_vars_add(vars, "player_name", player_name);

    // Card name
    prompt_vars_add(vars, "card_name",
                    card->type->name != NULL ? card->type->name : "a mysterious card");

    // Card faction
    const FactionTheme* theme = force_desc_get_theme(card->type->faction);
    prompt_vars_add(vars, "card_faction", theme->name);

    // Card effect (simplified description)
    char effect_str[128];
    snprintf(effect_str, sizeof(effect_str),
             "costs %d trade to acquire",
             card->type->cost);
    prompt_vars_add(vars, "card_effect", effect_str);

    char* prompt = prompt_build(PROMPT_CARD_PLAYED, vars);
    prompt_vars_free(vars);

    return prompt;
}
// }}}

// {{{ force_desc_build_base
char* force_desc_build_base(CardInstance* base, BasePlacement placement) {
    if (base == NULL || base->type == NULL) {
        return NULL;
    }

    // Build a custom prompt for bases
    const char* placement_str = (placement == ZONE_FRONTIER)
                                ? "at the frontier"
                                : "in the interior";
    const FactionTheme* theme = force_desc_get_theme(base->type->faction);

    size_t buffer_size = 512;
    char* prompt = malloc(buffer_size);
    if (prompt == NULL) {
        return NULL;
    }

    snprintf(prompt, buffer_size,
             "Describe the %s base named %s, positioned %s. "
             "It is a %s %s stronghold with %d defense. "
             "Use %s imagery in one dramatic sentence.",
             theme->adjectives[random_index(4)],
             base->type->name != NULL ? base->type->name : "unnamed outpost",
             placement_str,
             theme->adjectives[random_index(4)],
             theme->name,
             base->type->defense,
             theme->nouns[random_index(4)]);

    return prompt;
}
// }}}

// {{{ force_desc_build_attack
char* force_desc_build_attack(Player* attacker, Player* defender, int damage) {
    if (attacker == NULL || defender == NULL) {
        return NULL;
    }

    PromptVars* vars = prompt_vars_create();
    if (vars == NULL) {
        return NULL;
    }

    // Attacker name
    prompt_vars_add(vars, "attacker",
                    attacker->name != NULL ? attacker->name : "The attacker");

    // Defender name
    prompt_vars_add(vars, "defender",
                    defender->name != NULL ? defender->name : "their opponent");

    // Damage
    char damage_str[16];
    snprintf(damage_str, sizeof(damage_str), "%d", damage);
    prompt_vars_add(vars, "damage", damage_str);

    // Remaining authority
    char remaining_str[16];
    snprintf(remaining_str, sizeof(remaining_str), "%d", defender->authority);
    prompt_vars_add(vars, "remaining_authority", remaining_str);

    char* prompt = prompt_build(PROMPT_ATTACK, vars);
    prompt_vars_free(vars);

    return prompt;
}
// }}}
