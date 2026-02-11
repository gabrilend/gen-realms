/*
 * 02-prompts.h - Prompt Network Structure
 *
 * Defines prompt templates for narrative generation.
 * Supports variable interpolation, chaining, and system prompt management.
 * Templates produce consistent narrative style across all game events.
 */

#ifndef LLM_PROMPTS_H
#define LLM_PROMPTS_H

#include <stdbool.h>
#include <stddef.h>

// {{{ PromptType
typedef enum {
    PROMPT_WORLD_STATE,         // Describes current game state
    PROMPT_FORCE_DESCRIPTION,   // Describes a player's forces
    PROMPT_EVENT_NARRATION,     // Narrates a game event
    PROMPT_TRADE_ROW_SELECTION, // Selects cards for trade row
    PROMPT_TURN_SUMMARY,        // Summarizes a completed turn
    PROMPT_CARD_PLAYED,         // Describes a card being played
    PROMPT_ATTACK,              // Narrates an attack
    PROMPT_COUNT                // Number of prompt types
} PromptType;
// }}}

// {{{ PromptTemplate
typedef struct {
    PromptType type;
    const char* name;           // Human-readable name
    const char* template_text;  // Template with %s placeholders
    const char** required_vars; // Names of required variables
    int var_count;              // Number of required variables
} PromptTemplate;
// }}}

// {{{ PromptVar
typedef struct {
    const char* name;           // Variable name (e.g., "player_name")
    const char* value;          // Value to substitute
} PromptVar;
// }}}

// {{{ PromptVars
typedef struct {
    PromptVar* vars;
    int count;
    int capacity;
} PromptVars;
// }}}

// {{{ PromptChain
// Represents a sequence of prompts that build on each other.
typedef struct {
    PromptType* types;          // Array of prompt types in order
    int count;                  // Number of prompts in chain
    char** outputs;             // Outputs from each prompt
} PromptChain;
// }}}

// {{{ prompt_vars_create
// Creates a new empty PromptVars container.
PromptVars* prompt_vars_create(void);
// }}}

// {{{ prompt_vars_add
// Adds a variable to the container.
// Returns true on success.
bool prompt_vars_add(PromptVars* vars, const char* name, const char* value);
// }}}

// {{{ prompt_vars_get
// Gets a variable value by name.
// Returns NULL if not found.
const char* prompt_vars_get(const PromptVars* vars, const char* name);
// }}}

// {{{ prompt_vars_free
// Frees the PromptVars container and all contained strings.
void prompt_vars_free(PromptVars* vars);
// }}}

// {{{ prompt_get_template
// Gets the template for a prompt type.
// Returns NULL for invalid types.
const PromptTemplate* prompt_get_template(PromptType type);
// }}}

// {{{ prompt_build
// Builds a prompt from a template and variables.
// Caller must free the returned string.
// Returns NULL on error (missing required vars).
char* prompt_build(PromptType type, const PromptVars* vars);
// }}}

// {{{ prompt_validate_vars
// Checks if all required variables are present.
// Returns true if valid, false if missing vars.
bool prompt_validate_vars(PromptType type, const PromptVars* vars,
                          const char** missing_var);
// }}}

// {{{ prompt_get_system_prompt
// Returns the system prompt for narrative generation.
// This sets the tone and style for all LLM responses.
const char* prompt_get_system_prompt(void);
// }}}

// {{{ prompt_chain_create
// Creates a new prompt chain.
PromptChain* prompt_chain_create(void);
// }}}

// {{{ prompt_chain_add
// Adds a prompt type to the chain.
bool prompt_chain_add(PromptChain* chain, PromptType type);
// }}}

// {{{ prompt_chain_free
// Frees the chain and all outputs.
void prompt_chain_free(PromptChain* chain);
// }}}

// {{{ prompt_chain_get_types
// Standard chains for common narrative tasks.
// Returns array of prompt types (NULL-terminated).
const PromptType* prompt_chain_get_standard(const char* chain_name);
// }}}

#endif /* LLM_PROMPTS_H */
