/*
 * 07-narrative-cache.c - Narrative Caching Implementation
 *
 * Caches generated narratives to reduce LLM API calls.
 * Uses LRU eviction when cache is full.
 */

#include "07-narrative-cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// {{{ Constants
#define SIGNATURE_BUFFER_SIZE 256
// }}}

// {{{ narrative_cache_init
NarrativeCache* narrative_cache_init(int max_entries, int ttl_seconds) {
    if (max_entries <= 0) {
        return NULL;
    }

    NarrativeCache* cache = malloc(sizeof(NarrativeCache));
    if (cache == NULL) {
        return NULL;
    }

    cache->entries = calloc(max_entries, sizeof(NarrativeCacheEntry));
    if (cache->entries == NULL) {
        free(cache);
        return NULL;
    }

    cache->count = 0;
    cache->max_entries = max_entries;
    cache->ttl_seconds = ttl_seconds;
    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    cache->expirations = 0;

    return cache;
}
// }}}

// {{{ narrative_cache_free
void narrative_cache_free(NarrativeCache* cache) {
    if (cache == NULL) {
        return;
    }

    // Free all entry strings
    for (int i = 0; i < cache->count; i++) {
        free(cache->entries[i].signature);
        free(cache->entries[i].narrative);
    }

    free(cache->entries);
    free(cache);
}
// }}}

// {{{ find_entry_index
// Finds the index of an entry by signature.
// Returns -1 if not found.
static int find_entry_index(NarrativeCache* cache, const char* signature) {
    for (int i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].signature, signature) == 0) {
            return i;
        }
    }
    return -1;
}
// }}}

// {{{ is_entry_expired
// Checks if an entry has expired based on TTL.
static bool is_entry_expired(NarrativeCache* cache, NarrativeCacheEntry* entry) {
    if (cache->ttl_seconds <= 0) {
        return false;  // No expiration
    }

    time_t now = time(NULL);
    return (now - entry->generated_at) >= cache->ttl_seconds;
}
// }}}

// {{{ find_lru_index
// Finds the least recently used entry index.
static int find_lru_index(NarrativeCache* cache) {
    if (cache->count == 0) {
        return -1;
    }

    int lru_idx = 0;
    time_t oldest_time = cache->entries[0].last_used;

    for (int i = 1; i < cache->count; i++) {
        if (cache->entries[i].last_used < oldest_time) {
            oldest_time = cache->entries[i].last_used;
            lru_idx = i;
        }
    }

    return lru_idx;
}
// }}}

// {{{ remove_entry_at
// Removes entry at index and shifts remaining entries.
static void remove_entry_at(NarrativeCache* cache, int index) {
    if (index < 0 || index >= cache->count) {
        return;
    }

    // Free strings
    free(cache->entries[index].signature);
    free(cache->entries[index].narrative);

    // Shift remaining entries
    for (int i = index; i < cache->count - 1; i++) {
        cache->entries[i] = cache->entries[i + 1];
    }

    cache->count--;

    // Clear the now-unused slot
    cache->entries[cache->count].signature = NULL;
    cache->entries[cache->count].narrative = NULL;
}
// }}}

// {{{ narrative_cache_get
const char* narrative_cache_get(NarrativeCache* cache, const char* signature) {
    if (cache == NULL || signature == NULL) {
        return NULL;
    }

    int idx = find_entry_index(cache, signature);
    if (idx < 0) {
        cache->misses++;
        return NULL;
    }

    NarrativeCacheEntry* entry = &cache->entries[idx];

    // Check if expired
    if (is_entry_expired(cache, entry)) {
        remove_entry_at(cache, idx);
        cache->expirations++;
        cache->misses++;
        return NULL;
    }

    // Update access info
    entry->last_used = time(NULL);
    entry->use_count++;
    cache->hits++;

    return entry->narrative;
}
// }}}

// {{{ narrative_cache_set
bool narrative_cache_set(NarrativeCache* cache, const char* signature,
                          const char* narrative) {
    if (cache == NULL || signature == NULL || narrative == NULL) {
        return false;
    }

    // Check if entry already exists
    int existing_idx = find_entry_index(cache, signature);
    if (existing_idx >= 0) {
        // Update existing entry
        free(cache->entries[existing_idx].narrative);
        cache->entries[existing_idx].narrative = strdup(narrative);
        cache->entries[existing_idx].generated_at = time(NULL);
        cache->entries[existing_idx].last_used = time(NULL);
        return cache->entries[existing_idx].narrative != NULL;
    }

    // Need to add new entry - check if we need to evict
    if (cache->count >= cache->max_entries) {
        int lru_idx = find_lru_index(cache);
        if (lru_idx >= 0) {
            remove_entry_at(cache, lru_idx);
            cache->evictions++;
        }
    }

    // Add new entry
    NarrativeCacheEntry* entry = &cache->entries[cache->count];
    entry->signature = strdup(signature);
    entry->narrative = strdup(narrative);

    if (entry->signature == NULL || entry->narrative == NULL) {
        free(entry->signature);
        free(entry->narrative);
        entry->signature = NULL;
        entry->narrative = NULL;
        return false;
    }

    entry->generated_at = time(NULL);
    entry->last_used = time(NULL);
    entry->use_count = 0;

    cache->count++;
    return true;
}
// }}}

