/*
 * test-prompts.c - Tests for Prompt Network Structure
 *
 * Validates prompt templates, variable interpolation, and chain creation.
 * Run with: gcc -o test-prompts test-prompts.c ../src/llm/02-prompts.c && ./test-prompts
 */

#include "../src/llm/02-prompts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

// {{{ test_vars_create
TEST(test_vars_create) {
    PromptVars* vars = prompt_vars_create();
    assert(vars != NULL);
    assert(vars->count == 0);

    prompt_vars_free(vars);
}
// }}}

// {{{ test_vars_add_get
TEST(test_vars_add_get) {
    PromptVars* vars = prompt_vars_create();

    bool added = prompt_vars_add(vars, "player_name", "Aldric");
    assert(added == true);
    assert(vars->count == 1);

    const char* value = prompt_vars_get(vars, "player_name");
    assert(value != NULL);
    assert(strcmp(value, "Aldric") == 0);

    // Non-existent var
    value = prompt_vars_get(vars, "nonexistent");
    assert(value == NULL);

    prompt_vars_free(vars);
}
// }}}

// {{{ test_vars_update
TEST(test_vars_update) {
    PromptVars* vars = prompt_vars_create();

    prompt_vars_add(vars, "player_name", "Aldric");
    prompt_vars_add(vars, "player_name", "Belinda");

    assert(vars->count == 1); // Should update, not add

    const char* value = prompt_vars_get(vars, "player_name");
    assert(strcmp(value, "Belinda") == 0);

    prompt_vars_free(vars);
}
// }}}

// {{{ test_get_template
TEST(test_get_template) {
    const PromptTemplate* tmpl = prompt_get_template(PROMPT_WORLD_STATE);
    assert(tmpl != NULL);
    assert(tmpl->type == PROMPT_WORLD_STATE);
    assert(strcmp(tmpl->name, "World State") == 0);
    assert(tmpl->var_count == 6);

    // Invalid type
    tmpl = prompt_get_template(PROMPT_COUNT);
    assert(tmpl == NULL);

    tmpl = prompt_get_template(-1);
    assert(tmpl == NULL);
}
// }}}

// {{{ test_validate_vars_success
TEST(test_validate_vars_success) {
    PromptVars* vars = prompt_vars_create();

    // Add all required vars for ATTACK
    prompt_vars_add(vars, "attacker", "Aldric");
    prompt_vars_add(vars, "defender", "Belinda");
    prompt_vars_add(vars, "damage", "5");
    prompt_vars_add(vars, "remaining_authority", "45");

    bool valid = prompt_validate_vars(PROMPT_ATTACK, vars, NULL);
    assert(valid == true);

    prompt_vars_free(vars);
}
// }}}

// {{{ test_validate_vars_missing
TEST(test_validate_vars_missing) {
    PromptVars* vars = prompt_vars_create();

    // Only add some vars
    prompt_vars_add(vars, "attacker", "Aldric");
    prompt_vars_add(vars, "defender", "Belinda");

    const char* missing = NULL;
    bool valid = prompt_validate_vars(PROMPT_ATTACK, vars, &missing);
    assert(valid == false);
    assert(missing != NULL);
    assert(strcmp(missing, "damage") == 0);

    prompt_vars_free(vars);
}
// }}}

// {{{ test_build_prompt
TEST(test_build_prompt) {
    PromptVars* vars = prompt_vars_create();

    prompt_vars_add(vars, "attacker", "Aldric");
    prompt_vars_add(vars, "defender", "Belinda");
    prompt_vars_add(vars, "damage", "5");
    prompt_vars_add(vars, "remaining_authority", "45");

    char* prompt = prompt_build(PROMPT_ATTACK, vars);
    assert(prompt != NULL);

    // Check that variables were substituted
    assert(strstr(prompt, "Aldric") != NULL);
    assert(strstr(prompt, "Belinda") != NULL);
    assert(strstr(prompt, "5 damage") != NULL);
    assert(strstr(prompt, "45 authority") != NULL);

    // Check that no placeholders remain
    assert(strstr(prompt, "{attacker}") == NULL);
    assert(strstr(prompt, "{defender}") == NULL);

    free(prompt);
    prompt_vars_free(vars);
}
// }}}

