# 5-008: Narrative Caching

## Status: COMPLETED

## Current Behavior
Narrative caching system stores generated narratives by event signature, avoids redundant LLM calls, supports TTL expiration and LRU eviction.

## Intended Behavior
A caching system that:
- Stores generated narratives by event signature
- Avoids regenerating identical narratives
- Expires stale cache entries
- Persists across game sessions (optional)

## Implementation Details

### Files Created
- `src/llm/07-narrative-cache.h` - Header with cache types and functions
- `src/llm/07-narrative-cache.c` - Cache implementation
- `tests/test-narrative-cache.c` - Test suite (20 tests)

### Key Components

1. **NarrativeCacheEntry**: Individual cache entries with:
   - signature: Event signature string
   - narrative: Cached narrative text
   - generated_at: Creation timestamp
   - last_used: Last access timestamp (for LRU)
   - use_count: Number of cache hits

2. **NarrativeCache**: Cache manager with:
   - Configurable max_entries
   - Configurable TTL (0 = no expiration)
   - Hit/miss/eviction/expiration counters

3. **Event Signatures**: Unique identifiers built from:
   - Event type
   - Actor/target names
   - Card/base names
   - Numeric values (damage, cost, turn)

### Functions Implemented
- `narrative_cache_init()` - Create cache with size and TTL
- `narrative_cache_free()` - Free all resources
- `narrative_cache_get()` - Get narrative (updates LRU)
- `narrative_cache_set()` - Store narrative (LRU eviction)
- `narrative_cache_remove()` - Remove specific entry
- `narrative_cache_clear()` - Clear all entries
- `narrative_cache_get_stats()` - Get hit/miss statistics
- `narrative_cache_reset_stats()` - Reset counters
- `narrative_cache_cleanup_expired()` - Remove expired entries
- `event_build_signature()` - Build event signature

### Test Results
All 20 tests passing:
- Initialization (3 tests)
- Basic operations (5 tests)
- Eviction (3 tests)
- Statistics (2 tests)
- TTL (2 tests)
- Event signatures (4 tests)
- Safety (1 test)

### Usage Example

```c
NarrativeCache* cache = narrative_cache_init(100, 3600);  // 100 entries, 1hr TTL

// Check cache before LLM call
NarrationEvent* event = event_narration_create(GAME_EVENT_CARD_PLAYED);
event->actor = player;
event->card = card;

char* sig = event_build_signature(event);
const char* cached = narrative_cache_get(cache, sig);

if (cached) {
    // Use cached narrative
    use_narrative(cached);
} else {
    // Generate new narrative via LLM
    char* prompt = event_narration_build(event);
    char* narrative = llm_request(prompt);

    narrative_cache_set(cache, sig, narrative);
    use_narrative(narrative);

    free(prompt);
    free(narrative);
}

free(sig);
event_narration_free(event);
```

## Related Documents
- 5-005-event-narration-prompts.md
- 5-001-llm-api-client.md

## Dependencies
- 5-005: Event Narration Prompts (completed)

## Cache Strategy

| Event Type | Cacheable | TTL |
|------------|-----------|-----|
| Card played | Yes | Game duration |
| Attack | Partially | Short (varies by damage) |
| Turn start | No | - |
| Game over | Yes | Permanent |

## Acceptance Criteria
- [x] Narratives cached by signature
- [x] Cache hit avoids LLM call
- [x] TTL expiration works
- [x] LRU eviction works
- [x] Cache statistics available
