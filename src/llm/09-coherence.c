/*
 * 09-coherence.c - Narrative Coherence Recovery Implementation
 *
 * Detects and recovers from narrative inconsistencies by comparing
 * LLM output against game state. Uses pattern matching for detection
 * and world state rebuild for recovery.
 */

#include "09-coherence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Recovery narrative system prompt */
static const char* RECOVERY_SYSTEM_PROMPT =
    "You are the narrator for Symbeline Realms, a fantasy card game.\n"
    "The narrative context has been refreshed. Write a brief (1-2 sentences)\n"
    "transition that smoothly reestablishes the scene without drawing attention\n"
    "to any narrative discontinuity. Focus on the current moment.";

/* Recovery user prompt template */
static const char* RECOVERY_USER_PROMPT =
    "Current state: Turn %d. %s has %d authority. %s has %d authority.\n"
    "Tension level: %s. Dominant faction: %s.\n\n"
    "Write a brief transition to reestablish the narrative.";

/* Valid faction keywords for checking */
static const char* FACTION_KEYWORDS[] = {
    "neutral", "merchant", "guild", "wilds", "beast", "forest",
    "kingdom", "knight", "royal", "artificer", "construct", "golem"
};
static const int FACTION_KEYWORD_COUNT = 12;

/* {{{ strdup_safe */
static char* strdup_safe(const char* str) {
    if (str == NULL) return NULL;
    return strdup(str);
}
/* }}} */

/* {{{ str_contains_ignore_case
 * Case-insensitive substring search.
 */
static bool str_contains_ignore_case(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;

    size_t h_len = strlen(haystack);
    size_t n_len = strlen(needle);
    if (n_len > h_len) return false;

    for (size_t i = 0; i <= h_len - n_len; i++) {
        bool match = true;
        for (size_t j = 0; j < n_len && match; j++) {
            if (tolower((unsigned char)haystack[i + j]) !=
                tolower((unsigned char)needle[j])) {
                match = false;
            }
        }
        if (match) return true;
    }
    return false;
}
/* }}} */

/* {{{ extract_number_near
 * Extracts a number that appears near a keyword in text.
 * Looks both before and after the keyword.
 * Returns -1 if not found.
 */
static int extract_number_near(const char* text, const char* keyword) {
    if (!text || !keyword) return -1;

    const char* pos = strstr(text, keyword);
    if (!pos) return -1;

    /* Look for number BEFORE keyword (within 20 chars) */
    const char* search_start = pos - 20;
    if (search_start < text) search_start = text;

    for (const char* p = search_start; p < pos; p++) {
        if (isdigit(*p)) {
            return atoi(p);
        }
    }

    /* Look for number AFTER keyword */
    pos += strlen(keyword);
    while (*pos && (isspace(*pos) || ispunct(*pos))) {
        pos++;
    }
    if (isdigit(*pos)) {
        return atoi(pos);
    }

    return -1;
}
/* }}} */

/* {{{ coherence_log_entry_free */
static void coherence_log_entry_free(CoherenceLogEntry* entry) {
    if (!entry) return;
    free(entry->narrative_snippet);
    free(entry->issue_description);
}
/* }}} */

/* {{{ coherence_manager_create */
CoherenceManager* coherence_manager_create(LLMConfig* llm_config) {
    CoherenceManager* manager = malloc(sizeof(CoherenceManager));
    if (!manager) return NULL;

    manager->log = calloc(COHERENCE_MAX_LOG_ENTRIES, sizeof(CoherenceLogEntry));
    if (!manager->log) {
        free(manager);
        return NULL;
    }

    manager->log_count = 0;
    manager->log_cursor = 0;
    manager->total_checks = 0;
    manager->total_recoveries = 0;
    manager->consecutive_issues = 0;
    manager->llm_config = llm_config;

    return manager;
}
/* }}} */

/* {{{ coherence_manager_free */
void coherence_manager_free(CoherenceManager* manager) {
    if (!manager) return;

    for (int i = 0; i < COHERENCE_MAX_LOG_ENTRIES; i++) {
        coherence_log_entry_free(&manager->log[i]);
    }
    free(manager->log);
    /* Note: Does NOT free llm_config */
    free(manager);
}
/* }}} */

