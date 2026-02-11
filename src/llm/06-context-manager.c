/*
 * 06-context-manager.c - Context Window Management Implementation
 *
 * Manages the context window for LLM calls. Tracks token usage,
 * handles entry priorities, and provides statistics for monitoring.
 */

#include "06-context-manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// {{{ Constants
#define DEFAULT_MAX_ENTRIES 100
#define CHARS_PER_TOKEN 4  // Rough estimate for English text
// }}}

// {{{ context_init
ContextManager* context_init(int max_tokens) {
    if (max_tokens <= 0) {
        return NULL;
    }

    ContextManager* cm = malloc(sizeof(ContextManager));
    if (cm == NULL) {
        return NULL;
    }

    cm->entries = calloc(DEFAULT_MAX_ENTRIES, sizeof(ContextEntry));
    if (cm->entries == NULL) {
        free(cm);
        return NULL;
    }

    cm->entry_count = 0;
    cm->max_entries = DEFAULT_MAX_ENTRIES;
    cm->max_tokens = max_tokens;
    cm->current_tokens = 0;
    cm->eviction_count = 0;
    cm->summary_count = 0;

    return cm;
}
// }}}

// {{{ context_free
void context_free(ContextManager* cm) {
    if (cm == NULL) {
        return;
    }

    // Free all entry text
    for (int i = 0; i < cm->entry_count; i++) {
        free(cm->entries[i].text);
    }

    free(cm->entries);
    free(cm);
}
// }}}

// {{{ context_estimate_tokens
int context_estimate_tokens(const char* text) {
    if (text == NULL) {
        return 0;
    }

    size_t len = strlen(text);
    // Round up: (len + CHARS_PER_TOKEN - 1) / CHARS_PER_TOKEN
    return (int)((len + CHARS_PER_TOKEN - 1) / CHARS_PER_TOKEN);
}
// }}}

// {{{ context_get_stats
ContextManagerStats context_get_stats(ContextManager* cm) {
    ContextManagerStats stats = {0};

    if (cm == NULL) {
        return stats;
    }

    stats.total_entries = cm->entry_count;
    stats.current_tokens = cm->current_tokens;
    stats.max_tokens = cm->max_tokens;
    stats.eviction_count = cm->eviction_count;
    stats.summary_count = cm->summary_count;

    if (cm->max_tokens > 0) {
        stats.utilization = (float)cm->current_tokens / (float)cm->max_tokens;
    } else {
        stats.utilization = 0.0f;
    }

    return stats;
}
// }}}

// {{{ context_get_available_tokens
int context_get_available_tokens(ContextManager* cm) {
    if (cm == NULL) {
        return 0;
    }

    int available = cm->max_tokens - cm->current_tokens;
    return available > 0 ? available : 0;
}
// }}}

// {{{ context_is_full
bool context_is_full(ContextManager* cm, float threshold) {
    if (cm == NULL || cm->max_tokens <= 0) {
        return true;
    }

    // Clamp threshold to valid range
    if (threshold < 0.0f) threshold = 0.0f;
    if (threshold > 1.0f) threshold = 1.0f;

    float utilization = (float)cm->current_tokens / (float)cm->max_tokens;
    return utilization >= threshold;
}
// }}}

// {{{ context_clear
void context_clear(ContextManager* cm) {
    if (cm == NULL) {
        return;
    }

    // Free all entry text
    for (int i = 0; i < cm->entry_count; i++) {
        free(cm->entries[i].text);
        cm->entries[i].text = NULL;
    }

    cm->entry_count = 0;
    cm->current_tokens = 0;
}
// }}}

// {{{ context_clear_priority
void context_clear_priority(ContextManager* cm, ContextPriority priority) {
    if (cm == NULL) {
        return;
    }

    int write_idx = 0;

    // Compact the array, removing entries with matching priority
    for (int read_idx = 0; read_idx < cm->entry_count; read_idx++) {
        if (cm->entries[read_idx].priority == priority) {
            // Free this entry and update token count
            cm->current_tokens -= cm->entries[read_idx].token_count;
            free(cm->entries[read_idx].text);
        } else {
            // Keep this entry
            if (write_idx != read_idx) {
                cm->entries[write_idx] = cm->entries[read_idx];
            }
            write_idx++;
        }
    }

    // Clear remaining slots
    for (int i = write_idx; i < cm->entry_count; i++) {
        cm->entries[i].text = NULL;
        cm->entries[i].token_count = 0;
    }

    cm->entry_count = write_idx;
}
// }}}

