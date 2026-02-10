# 6-009: Image Caching and Persistence

## Current Behavior
Generated images are not cached or saved.

## Intended Behavior
A caching system that:
- Caches generated images by prompt hash
- Persists canvas states to disk
- Supports game replay with cached images
- Exports final battle scenes
- Reduces redundant generation

## Suggested Implementation Steps

1. Create `src/visual/image-cache.h`:
   ```c
   // {{{ cache types
   typedef struct {
       char* prompt_hash;
       unsigned char* image_data;
       size_t image_size;
       time_t generated_at;
       int use_count;
   } ImageCacheEntry;

   typedef struct {
       ImageCacheEntry* entries;
       int entry_count;
       int max_entries;
       char* cache_dir;
       char* game_id;
   } ImageCache;
   // }}}
   ```

2. Implement `image_cache_init()`:
   ```c
   // {{{ init
   ImageCache* image_cache_init(const char* cache_dir, const char* game_id) {
       ImageCache* ic = malloc(sizeof(ImageCache));
       ic->cache_dir = strdup(cache_dir);
       ic->game_id = strdup(game_id);
       ic->max_entries = 256;
       ic->entries = calloc(ic->max_entries, sizeof(ImageCacheEntry));

       // Create game-specific directory
       char path[512];
       snprintf(path, sizeof(path), "%s/%s", cache_dir, game_id);
       mkdir(path, 0755);

       return ic;
   }
   // }}}
   ```

3. Implement `image_cache_get()`:
   ```c
   // {{{ get
   unsigned char* image_cache_get(ImageCache* ic, const char* prompt_hash,
                                   size_t* out_size) {
       for (int i = 0; i < ic->entry_count; i++) {
           if (strcmp(ic->entries[i].prompt_hash, prompt_hash) == 0) {
               ic->entries[i].use_count++;
               *out_size = ic->entries[i].image_size;
               return ic->entries[i].image_data;
           }
       }
       return NULL;  // Cache miss
   }
   // }}}
   ```

4. Implement `image_cache_set()`:
   ```c
   // {{{ set
   void image_cache_set(ImageCache* ic, const char* prompt_hash,
                        unsigned char* data, size_t size) {
       // Add to memory cache
       ImageCacheEntry* entry = &ic->entries[ic->entry_count++];
       entry->prompt_hash = strdup(prompt_hash);
       entry->image_data = malloc(size);
       memcpy(entry->image_data, data, size);
       entry->image_size = size;
       entry->generated_at = time(NULL);

       // Persist to disk
       image_cache_save_entry(ic, entry);
   }
   // }}}
   ```

5. Implement `image_cache_save_frame()` for canvas snapshots

6. Implement `image_cache_export_final()` for game completion

7. Implement `image_cache_load_game()` for replay support

8. Add LRU eviction when cache is full

9. Write tests for cache operations

## Related Documents
- 5-008-narrative-caching.md (similar pattern)
- 6-005-battle-canvas-manager.md

## Dependencies
- 6-005: Battle Canvas Manager
- stb_image_write (PNG export)

## Cache Directory Structure

```
output/images/
  game_20240115_143022/
    frame_001.png
    frame_002.png
    ...
    frame_final.png
    cache/
      abc123def.png  (by prompt hash)
      456789fed.png
    metadata.json
```

## Acceptance Criteria
- [ ] Images cached by prompt hash
- [ ] Cache hit avoids regeneration
- [ ] Canvas states persist to disk
- [ ] Final image exportable
- [ ] Replay loads cached images
