/*
 * 07-narrative-cache.h - Narrative Caching System
 *
 * Caches generated narratives by event signature to avoid
 * redundant LLM calls. Supports TTL expiration and LRU eviction.
 */

#ifndef LLM_NARRATIVE_CACHE_H
#define LLM_NARRATIVE_CACHE_H

#include <stdbool.h>
#include <time.h>
#include "05-event-narration.h"

// {{{ NarrativeCacheEntry
// A single cached narrative entry.
typedef struct {
    char* signature;      // Event signature (e.g., "card_played:dire_bear:player1")
    char* narrative;      // The cached narrative text
    time_t generated_at;  // When this narrative was generated
    time_t last_used;     // When this entry was last accessed
    int use_count;        // Number of cache hits
} NarrativeCacheEntry;
// }}}

// {{{ NarrativeCacheStats
// Statistics about cache performance.
typedef struct {
    int total_entries;
    int hits;
    int misses;
    int evictions;
    int expirations;
    float hit_rate;       // hits / (hits + misses)
} NarrativeCacheStats;
// }}}

// {{{ NarrativeCache
// The narrative cache manager.
typedef struct {
    NarrativeCacheEntry* entries;
    int count;
    int max_entries;
    int ttl_seconds;      // Time-to-live for entries (0 = no expiration)
    int hits;
    int misses;
    int evictions;
    int expirations;
} NarrativeCache;
// }}}

// {{{ narrative_cache_init
// Creates a new narrative cache.
// max_entries: Maximum number of cached narratives.
// ttl_seconds: Time-to-live in seconds (0 = no expiration).
// Returns NULL on allocation failure.
NarrativeCache* narrative_cache_init(int max_entries, int ttl_seconds);
// }}}

// {{{ narrative_cache_free
// Frees all resources used by the cache.
void narrative_cache_free(NarrativeCache* cache);
// }}}

// {{{ narrative_cache_get
// Retrieves a cached narrative by signature.
// Returns the narrative string (do NOT free), or NULL if not found/expired.
const char* narrative_cache_get(NarrativeCache* cache, const char* signature);
// }}}

// {{{ narrative_cache_set
// Stores a narrative in the cache.
// If the cache is full, evicts the least recently used entry.
// Returns true on success, false on failure.
bool narrative_cache_set(NarrativeCache* cache, const char* signature,
                          const char* narrative);
// }}}

// {{{ narrative_cache_remove
// Removes an entry from the cache by signature.
// Returns true if entry was found and removed.
bool narrative_cache_remove(NarrativeCache* cache, const char* signature);
// }}}

// {{{ narrative_cache_clear
// Removes all entries from the cache.
void narrative_cache_clear(NarrativeCache* cache);
// }}}

// {{{ narrative_cache_get_stats
// Gets current cache statistics.
NarrativeCacheStats narrative_cache_get_stats(NarrativeCache* cache);
// }}}

// {{{ narrative_cache_reset_stats
// Resets hit/miss/eviction counters.
void narrative_cache_reset_stats(NarrativeCache* cache);
// }}}

// {{{ event_build_signature
// Builds a unique signature string for a narration event.
// Caller must free the returned string.
// Returns NULL on allocation failure.
char* event_build_signature(NarrationEvent* event);
// }}}

// {{{ narrative_cache_cleanup_expired
// Removes all expired entries from the cache.
// Returns the number of entries removed.
int narrative_cache_cleanup_expired(NarrativeCache* cache);
// }}}

#endif /* LLM_NARRATIVE_CACHE_H */