// {{{ test_build_prompt_missing_var
TEST(test_build_prompt_missing_var) {
    PromptVars* vars = prompt_vars_create();

    prompt_vars_add(vars, "attacker", "Aldric");
    // Missing other required vars

    char* prompt = prompt_build(PROMPT_ATTACK, vars);
    assert(prompt == NULL); // Should fail

    prompt_vars_free(vars);
}
// }}}

// {{{ test_system_prompt
TEST(test_system_prompt) {
    const char* sys = prompt_get_system_prompt();
    assert(sys != NULL);
    assert(strlen(sys) > 0);

    // Should contain key elements
    assert(strstr(sys, "Symbeline") != NULL);
    assert(strstr(sys, "narrator") != NULL);
}
// }}}

// {{{ test_chain_create
TEST(test_chain_create) {
    PromptChain* chain = prompt_chain_create();
    assert(chain != NULL);
    assert(chain->count == 0);

    prompt_chain_free(chain);
}
// }}}

// {{{ test_chain_add
TEST(test_chain_add) {
    PromptChain* chain = prompt_chain_create();

    bool added = prompt_chain_add(chain, PROMPT_WORLD_STATE);
    assert(added == true);
    assert(chain->count == 1);
    assert(chain->types[0] == PROMPT_WORLD_STATE);

    added = prompt_chain_add(chain, PROMPT_ATTACK);
    assert(added == true);
    assert(chain->count == 2);

    prompt_chain_free(chain);
}
// }}}

// {{{ test_chain_standard
TEST(test_chain_standard) {
    const PromptType* turn_chain = prompt_chain_get_standard("turn");
    assert(turn_chain != NULL);
    assert(turn_chain[0] == PROMPT_WORLD_STATE);

    const PromptType* combat_chain = prompt_chain_get_standard("combat");
    assert(combat_chain != NULL);
    assert(combat_chain[0] == PROMPT_ATTACK);

    // Unknown chain
    const PromptType* unknown = prompt_chain_get_standard("unknown");
    assert(unknown == NULL);
}
// }}}

// {{{ test_vars_free_null
TEST(test_vars_free_null) {
    // Should not crash
    prompt_vars_free(NULL);
}
// }}}

// {{{ test_chain_free_null
TEST(test_chain_free_null) {
    // Should not crash
    prompt_chain_free(NULL);
}
// }}}

// {{{ test_world_state_prompt
TEST(test_world_state_prompt) {
    PromptVars* vars = prompt_vars_create();

    prompt_vars_add(vars, "turn", "5");
    prompt_vars_add(vars, "player1_name", "Aldric");
    prompt_vars_add(vars, "player1_authority", "42");
    prompt_vars_add(vars, "player2_name", "Belinda");
    prompt_vars_add(vars, "player2_authority", "38");
    prompt_vars_add(vars, "phase", "Main");

    char* prompt = prompt_build(PROMPT_WORLD_STATE, vars);
    assert(prompt != NULL);

    assert(strstr(prompt, "Turn 5") != NULL);
    assert(strstr(prompt, "Aldric") != NULL);
    assert(strstr(prompt, "42 authority") != NULL);
    assert(strstr(prompt, "Belinda") != NULL);
    assert(strstr(prompt, "38 authority") != NULL);
    assert(strstr(prompt, "Main") != NULL);

    free(prompt);
    prompt_vars_free(vars);
}
// }}}

// {{{ main
int main(void) {
    printf("=== Prompt Network Structure Tests ===\n");

    RUN_TEST(test_vars_create);
    RUN_TEST(test_vars_add_get);
    RUN_TEST(test_vars_update);
    RUN_TEST(test_get_template);
    RUN_TEST(test_validate_vars_success);
    RUN_TEST(test_validate_vars_missing);
    RUN_TEST(test_build_prompt);
    RUN_TEST(test_build_prompt_missing_var);
    RUN_TEST(test_system_prompt);
    RUN_TEST(test_chain_create);
    RUN_TEST(test_chain_add);
    RUN_TEST(test_chain_standard);
    RUN_TEST(test_vars_free_null);
    RUN_TEST(test_chain_free_null);
    RUN_TEST(test_world_state_prompt);

    printf("\nAll tests passed!\n");
    return 0;
}
// }}}
