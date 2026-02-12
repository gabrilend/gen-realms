/*
 * 09-coherence.h - Narrative Coherence Recovery
 *
 * Detects narrative inconsistencies in LLM-generated content and
 * recovers by rebuilding world state from game data. Ensures smooth
 * transitions after recovery to maintain immersion.
 */

#ifndef LLM_COHERENCE_H
#define LLM_COHERENCE_H

#include "../core/05-game.h"
#include "03-world-state.h"
#include "01-api-client.h"
#include <stdbool.h>

/* Thresholds for coherence levels */
#define COHERENCE_THRESHOLD_MINOR 0.7f
#define COHERENCE_THRESHOLD_MAJOR 0.5f
#define COHERENCE_THRESHOLD_RECOVERY 0.3f

/* Maximum log entries to keep */
#define COHERENCE_MAX_LOG_ENTRIES 50

/* {{{ CoherenceLevel
 * Severity of coherence issues detected.
 */
typedef enum {
    COHERENCE_OK,              /* No issues detected */
    COHERENCE_MINOR_ISSUE,     /* Minor inconsistencies, log but continue */
    COHERENCE_MAJOR_ISSUE,     /* Significant issues, may need intervention */
    COHERENCE_RECOVERY_NEEDED  /* Critical issues, must rebuild state */
} CoherenceLevel;
/* }}} */

/* {{{ CoherenceCheck
 * Results of checking a narrative for coherence.
 */
typedef struct {
    bool names_consistent;     /* Player names match game state */
    bool faction_consistent;   /* Faction references are valid */
    bool timeline_consistent;  /* Turn numbers make sense */
    bool authority_plausible;  /* Authority values are plausible */
    bool event_referenced;     /* Recent events properly referenced */

    float overall_score;       /* Combined coherence score 0.0-1.0 */
    CoherenceLevel level;      /* Determined coherence level */

    char* issue_description;   /* Human-readable issue description */
} CoherenceCheck;
/* }}} */

/* {{{ CoherenceLogEntry
 * Record of a coherence check for debugging.
 */
typedef struct {
    int turn_number;           /* Game turn when checked */
    CoherenceLevel level;      /* Coherence level detected */
    float score;               /* Overall score */
    char* narrative_snippet;   /* First 200 chars of checked narrative */
    char* issue_description;   /* What went wrong (if anything) */
    bool recovery_triggered;   /* Whether recovery was performed */
} CoherenceLogEntry;
/* }}} */

/* {{{ CoherenceManager
 * Manages coherence checking and recovery.
 */
typedef struct {
    CoherenceLogEntry* log;    /* Circular log of checks */
    int log_count;             /* Number of entries in log */
    int log_cursor;            /* Next write position */

    int total_checks;          /* Total checks performed */
    int total_recoveries;      /* Total recoveries triggered */
    int consecutive_issues;    /* Consecutive issues without OK */

    LLMConfig* llm_config;     /* Config for recovery narration (optional) */
} CoherenceManager;
/* }}} */

/* {{{ coherence_manager_create
 * Creates a new coherence manager.
 * @param llm_config - Optional LLM config for recovery narration
 */
CoherenceManager* coherence_manager_create(LLMConfig* llm_config);
/* }}} */

/* {{{ coherence_manager_free
 * Frees all memory associated with manager.
 * Does NOT free llm_config.
 */
void coherence_manager_free(CoherenceManager* manager);
/* }}} */

/* {{{ coherence_check
 * Checks a narrative for coherence with game state.
 * @param manager - The coherence manager
 * @param narrative - The narrative text to check
 * @param game - Current game state
 * @param world_state - Current world state
 * @return Coherence check results (caller must free with coherence_check_free)
 */
CoherenceCheck* coherence_check(CoherenceManager* manager,
                                 const char* narrative,
                                 Game* game,
                                 WorldState* world_state);
/* }}} */

/* {{{ coherence_check_free
 * Frees a coherence check result.
 */