/* {{{ coherence_check_names */
bool coherence_check_names(const char* narrative, Game* game) {
    if (!narrative || !game) return true;  /* No narrative = OK */

    /* Check that player names appear if mentioned */
    for (int i = 0; i < game->player_count && i < MAX_PLAYERS; i++) {
        Player* player = game->players[i];
        if (!player || !player->name) continue;

        /* If "player" is mentioned, the name should also appear */
        char player_ref[32];
        snprintf(player_ref, sizeof(player_ref), "player %d", i + 1);

        if (str_contains_ignore_case(narrative, player_ref)) {
            /* Generic reference found - check if actual name also present */
            if (!str_contains_ignore_case(narrative, player->name)) {
                /* Name should be used instead of generic reference */
                return false;
            }
        }
    }

    return true;
}
/* }}} */

/* {{{ coherence_check_factions */
bool coherence_check_factions(const char* narrative) {
    if (!narrative) return true;

    /* Check for obviously wrong faction combinations */
    /* e.g., "Merchant Wilds" or "Kingdom Artificer" as single faction */

    /* For now, just verify at least one valid faction keyword appears */
    /* if any faction-related words are present */

    bool has_faction_word = false;
    for (int i = 0; i < FACTION_KEYWORD_COUNT; i++) {
        if (str_contains_ignore_case(narrative, FACTION_KEYWORDS[i])) {
            has_faction_word = true;
            break;
        }
    }

    /* If faction words present but we find impossible combinations */
    if (has_faction_word) {
        /* Check for contradictory pairings */
        if (str_contains_ignore_case(narrative, "merchant wilds") ||
            str_contains_ignore_case(narrative, "kingdom artificer") ||
            str_contains_ignore_case(narrative, "wilds kingdom")) {
            return false;
        }
    }

    return true;
}
/* }}} */

/* {{{ coherence_check_timeline */
bool coherence_check_timeline(const char* narrative, WorldState* world_state) {
    if (!narrative || !world_state) return true;

    /* Look for turn references in narrative */
    int mentioned_turn = extract_number_near(narrative, "turn");

    if (mentioned_turn > 0) {
        /* Turn mentioned should be close to actual turn */
        int diff = abs(mentioned_turn - world_state->turn_number);
        if (diff > 3) {
            /* Turn is way off - likely incoherent */
            return false;
        }
    }

    return true;
}
/* }}} */

/* {{{ coherence_check_authority */
bool coherence_check_authority(const char* narrative, Game* game) {
    if (!narrative || !game) return true;

    /* Look for authority references */
    int mentioned_auth = extract_number_near(narrative, "authority");
    if (mentioned_auth < 0) {
        mentioned_auth = extract_number_near(narrative, "health");
    }

    if (mentioned_auth > 0) {
        /* Check if any player is close to this value */
        bool plausible = false;
        for (int i = 0; i < game->player_count && i < MAX_PLAYERS; i++) {
            Player* player = game->players[i];
            if (!player) continue;

            int diff = abs(mentioned_auth - player->authority);
            if (diff <= 10) {
                plausible = true;
                break;
            }
        }

        if (!plausible && mentioned_auth > PLAYER_STARTING_AUTHORITY * 2) {
            /* Authority way too high - likely hallucinated */
            return false;
        }
    }

    return true;
}
/* }}} */

/* {{{ coherence_calculate_score */
static float coherence_calculate_score(CoherenceCheck* check) {
    float score = 0.0f;
    int factors = 0;

    if (check->names_consistent) {
        score += 1.0f;
    }
    factors++;

    if (check->faction_consistent) {
        score += 1.0f;
    }
    factors++;

    if (check->timeline_consistent) {
        score += 1.0f;
    }
    factors++;

    if (check->authority_plausible) {
        score += 1.0f;
    }
    factors++;

    /* Event reference is optional, give partial credit */
    if (check->event_referenced) {
        score += 0.5f;
    }
    factors++;

    return score / (float)factors;
}
/* }}} */

