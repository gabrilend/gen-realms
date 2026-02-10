# 5-008: Narrative Caching

## Current Behavior
No caching of generated narratives.

## Intended Behavior
A caching system that:
- Stores generated narratives by event signature
- Avoids regenerating identical narratives
- Expires stale cache entries
- Persists across game sessions (optional)

## Suggested Implementation Steps

1. Create cache structure:
   ```c
   // {{{ narrative cache
   typedef struct {
       char* event_signature;
       char* narrative;
       time_t generated_at;
       int use_count;
   } NarrativeCacheEntry;

   typedef struct {
       NarrativeCacheEntry* entries;
       int count;
       int max_entries;
       int ttl_seconds;
   } NarrativeCache;
   // }}}
   ```

2. Implement `narrative_cache_init()`:
   ```c
   // {{{ init
   NarrativeCache* narrative_cache_init(int max_entries, int ttl) {
       NarrativeCache* nc = malloc(sizeof(NarrativeCache));
       nc->max_entries = max_entries;
       nc->ttl_seconds = ttl;
       nc->entries = calloc(max_entries, sizeof(NarrativeCacheEntry));
       return nc;
   }
   // }}}
   ```

3. Implement event signature generation:
   ```c
   // {{{ event signature
   char* event_signature(GameEvent* event) {
       // Create unique string identifying this event type
       // e.g., "card_played:dire_bear:player1"
   }
   // }}}
   ```

4. Implement `narrative_cache_get()`:
   ```c
   // {{{ get
   char* narrative_cache_get(NarrativeCache* nc, const char* sig) {
       for (int i = 0; i < nc->count; i++) {
           if (strcmp(nc->entries[i].event_signature, sig) == 0) {
               // Check TTL
               if (time(NULL) - nc->entries[i].generated_at < nc->ttl_seconds) {
                   nc->entries[i].use_count++;
                   return nc->entries[i].narrative;
               }
           }
       }
       return NULL;
   }
   // }}}
   ```

5. Implement `narrative_cache_set()`

6. Implement LRU eviction when cache is full

7. Add cache statistics

8. Write tests

## Related Documents
- 5-005-event-narration-prompts.md
- 5-001-llm-api-client.md

## Dependencies
- 5-005: Event Narration Prompts

## Cache Strategy

| Event Type | Cacheable | TTL |
|------------|-----------|-----|
| Card played | Yes | Game duration |
| Attack | Partially | Short (varies by damage) |
| Turn start | No | - |
| Game over | Yes | Permanent |

## Acceptance Criteria
- [ ] Narratives cached by signature
- [ ] Cache hit avoids LLM call
- [ ] TTL expiration works
- [ ] LRU eviction works
- [ ] Cache statistics available
