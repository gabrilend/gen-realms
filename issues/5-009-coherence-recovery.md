# 3-009: LLM Response Caching

## Current Behavior
Every LLM request goes to the API.

## Intended Behavior
A caching system that reduces API calls:
- Cache responses for identical prompts
- Persist cache to disk for replay/debug
- Cache trade row selections by game state hash
- Cache narrative snippets by event signature
- Configurable cache TTL and size

## Suggested Implementation Steps

1. Create `src/llm/cache.lua`
2. Define cache structure:
   ```lua
   local cache = {
     memory = {},           -- in-memory cache
     disk_path = "tmp/llm-cache/",
     max_memory_entries = 100,
     ttl_seconds = 3600
   }
   ```
3. Implement `Cache.hash(prompt)` - generate cache key
4. Implement `Cache.get(key)` - retrieve cached response
5. Implement `Cache.set(key, response)` - store response
6. Implement `Cache.save_to_disk(key, response)` - persist
7. Implement `Cache.load_from_disk(key)` - restore
8. Add LRU eviction for memory cache
9. Integrate with LLM client (check cache before API)
10. Write tests for cache hit/miss scenarios

## Related Documents
- 3-001-llm-api-integration-module.md

## Dependencies
- 3-001: LLM API Integration

## Cache Flow

```
1. Generate prompt for "dire_bear played"
2. Hash prompt: "abc123"
3. Check memory cache: miss
4. Check disk cache: miss
5. Call LLM API
6. Receive: "The dire bear roars..."
7. Store in memory cache
8. Store on disk for replay

Later (same game state):
1. Generate identical prompt
2. Hash: "abc123"
3. Check memory cache: HIT
4. Return cached narrative (no API call)
```

## Acceptance Criteria
- [ ] Cache reduces duplicate API calls
- [ ] Disk persistence works
- [ ] LRU eviction prevents memory bloat
- [ ] Cache integrates with LLM client
- [ ] Replay uses cached responses
