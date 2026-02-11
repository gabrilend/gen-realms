/*
 * 02-prompts.c - Prompt Network Structure Implementation
 *
 * Defines narrative templates with variable interpolation.
 * Templates are designed for fantasy battle narration with
 * consistent tone matching the Symbeline Realms theme.
 */

#include "02-prompts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// {{{ System Prompt
// Sets the narrative tone for all LLM responses.
static const char* SYSTEM_PROMPT =
    "You are the narrator of Symbeline Realms, a fantasy deck-building battle game. "
    "Your role is to describe the clash between two rival forces vying for control "
    "of the mystical realm of Symbeline. "
    "\n\n"
    "Guidelines:\n"
    "- Use vivid, evocative language befitting high fantasy\n"
    "- Keep descriptions concise (2-3 sentences unless asked for more)\n"
    "- Reference the mechanical effects of cards while maintaining narrative immersion\n"
    "- Maintain consistent characterization for each faction\n"
    "- Build tension as authority levels drop\n"
    "- Celebrate dramatic moments like powerful card combos\n";
// }}}

// {{{ Template Definitions
// Required variable names for each template type.
static const char* WORLD_STATE_VARS[] = {
    "turn", "player1_name", "player1_authority",
    "player2_name", "player2_authority", "phase"
};

static const char* FORCE_DESCRIPTION_VARS[] = {
    "player_name", "faction", "bases_in_play", "cards_in_hand"
};

static const char* EVENT_NARRATION_VARS[] = {
    "event_type", "actor", "target", "effect"
};

static const char* TRADE_ROW_VARS[] = {
    "available_factions", "game_tension", "player_factions"
};

static const char* TURN_SUMMARY_VARS[] = {
    "player_name", "trade_made", "combat_dealt", "cards_played"
};

static const char* CARD_PLAYED_VARS[] = {
    "player_name", "card_name", "card_faction", "card_effect"
};

static const char* ATTACK_VARS[] = {
    "attacker", "defender", "damage", "remaining_authority"
};
// }}}

// Template text macros for static initialization
#define WORLD_STATE_TEMPLATE \
    "Turn {turn} of the battle for Symbeline. " \
    "{player1_name} commands {player1_authority} authority. " \
    "{player2_name} commands {player2_authority} authority. " \
    "The current phase is {phase}."

#define FORCE_DESCRIPTION_TEMPLATE \
    "Describe the forces of {player_name}, aligned with the {faction}. " \
    "They have {bases_in_play} bases in play and {cards_in_hand} cards in hand."

#define EVENT_NARRATION_TEMPLATE \
    "Narrate this event: {event_type}. " \
    "Actor: {actor}. Target: {target}. Effect: {effect}. " \
    "Keep it brief and dramatic."

#define TRADE_ROW_TEMPLATE \
    "Select 5 cards for the trade row from available factions: {available_factions}. " \
    "Current game tension level: {game_tension}. " \
    "Player faction preferences: {player_factions}. " \
    "Choose cards that create interesting strategic choices."

#define TURN_SUMMARY_TEMPLATE \
    "Summarize {player_name}'s turn. " \
    "Trade value spent: {trade_made}. " \
    "Combat dealt: {combat_dealt}. " \
    "Cards played: {cards_played}."

#define CARD_PLAYED_TEMPLATE \
    "{player_name} plays {card_name} from the {card_faction}. " \
    "Effect: {card_effect}. Describe this moment."

#define ATTACK_TEMPLATE \
    "{attacker} strikes at {defender} for {damage} damage! " \
    "{defender} now has {remaining_authority} authority remaining."