/* {{{ coherence_determine_level */
static CoherenceLevel coherence_determine_level(float score) {
    if (score >= COHERENCE_THRESHOLD_MINOR) {
        return COHERENCE_OK;
    } else if (score >= COHERENCE_THRESHOLD_MAJOR) {
        return COHERENCE_MINOR_ISSUE;
    } else if (score >= COHERENCE_THRESHOLD_RECOVERY) {
        return COHERENCE_MAJOR_ISSUE;
    } else {
        return COHERENCE_RECOVERY_NEEDED;
    }
}
/* }}} */

/* {{{ coherence_build_issue_description */
static char* coherence_build_issue_description(CoherenceCheck* check) {
    char buffer[512] = "";
    int offset = 0;

    if (!check->names_consistent) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "Name inconsistency. ");
    }
    if (!check->faction_consistent) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "Faction reference error. ");
    }
    if (!check->timeline_consistent) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "Timeline mismatch. ");
    }
    if (!check->authority_plausible) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "Authority value implausible. ");
    }

    if (offset == 0) {
        return strdup_safe("No issues detected.");
    }

    return strdup_safe(buffer);
}
/* }}} */

/* {{{ coherence_check */
CoherenceCheck* coherence_check(CoherenceManager* manager,
                                 const char* narrative,
                                 Game* game,
                                 WorldState* world_state) {
    CoherenceCheck* check = malloc(sizeof(CoherenceCheck));
    if (!check) return NULL;

    /* Perform individual checks */
    check->names_consistent = coherence_check_names(narrative, game);
    check->faction_consistent = coherence_check_factions(narrative);
    check->timeline_consistent = coherence_check_timeline(narrative, world_state);
    check->authority_plausible = coherence_check_authority(narrative, game);
    check->event_referenced = true;  /* Assume OK for now */

    /* Calculate overall score */
    check->overall_score = coherence_calculate_score(check);
    check->level = coherence_determine_level(check->overall_score);

    /* Build issue description */
    check->issue_description = coherence_build_issue_description(check);

    /* Update manager stats */
    if (manager) {
        manager->total_checks++;

        if (check->level == COHERENCE_OK) {
            manager->consecutive_issues = 0;
        } else {
            manager->consecutive_issues++;
        }
    }

    return check;
}
/* }}} */

/* {{{ coherence_check_free */
void coherence_check_free(CoherenceCheck* check) {
    if (!check) return;
    free(check->issue_description);
    free(check);
}
/* }}} */

/* {{{ coherence_rebuild_world_state */
void coherence_rebuild_world_state(WorldState* world_state, Game* game) {
    if (!world_state || !game) return;

    /* Clear event history */
    for (int i = 0; i < WORLD_STATE_MAX_EVENTS; i++) {
        if (world_state->events[i]) {
            free(world_state->events[i]->event_type);
            free(world_state->events[i]->description);
            free(world_state->events[i]);
            world_state->events[i] = NULL;
        }
    }
    world_state->event_count = 0;
    world_state->event_cursor = 0;

    /* Reset faction counts */
    for (int i = 0; i < FACTION_COUNT; i++) {
        world_state->faction_card_counts[i] = 0;
    }

    /* Count cards currently in play to rebuild faction counts */
    for (int p = 0; p < game->player_count && p < MAX_PLAYERS; p++) {
        Player* player = game->players[p];
        if (!player || !player->deck) continue;

        /* Count cards in played area */
        for (int c = 0; c < player->deck->played_count; c++) {
            CardInstance* card = player->deck->played[c];
            if (card && card->type) {
                world_state->faction_card_counts[card->type->faction]++;
            }
        }

        /* Count frontier bases */
        for (int c = 0; c < player->deck->frontier_base_count; c++) {
            CardInstance* card = player->deck->frontier_bases[c];
            if (card && card->type) {
                world_state->faction_card_counts[card->type->faction]++;
            }
        }

        /* Count interior bases */
        for (int c = 0; c < player->deck->interior_base_count; c++) {
            CardInstance* card = player->deck->interior_bases[c];
            if (card && card->type) {
                world_state->faction_card_counts[card->type->faction]++;
            }
        }
    }

    /* Update turn and tension from game state */
    world_state->turn_number = game->turn_number;
    world_state->last_update_turn = game->turn_number;
    world_state_calculate_tension(world_state, game);

    /* Force battlefield description update */
    world_state_update(world_state, game);
}
/* }}} */

