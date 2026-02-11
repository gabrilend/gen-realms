/*
 * 06-context-manager.h - Context Window Management
 *
 * Manages the context window for LLM calls, tracking token usage,
 * managing entry priorities, and preventing context overflow.
 */

#ifndef LLM_CONTEXT_MANAGER_H
#define LLM_CONTEXT_MANAGER_H

#include <stdbool.h>
#include <time.h>

// {{{ ContextPriority
// Priority levels for context entries.
// Lower number = higher priority (won't be evicted first).
typedef enum {
    PRIORITY_SYSTEM = 0,        // Highest: system prompts, never evicted
    PRIORITY_CURRENT_TURN = 1,  // Current turn events
    PRIORITY_WORLD_STATE = 2,   // World state context
    PRIORITY_RECENT_EVENTS = 3, // Events from last 3 turns
    PRIORITY_FORCE_DESC = 4,    // Force descriptions
    PRIORITY_OLD_EVENTS = 5     // Lowest: old events, evicted first
} ContextPriority;
// }}}

// {{{ ContextEntry
// A single entry in the context window.
typedef struct {
    char* text;         // The context text
    int token_count;    // Estimated tokens for this entry
    ContextPriority priority;
    time_t added_at;    // When this entry was added
    bool is_summary;    // True if this is a summarized entry
} ContextEntry;
// }}}

// {{{ ContextManagerStats
// Statistics about context usage.
typedef struct {
    int total_entries;
    int current_tokens;
    int max_tokens;
    int eviction_count;
    int summary_count;
    float utilization;  // current_tokens / max_tokens
} ContextManagerStats;
// }}}

// {{{ ContextManager
// Manages the context window for LLM calls.
typedef struct {
    ContextEntry* entries;
    int entry_count;
    int max_entries;
    int max_tokens;
    int current_tokens;
    int eviction_count;
    int summary_count;
} ContextManager;
// }}}

// {{{ context_init
// Creates a new context manager.
// max_tokens: Maximum tokens allowed in the context window.
// Returns NULL on allocation failure.
ContextManager* context_init(int max_tokens);
// }}}

// {{{ context_free
// Frees all resources used by the context manager.
void context_free(ContextManager* cm);
// }}}

// {{{ context_estimate_tokens
// Estimates the token count for a text string.
// Uses a rough heuristic (~4 characters per token for English).
int context_estimate_tokens(const char* text);
// }}}

// {{{ context_get_stats
// Gets current statistics about context usage.
ContextManagerStats context_get_stats(ContextManager* cm);
// }}}

// {{{ context_get_available_tokens
// Returns the number of tokens available for new content.
int context_get_available_tokens(ContextManager* cm);
// }}}

// {{{ context_is_full
// Returns true if the context is at or near capacity.
// threshold: Percentage of max_tokens (0.0 to 1.0) considered "full".
bool context_is_full(ContextManager* cm, float threshold);
// }}}

// {{{ context_clear
// Removes all entries from the context manager.
void context_clear(ContextManager* cm);
// }}}

// {{{ context_clear_priority
// Removes all entries with a specific priority level.
void context_clear_priority(ContextManager* cm, ContextPriority priority);
// }}}

#endif /* LLM_CONTEXT_MANAGER_H */
