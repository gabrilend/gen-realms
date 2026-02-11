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