/* {{{ coherence_get_tension_description */
static const char* coherence_get_tension_description(float tension) {
    if (tension > 0.8f) return "climactic";
    if (tension > 0.5f) return "intense";
    if (tension > 0.2f) return "building";
    return "calm";
}
/* }}} */

/* {{{ coherence_generate_recovery_narrative */
char* coherence_generate_recovery_narrative(CoherenceManager* manager,
                                             Game* game,
                                             WorldState* world_state) {
    if (!game || !world_state) return NULL;

    /* If no LLM, return a default recovery narrative */
    if (!manager || !manager->llm_config) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer),
                 "The mists of battle shift, revealing the current state of "
                 "the conflict. Turn %d unfolds as %s and %s continue their "
                 "struggle for dominion.",
                 game->turn_number,
                 game->players[0] ? game->players[0]->name : "Player 1",
                 game->players[1] ? game->players[1]->name : "Player 2");
        return strdup_safe(buffer);
    }

    /* Build prompt for LLM */
    char prompt[1024];
    snprintf(prompt, sizeof(prompt), RECOVERY_USER_PROMPT,
             game->turn_number,
             game->players[0] ? game->players[0]->name : "Player 1",
             game->players[0] ? game->players[0]->authority : 0,
             game->players[1] ? game->players[1]->name : "Player 2",
             game->players[1] ? game->players[1]->authority : 0,
             coherence_get_tension_description(world_state->tension),
             world_state_get_faction_name(world_state->dominant_faction));

    /* Call LLM */
    LLMResponse* response = llm_request(manager->llm_config,
                                         RECOVERY_SYSTEM_PROMPT,
                                         prompt);

    if (!response || !response->success || !response->text) {
        llm_response_free(response);
        /* Fallback to default */
        return coherence_generate_recovery_narrative(NULL, game, world_state);
    }

    char* narrative = strdup_safe(response->text);
    llm_response_free(response);

    return narrative;
}
/* }}} */

/* {{{ coherence_recover */
char* coherence_recover(CoherenceManager* manager,
                        Game* game,
                        WorldState* world_state) {
    if (!game || !world_state) return NULL;

    /* Rebuild world state from game data */
    coherence_rebuild_world_state(world_state, game);

    /* Generate recovery narrative */
    char* narrative = coherence_generate_recovery_narrative(manager, game,
                                                             world_state);

    /* Update stats */
    if (manager) {
        manager->total_recoveries++;
        manager->consecutive_issues = 0;
    }

    return narrative;
}
/* }}} */

/* {{{ coherence_should_check */
bool coherence_should_check(CoherenceManager* manager, Game* game) {
    if (!manager || !game) return false;

    /* Always check after recovery */
    if (manager->consecutive_issues > 0) {
        return true;
    }

    /* Check every 3rd turn normally */
    if (game->turn_number % 3 == 0) {
        return true;
    }

    /* Check at significant moments */
    for (int i = 0; i < game->player_count && i < MAX_PLAYERS; i++) {
        Player* player = game->players[i];
        if (!player) continue;

        /* Check when player is low on authority */
        if (player->authority <= 10) {
            return true;
        }
    }

    return false;
}
/* }}} */

/* {{{ coherence_get_level_name */
const char* coherence_get_level_name(CoherenceLevel level) {
    switch (level) {
        case COHERENCE_OK: return "OK";
        case COHERENCE_MINOR_ISSUE: return "Minor Issue";
        case COHERENCE_MAJOR_ISSUE: return "Major Issue";
        case COHERENCE_RECOVERY_NEEDED: return "Recovery Needed";
        default: return "Unknown";
    }
}
/* }}} */