// {{{ Template Array
static const PromptTemplate TEMPLATES[PROMPT_COUNT] = {
    {
        .type = PROMPT_WORLD_STATE,
        .name = "World State",
        .template_text = WORLD_STATE_TEMPLATE,
        .required_vars = WORLD_STATE_VARS,
        .var_count = 6
    },
    {
        .type = PROMPT_FORCE_DESCRIPTION,
        .name = "Force Description",
        .template_text = FORCE_DESCRIPTION_TEMPLATE,
        .required_vars = FORCE_DESCRIPTION_VARS,
        .var_count = 4
    },
    {
        .type = PROMPT_EVENT_NARRATION,
        .name = "Event Narration",
        .template_text = EVENT_NARRATION_TEMPLATE,
        .required_vars = EVENT_NARRATION_VARS,
        .var_count = 4
    },
    {
        .type = PROMPT_TRADE_ROW_SELECTION,
        .name = "Trade Row Selection",
        .template_text = TRADE_ROW_TEMPLATE,
        .required_vars = TRADE_ROW_VARS,
        .var_count = 3
    },
    {
        .type = PROMPT_TURN_SUMMARY,
        .name = "Turn Summary",
        .template_text = TURN_SUMMARY_TEMPLATE,
        .required_vars = TURN_SUMMARY_VARS,
        .var_count = 4
    },
    {
        .type = PROMPT_CARD_PLAYED,
        .name = "Card Played",
        .template_text = CARD_PLAYED_TEMPLATE,
        .required_vars = CARD_PLAYED_VARS,
        .var_count = 4
    },
    {
        .type = PROMPT_ATTACK,
        .name = "Attack",
        .template_text = ATTACK_TEMPLATE,
        .required_vars = ATTACK_VARS,
        .var_count = 4
    }
};
// }}}

// {{{ Standard Chains
// Chain for describing a complete turn.
static const PromptType TURN_CHAIN[] = {
    PROMPT_WORLD_STATE,
    PROMPT_FORCE_DESCRIPTION,
    PROMPT_TURN_SUMMARY,
    PROMPT_COUNT // Sentinel
};

// Chain for narrating a card play.
static const PromptType CARD_PLAY_CHAIN[] = {
    PROMPT_CARD_PLAYED,
    PROMPT_EVENT_NARRATION,
    PROMPT_COUNT // Sentinel
};

// Chain for combat resolution.
static const PromptType COMBAT_CHAIN[] = {
    PROMPT_ATTACK,
    PROMPT_EVENT_NARRATION,
    PROMPT_COUNT // Sentinel
};
// }}}

// {{{ prompt_vars_create
PromptVars* prompt_vars_create(void) {
    PromptVars* vars = malloc(sizeof(PromptVars));
    if (vars == NULL) {
        return NULL;
    }

    vars->capacity = 8;
    vars->count = 0;
    vars->vars = malloc(sizeof(PromptVar) * vars->capacity);
    if (vars->vars == NULL) {
        free(vars);
        return NULL;
    }

    return vars;
}
// }}}

// {{{ prompt_vars_add
bool prompt_vars_add(PromptVars* vars, const char* name, const char* value) {
    if (vars == NULL || name == NULL || value == NULL) {
        return false;
    }

    // Grow array if needed
    if (vars->count >= vars->capacity) {
        int new_capacity = vars->capacity * 2;
        PromptVar* new_vars = realloc(vars->vars, sizeof(PromptVar) * new_capacity);
        if (new_vars == NULL) {
            return false;
        }
        vars->vars = new_vars;
        vars->capacity = new_capacity;
    }

    // Check if variable already exists and update it
    for (int i = 0; i < vars->count; i++) {
        if (strcmp(vars->vars[i].name, name) == 0) {
            free((void*)vars->vars[i].value);
            vars->vars[i].value = strdup(value);
            return true;
        }
    }

    // Add new variable
    vars->vars[vars->count].name = strdup(name);
    vars->vars[vars->count].value = strdup(value);
    vars->count++;

    return true;
}
// }}}

// {{{ prompt_vars_get
const char* prompt_vars_get(const PromptVars* vars, const char* name) {
    if (vars == NULL || name == NULL) {
        return NULL;
    }

    for (int i = 0; i < vars->count; i++) {
        if (strcmp(vars->vars[i].name, name) == 0) {
            return vars->vars[i].value;
        }
    }

    return NULL;
}
// }}}

// {{{ prompt_vars_free
void prompt_vars_free(PromptVars* vars) {
    if (vars == NULL) {
        return;
    }

    for (int i = 0; i < vars->count; i++) {
        free((void*)vars->vars[i].name);
        free((void*)vars->vars[i].value);
    }

    free(vars->vars);
    free(vars);
}
// }}}

// {{{ prompt_get_template
const PromptTemplate* prompt_get_template(PromptType type) {
    if (type < 0 || type >= PROMPT_COUNT) {
        return NULL;
    }
    return &TEMPLATES[type];
}
// }}}