void coherence_check_free(CoherenceCheck* check);
/* }}} */

/* {{{ coherence_check_names
 * Checks if player names in narrative match game state.
 * @param narrative - Text to check
 * @param game - Current game state
 * @return true if names are consistent
 */
bool coherence_check_names(const char* narrative, Game* game);
/* }}} */

/* {{{ coherence_check_factions
 * Checks if faction references in narrative are valid.
 * @param narrative - Text to check
 * @return true if all faction references are valid
 */
bool coherence_check_factions(const char* narrative);
/* }}} */

/* {{{ coherence_check_timeline
 * Checks if timeline references are consistent.
 * @param narrative - Text to check
 * @param world_state - Current world state
 * @return true if timeline is consistent
 */
bool coherence_check_timeline(const char* narrative, WorldState* world_state);
/* }}} */

/* {{{ coherence_check_authority
 * Checks if authority references are plausible.
 * @param narrative - Text to check
 * @param game - Current game state
 * @return true if authority values are plausible
 */
bool coherence_check_authority(const char* narrative, Game* game);
/* }}} */

/* {{{ coherence_recover
 * Performs coherence recovery by rebuilding world state.
 * @param manager - The coherence manager
 * @param game - Current game state
 * @param world_state - World state to rebuild
 * @return Recovery narrative (caller must free) or NULL on failure
 */
char* coherence_recover(CoherenceManager* manager,
                        Game* game,
                        WorldState* world_state);
/* }}} */

/* {{{ coherence_rebuild_world_state
 * Rebuilds world state from current game state.
 * @param world_state - World state to rebuild
 * @param game - Current game state to rebuild from
 */
void coherence_rebuild_world_state(WorldState* world_state, Game* game);
/* }}} */

/* {{{ coherence_generate_recovery_narrative
 * Generates a smooth transition narrative after recovery.
 * @param manager - The coherence manager (for LLM access)
 * @param game - Current game state
 * @param world_state - Rebuilt world state
 * @return Recovery narrative (caller must free) or NULL
 */
char* coherence_generate_recovery_narrative(CoherenceManager* manager,
                                             Game* game,
                                             WorldState* world_state);
/* }}} */

/* {{{ coherence_should_check
 * Determines if coherence should be checked based on conditions.
 * Avoids checking too frequently to reduce overhead.
 * @param manager - The coherence manager
 * @param game - Current game state
 * @return true if coherence should be checked
 */
bool coherence_should_check(CoherenceManager* manager, Game* game);
/* }}} */

/* {{{ coherence_get_level_name
 * Returns human-readable name for coherence level.
 */
const char* coherence_get_level_name(CoherenceLevel level);
/* }}} */

/* {{{ coherence_get_stats
 * Returns coherence statistics.
 * @param manager - The coherence manager
 * @param out_checks - Output: total checks
 * @param out_recoveries - Output: total recoveries
 * @param out_avg_score - Output: average coherence score
 */
void coherence_get_stats(CoherenceManager* manager,
                          int* out_checks,
                          int* out_recoveries,
                          float* out_avg_score);
/* }}} */

/* {{{ coherence_log_entry
 * Logs a coherence check result.
 * @param manager - The coherence manager
 * @param check - Check result to log
 * @param narrative - Original narrative (first 200 chars stored)
 * @param recovery_triggered - Whether recovery was performed
 */
void coherence_log_entry(CoherenceManager* manager,
                          CoherenceCheck* check,
                          const char* narrative,
                          bool recovery_triggered);
/* }}} */

/* {{{ coherence_get_recent_issues
 * Returns description of recent coherence issues.
 * Caller must free returned string.
 * @param manager - The coherence manager
 * @param max_entries - Maximum entries to include
 * @return Description string or NULL
 */
char* coherence_get_recent_issues(CoherenceManager* manager, int max_entries);
/* }}} */

#endif /* LLM_COHERENCE_H */