/* {{{ coherence_get_stats */
void coherence_get_stats(CoherenceManager* manager,
                          int* out_checks,
                          int* out_recoveries,
                          float* out_avg_score) {
    if (!manager) {
        if (out_checks) *out_checks = 0;
        if (out_recoveries) *out_recoveries = 0;
        if (out_avg_score) *out_avg_score = 0.0f;
        return;
    }

    if (out_checks) *out_checks = manager->total_checks;
    if (out_recoveries) *out_recoveries = manager->total_recoveries;

    if (out_avg_score) {
        if (manager->log_count == 0) {
            *out_avg_score = 1.0f;  /* No checks = assume perfect */
        } else {
            float sum = 0.0f;
            for (int i = 0; i < manager->log_count; i++) {
                sum += manager->log[i].score;
            }
            *out_avg_score = sum / manager->log_count;
        }
    }
}
/* }}} */

/* {{{ coherence_log_entry */
void coherence_log_entry(CoherenceManager* manager,
                          CoherenceCheck* check,
                          const char* narrative,
                          bool recovery_triggered) {
    if (!manager || !check) return;

    /* Free old entry at cursor */
    coherence_log_entry_free(&manager->log[manager->log_cursor]);

    /* Create new entry */
    CoherenceLogEntry* entry = &manager->log[manager->log_cursor];
    entry->turn_number = 0;  /* Would need game reference */
    entry->level = check->level;
    entry->score = check->overall_score;
    entry->recovery_triggered = recovery_triggered;
    entry->issue_description = strdup_safe(check->issue_description);

    /* Store snippet of narrative */
    if (narrative) {
        size_t len = strlen(narrative);
        if (len > 200) len = 200;
        entry->narrative_snippet = malloc(len + 1);
        if (entry->narrative_snippet) {
            strncpy(entry->narrative_snippet, narrative, len);
            entry->narrative_snippet[len] = '\0';
        }
    } else {
        entry->narrative_snippet = NULL;
    }

    /* Advance cursor */
    manager->log_cursor = (manager->log_cursor + 1) % COHERENCE_MAX_LOG_ENTRIES;
    if (manager->log_count < COHERENCE_MAX_LOG_ENTRIES) {
        manager->log_count++;
    }
}
/* }}} */

/* {{{ coherence_get_recent_issues */
char* coherence_get_recent_issues(CoherenceManager* manager, int max_entries) {
    if (!manager || manager->log_count == 0) {
        return strdup_safe("No coherence issues logged.");
    }

    if (max_entries > manager->log_count) {
        max_entries = manager->log_count;
    }

    /* Calculate buffer size */
    size_t total_size = 256;
    for (int i = 0; i < max_entries; i++) {
        int idx = (manager->log_cursor - 1 - i + COHERENCE_MAX_LOG_ENTRIES) %
                  COHERENCE_MAX_LOG_ENTRIES;
        if (manager->log[idx].issue_description) {
            total_size += strlen(manager->log[idx].issue_description) + 100;
        }
    }

    char* result = malloc(total_size);
    if (!result) return NULL;
    result[0] = '\0';

    strcat(result, "Recent coherence checks:\n");

    int issues_found = 0;
    for (int i = 0; i < max_entries; i++) {
        int idx = (manager->log_cursor - 1 - i + COHERENCE_MAX_LOG_ENTRIES) %
                  COHERENCE_MAX_LOG_ENTRIES;
        CoherenceLogEntry* entry = &manager->log[idx];

        if (entry->level != COHERENCE_OK) {
            char line[256];
            snprintf(line, sizeof(line),
                     "- [%s] Score: %.2f%s%s\n",
                     coherence_get_level_name(entry->level),
                     entry->score,
                     entry->recovery_triggered ? " (recovered)" : "",
                     entry->issue_description ? entry->issue_description : "");
            strcat(result, line);
            issues_found++;
        }
    }

    if (issues_found == 0) {
        free(result);
        return strdup_safe("No coherence issues in recent history.");
    }

    return result;
}
/* }}} */