// {{{ prompt_validate_vars
bool prompt_validate_vars(PromptType type, const PromptVars* vars,
                          const char** missing_var) {
    const PromptTemplate* tmpl = prompt_get_template(type);
    if (tmpl == NULL) {
        return false;
    }

    for (int i = 0; i < tmpl->var_count; i++) {
        const char* value = prompt_vars_get(vars, tmpl->required_vars[i]);
        if (value == NULL) {
            if (missing_var != NULL) {
                *missing_var = tmpl->required_vars[i];
            }
            return false;
        }
    }

    return true;
}
// }}}

// {{{ replace_var
// Helper to replace a single {var} placeholder with its value.
static char* replace_var(const char* text, const char* name, const char* value) {
    // Build placeholder string: {name}
    size_t placeholder_len = strlen(name) + 2;
    char* placeholder = malloc(placeholder_len + 1);
    snprintf(placeholder, placeholder_len + 1, "{%s}", name);

    // Find placeholder in text
    const char* pos = strstr(text, placeholder);
    if (pos == NULL) {
        free(placeholder);
        return strdup(text);
    }

    // Calculate new string size
    size_t text_len = strlen(text);
    size_t value_len = strlen(value);
    size_t new_len = text_len - placeholder_len + value_len;

    char* result = malloc(new_len + 1);
    if (result == NULL) {
        free(placeholder);
        return NULL;
    }

    // Copy parts: before placeholder, value, after placeholder
    size_t before_len = pos - text;
    memcpy(result, text, before_len);
    memcpy(result + before_len, value, value_len);
    strcpy(result + before_len + value_len, pos + placeholder_len);

    free(placeholder);

    // Recursively replace if there are more occurrences
    char* temp = replace_var(result, name, value);
    free(result);
    return temp;
}
// }}}

// {{{ prompt_build
char* prompt_build(PromptType type, const PromptVars* vars) {
    const PromptTemplate* tmpl = prompt_get_template(type);
    if (tmpl == NULL || vars == NULL) {
        return NULL;
    }

    // Validate all required vars are present
    const char* missing = NULL;
    if (!prompt_validate_vars(type, vars, &missing)) {
        return NULL;
    }

    // Start with the template
    char* result = strdup(tmpl->template_text);
    if (result == NULL) {
        return NULL;
    }

    // Replace each variable
    for (int i = 0; i < tmpl->var_count; i++) {
        const char* value = prompt_vars_get(vars, tmpl->required_vars[i]);
        char* temp = replace_var(result, tmpl->required_vars[i], value);
        free(result);
        result = temp;
        if (result == NULL) {
            return NULL;
        }
    }

    return result;
}
// }}}

// {{{ prompt_get_system_prompt
const char* prompt_get_system_prompt(void) {
    return SYSTEM_PROMPT;
}
// }}}

// {{{ prompt_chain_create
PromptChain* prompt_chain_create(void) {
    PromptChain* chain = malloc(sizeof(PromptChain));
    if (chain == NULL) {
        return NULL;
    }

    chain->count = 0;
    chain->types = malloc(sizeof(PromptType) * 8);
    chain->outputs = malloc(sizeof(char*) * 8);

    if (chain->types == NULL || chain->outputs == NULL) {
        free(chain->types);
        free(chain->outputs);
        free(chain);
        return NULL;
    }

    return chain;
}
// }}}

// {{{ prompt_chain_add
bool prompt_chain_add(PromptChain* chain, PromptType type) {
    if (chain == NULL || type < 0 || type >= PROMPT_COUNT) {
        return false;
    }

    // Simple implementation with fixed capacity
    if (chain->count >= 8) {
        return false;
    }

    chain->types[chain->count] = type;
    chain->outputs[chain->count] = NULL;
    chain->count++;

    return true;
}
// }}}

// {{{ prompt_chain_free
void prompt_chain_free(PromptChain* chain) {
    if (chain == NULL) {
        return;
    }

    for (int i = 0; i < chain->count; i++) {
        free(chain->outputs[i]);
    }

    free(chain->types);
    free(chain->outputs);
    free(chain);
}
// }}}

// {{{ prompt_chain_get_standard
const PromptType* prompt_chain_get_standard(const char* chain_name) {
    if (chain_name == NULL) {
        return NULL;
    }

    if (strcmp(chain_name, "turn") == 0) {
        return TURN_CHAIN;
    } else if (strcmp(chain_name, "card_play") == 0) {
        return CARD_PLAY_CHAIN;
    } else if (strcmp(chain_name, "combat") == 0) {
        return COMBAT_CHAIN;
    }

    return NULL;
}
// }}}