// {{{ compare_entries_by_priority
// Comparison function for qsort - sorts by priority (ascending = highest priority first)
static int compare_entries_by_priority(const void* a, const void* b) {
    const ContextEntry* entry_a = (const ContextEntry*)a;
    const ContextEntry* entry_b = (const ContextEntry*)b;

    // Lower priority number = higher priority, should come first
    if (entry_a->priority != entry_b->priority) {
        return entry_a->priority - entry_b->priority;
    }

    // Same priority: older entries (lower timestamp) come first
    if (entry_a->added_at < entry_b->added_at) return -1;
    if (entry_a->added_at > entry_b->added_at) return 1;
    return 0;
}
// }}}

// {{{ context_evict_lowest
bool context_evict_lowest(ContextManager* cm) {
    if (cm == NULL || cm->entry_count == 0) {
        return false;
    }

    // Find lowest priority (highest number) non-system entry
    int lowest_idx = -1;
    ContextPriority lowest_priority = PRIORITY_SYSTEM;  // Start with highest priority
    time_t oldest_time = 0;
    bool found_candidate = false;

    for (int i = 0; i < cm->entry_count; i++) {
        // Never evict system entries
        if (cm->entries[i].priority == PRIORITY_SYSTEM) {
            continue;
        }

        // Found first evictable candidate or a lower priority entry
        if (!found_candidate || cm->entries[i].priority > lowest_priority) {
            lowest_priority = cm->entries[i].priority;
            lowest_idx = i;
            oldest_time = cm->entries[i].added_at;
            found_candidate = true;
        }
        // Same priority: prefer to evict older entries
        else if (cm->entries[i].priority == lowest_priority &&
                 cm->entries[i].added_at < oldest_time) {
            lowest_idx = i;
            oldest_time = cm->entries[i].added_at;
        }
    }

    // No evictable entry found
    if (lowest_idx < 0) {
        return false;
    }

    // Remove the entry
    cm->current_tokens -= cm->entries[lowest_idx].token_count;
    free(cm->entries[lowest_idx].text);
    cm->eviction_count++;

    // Shift remaining entries down
    for (int i = lowest_idx; i < cm->entry_count - 1; i++) {
        cm->entries[i] = cm->entries[i + 1];
    }
    cm->entry_count--;

    // Clear the now-unused slot
    cm->entries[cm->entry_count].text = NULL;
    cm->entries[cm->entry_count].token_count = 0;

    return true;
}
// }}}

// {{{ context_add
bool context_add(ContextManager* cm, const char* text, ContextPriority priority) {
    if (cm == NULL || text == NULL) {
        return false;
    }

    int tokens = context_estimate_tokens(text);

    // Check if entry would exceed max_tokens even in an empty context
    if (tokens > cm->max_tokens) {
        return false;  // Entry is too large
    }

    // Evict entries until we have room
    while (cm->current_tokens + tokens > cm->max_tokens) {
        if (!context_evict_lowest(cm)) {
            return false;  // Can't make room
        }
    }

    // Check if we have room in the entries array
    if (cm->entry_count >= cm->max_entries) {
        // Need to evict to make room in array
        if (!context_evict_lowest(cm)) {
            return false;
        }
    }

    // Add the new entry
    ContextEntry* entry = &cm->entries[cm->entry_count];
    entry->text = strdup(text);
    if (entry->text == NULL) {
        return false;  // Allocation failed
    }

    entry->token_count = tokens;
    entry->priority = priority;
    entry->added_at = time(NULL);
    entry->is_summary = false;

    cm->current_tokens += tokens;
    cm->entry_count++;

    return true;
}
// }}}

// {{{ context_build_prompt
char* context_build_prompt(ContextManager* cm) {
    if (cm == NULL || cm->entry_count == 0) {
        char* empty = strdup("");
        return empty;
    }

    // Sort entries by priority (creates a stable order)
    qsort(cm->entries, cm->entry_count, sizeof(ContextEntry),
          compare_entries_by_priority);

    // Calculate total size needed
    size_t total_size = 1;  // For null terminator
    for (int i = 0; i < cm->entry_count; i++) {
        total_size += strlen(cm->entries[i].text) + 2;  // +2 for "\n\n"
    }

    // Allocate and build the prompt
    char* prompt = malloc(total_size);
    if (prompt == NULL) {
        return NULL;
    }

    prompt[0] = '\0';
    for (int i = 0; i < cm->entry_count; i++) {
        strcat(prompt, cm->entries[i].text);
        if (i < cm->entry_count - 1) {
            strcat(prompt, "\n\n");
        }
    }

    return prompt;
}
// }}}

