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

// {{{ context_add
// Adds a new entry to the context.
// Automatically estimates token count and evicts lowest priority entries if needed.
// Returns true if entry was added, false if it couldn't fit (even after eviction).
bool context_add(ContextManager* cm, const char* text, ContextPriority priority);
// }}}

// {{{ context_evict_lowest
// Removes the lowest priority entry from the context.
// If multiple entries have the same priority, removes the oldest one.
// Returns true if an entry was evicted, false if context is empty or only system entries remain.
bool context_evict_lowest(ContextManager* cm);
// }}}

// {{{ context_build_prompt
// Builds the final prompt string from all entries.
// Entries are sorted by priority (highest priority first).
// Caller must free the returned string.
// Returns NULL on allocation failure.
char* context_build_prompt(ContextManager* cm);
// }}}

// {{{ context_get_entry_count
// Returns the number of entries at a specific priority level.
int context_get_entry_count(ContextManager* cm, ContextPriority priority);
// }}}

// {{{ context_update_priority
// Updates the priority of entries matching a text prefix.
// Useful for demoting recent events to old events as time passes.
void context_update_priority(ContextManager* cm, ContextPriority old_priority,
                              ContextPriority new_priority);
// }}}

// {{{ context_remove_at
// Removes the entry at a specific index.
// Returns true if removed, false if index is out of bounds.
bool context_remove_at(ContextManager* cm, int index);
// }}}

// {{{ context_find_summarizable
// Finds entries that are candidates for summarization.
// Fills indices array with entry indices, returns count found.
// max_count limits how many entries to find.
int context_find_summarizable(ContextManager* cm, int* indices, int max_count);
// }}}

// {{{ context_get_entries_text
// Concatenates text from specified entries into a single string.
// Caller must free the returned string.
// Returns NULL on allocation failure.
char* context_get_entries_text(ContextManager* cm, int* indices, int count);
// }}}

// {{{ context_replace_with_summary
// Replaces multiple entries with a single summary entry.
// indices: Array of entry indices to remove (must be sorted ascending)
// count: Number of indices
// summary: The summary text to add
// Returns true on success, false on failure.
bool context_replace_with_summary(ContextManager* cm, int* indices, int count,
                                   const char* summary);
// }}}

// {{{ context_needs_summarization
// Checks if context should trigger summarization based on usage.
// threshold: Percentage (0.0-1.0) of capacity that triggers summarization.
// Returns true if context should be summarized.
bool context_needs_summarization(ContextManager* cm, float threshold);
// }}}

// {{{ context_mark_as_summary
// Marks the most recently added entry as a summary.
void context_mark_as_summary(ContextManager* cm);
// }}}

#endif /* LLM_CONTEXT_MANAGER_H */