// {{{ narrative_cache_remove
bool narrative_cache_remove(NarrativeCache* cache, const char* signature) {
    if (cache == NULL || signature == NULL) {
        return false;
    }

    int idx = find_entry_index(cache, signature);
    if (idx < 0) {
        return false;
    }

    remove_entry_at(cache, idx);
    return true;
}
// }}}

// {{{ narrative_cache_clear
void narrative_cache_clear(NarrativeCache* cache) {
    if (cache == NULL) {
        return;
    }

    // Free all entries
    for (int i = 0; i < cache->count; i++) {
        free(cache->entries[i].signature);
        free(cache->entries[i].narrative);
        cache->entries[i].signature = NULL;
        cache->entries[i].narrative = NULL;
    }

    cache->count = 0;
}
// }}}

// {{{ narrative_cache_get_stats
NarrativeCacheStats narrative_cache_get_stats(NarrativeCache* cache) {
    NarrativeCacheStats stats = {0};

    if (cache == NULL) {
        return stats;
    }

    stats.total_entries = cache->count;
    stats.hits = cache->hits;
    stats.misses = cache->misses;
    stats.evictions = cache->evictions;
    stats.expirations = cache->expirations;

    int total_requests = cache->hits + cache->misses;
    if (total_requests > 0) {
        stats.hit_rate = (float)cache->hits / (float)total_requests;
    } else {
        stats.hit_rate = 0.0f;
    }

    return stats;
}
// }}}

// {{{ narrative_cache_reset_stats
void narrative_cache_reset_stats(NarrativeCache* cache) {
    if (cache == NULL) {
        return;
    }

    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    cache->expirations = 0;
}
// }}}

// {{{ event_build_signature
char* event_build_signature(NarrationEvent* event) {
    if (event == NULL) {
        return NULL;
    }

    char buffer[SIGNATURE_BUFFER_SIZE];
    const char* type_str = event_type_to_string(event->type);

    // Build signature based on event type and key attributes
    switch (event->type) {
        case GAME_EVENT_CARD_PLAYED:
            if (event->actor && event->card && event->card->type) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%s",
                         type_str,
                         event->card->type->name ? event->card->type->name : "unknown",
                         event->actor->name ? event->actor->name : "unknown");
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:unknown", type_str);
            }
            break;

        case GAME_EVENT_CARD_PURCHASED:
            if (event->actor && event->card && event->card->type) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%s:%d",
                         type_str,
                         event->card->type->name ? event->card->type->name : "unknown",
                         event->actor->name ? event->actor->name : "unknown",
                         event->cost);
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:unknown:%d",
                         type_str, event->cost);
            }
            break;

        case GAME_EVENT_ATTACK_PLAYER:
            if (event->actor && event->target) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%s:%d",
                         type_str,
                         event->actor->name ? event->actor->name : "unknown",
                         event->target->name ? event->target->name : "unknown",
                         event->damage);
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:unknown:%d",
                         type_str, event->damage);
            }
            break;

        case GAME_EVENT_ATTACK_BASE:
            if (event->actor && event->base && event->base->type) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%s:%d",
                         type_str,
                         event->actor->name ? event->actor->name : "unknown",
                         event->base->type->name ? event->base->type->name : "unknown",
                         event->damage);
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:unknown:%d",
                         type_str, event->damage);
            }
            break;

        case GAME_EVENT_BASE_DESTROYED:
            if (event->target && event->base && event->base->type) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%s",
                         type_str,
                         event->target->name ? event->target->name : "unknown",
                         event->base->type->name ? event->base->type->name : "unknown");
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:unknown", type_str);
            }
            break;

        case GAME_EVENT_TURN_START:
        case GAME_EVENT_TURN_END:
            if (event->actor) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%d",
                         type_str,
                         event->actor->name ? event->actor->name : "unknown",
                         event->turn);
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:%d",
                         type_str, event->turn);
            }
            break;

        case GAME_EVENT_GAME_OVER:
            if (event->actor && event->target) {
                snprintf(buffer, sizeof(buffer), "%s:%s:%s:%d",
                         type_str,
                         event->actor->name ? event->actor->name : "unknown",
                         event->target->name ? event->target->name : "unknown",
                         event->damage);  // final_authority stored in damage
            } else {
                snprintf(buffer, sizeof(buffer), "%s:unknown:unknown", type_str);
            }
            break;

        default:
            snprintf(buffer, sizeof(buffer), "%s", type_str);
            break;
    }

    return strdup(buffer);
}
// }}}

// {{{ narrative_cache_cleanup_expired
int narrative_cache_cleanup_expired(NarrativeCache* cache) {
    if (cache == NULL || cache->ttl_seconds <= 0) {
        return 0;
    }

    int removed = 0;

    // Iterate backwards to safely remove entries
    for (int i = cache->count - 1; i >= 0; i--) {
        if (is_entry_expired(cache, &cache->entries[i])) {
            remove_entry_at(cache, i);
            cache->expirations++;
            removed++;
        }
    }

    return removed;
}
// }}}