// {{{ context_get_entry_count
int context_get_entry_count(ContextManager* cm, ContextPriority priority) {
    if (cm == NULL) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < cm->entry_count; i++) {
        if (cm->entries[i].priority == priority) {
            count++;
        }
    }

    return count;
}
// }}}

// {{{ context_update_priority
void context_update_priority(ContextManager* cm, ContextPriority old_priority,
                              ContextPriority new_priority) {
    if (cm == NULL) {
        return;
    }

    for (int i = 0; i < cm->entry_count; i++) {
        if (cm->entries[i].priority == old_priority) {
            cm->entries[i].priority = new_priority;
        }
    }
}
// }}}

// {{{ context_remove_at
bool context_remove_at(ContextManager* cm, int index) {
    if (cm == NULL || index < 0 || index >= cm->entry_count) {
        return false;
    }

    // Update token count
    cm->current_tokens -= cm->entries[index].token_count;

    // Free the entry text
    free(cm->entries[index].text);

    // Shift remaining entries down
    for (int i = index; i < cm->entry_count - 1; i++) {
        cm->entries[i] = cm->entries[i + 1];
    }
    cm->entry_count--;

    // Clear the now-unused slot
    cm->entries[cm->entry_count].text = NULL;
    cm->entries[cm->entry_count].token_count = 0;

    return true;
}
// }}}

// {{{ context_find_summarizable
int context_find_summarizable(ContextManager* cm, int* indices, int max_count) {
    if (cm == NULL || indices == NULL || max_count <= 0) {
        return 0;
    }

    int count = 0;

    // Find old, low-priority, non-summary entries
    for (int i = 0; i < cm->entry_count && count < max_count; i++) {
        // Only summarize old events and force descriptions
        if (cm->entries[i].priority >= PRIORITY_FORCE_DESC &&
            !cm->entries[i].is_summary) {
            indices[count++] = i;
        }
    }

    return count;
}
// }}}

// {{{ context_get_entries_text
char* context_get_entries_text(ContextManager* cm, int* indices, int count) {
    if (cm == NULL || indices == NULL || count <= 0) {
        return NULL;
    }

    // Calculate total length needed
    size_t total_len = 1;  // For null terminator
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        if (idx >= 0 && idx < cm->entry_count && cm->entries[idx].text != NULL) {
            total_len += strlen(cm->entries[idx].text) + 1;  // +1 for newline
        }
    }

    // Allocate and build combined text
    char* combined = malloc(total_len);
    if (combined == NULL) {
        return NULL;
    }

    combined[0] = '\0';
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        if (idx >= 0 && idx < cm->entry_count && cm->entries[idx].text != NULL) {
            strcat(combined, cm->entries[idx].text);
            if (i < count - 1) {
                strcat(combined, "\n");
            }
        }
    }

    return combined;
}
// }}}

// {{{ context_replace_with_summary
bool context_replace_with_summary(ContextManager* cm, int* indices, int count,
                                   const char* summary) {
    if (cm == NULL || indices == NULL || count <= 0 || summary == NULL) {
        return false;
    }

    // Remove entries in reverse order to maintain indices
    for (int i = count - 1; i >= 0; i--) {
        context_remove_at(cm, indices[i]);
    }

    // Add the summary as a single entry
    bool added = context_add(cm, summary, PRIORITY_OLD_EVENTS);
    if (added) {
        context_mark_as_summary(cm);
        cm->summary_count++;
    }

    return added;
}
// }}}

// {{{ context_needs_summarization
bool context_needs_summarization(ContextManager* cm, float threshold) {
    if (cm == NULL || cm->max_tokens <= 0) {
        return false;
    }

    // Clamp threshold
    if (threshold < 0.0f) threshold = 0.0f;
    if (threshold > 1.0f) threshold = 1.0f;

    float utilization = (float)cm->current_tokens / (float)cm->max_tokens;
    return utilization >= threshold;
}
// }}}

// {{{ context_mark_as_summary
void context_mark_as_summary(ContextManager* cm) {
    if (cm == NULL || cm->entry_count == 0) {
        return;
    }

    cm->entries[cm->entry_count - 1].is_summary = true;
}
// }}}
