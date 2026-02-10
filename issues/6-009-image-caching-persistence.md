# 4-009: Image Caching and Persistence

## Current Behavior
Generated images are not saved.

## Intended Behavior
Save and cache generated images for replay and export:
- Cache generated regions by prompt hash
- Persist full canvas states to disk
- Support game replay with cached images
- Export final battle scenes
- Reduce redundant generation

## Suggested Implementation Steps

1. Create `src/visual/image-cache.lua`
2. Define cache structure:
   ```lua
   local cache = {
     memory = {},
     disk_path = "output/images/",
     game_id = "",
     frame_index = 0
   }
   ```
3. Implement `ImageCache.new(game_id)` - create for game session
4. Implement `ImageCache.get(prompt_hash)` - check for cached region
5. Implement `ImageCache.set(prompt_hash, image)` - store region
6. Implement `ImageCache.save_frame(canvas)` - persist full canvas
7. Implement `ImageCache.export_final(path)` - save final image
8. Implement `ImageCache.load_game(game_id)` - restore for replay
9. Add cleanup for old cache entries
10. Write tests for cache operations

## Related Documents
- 3-009-llm-response-caching.md (similar pattern)
- 4-001-canvas-state-manager.md

## Dependencies
- 4-001: Canvas State Manager

## Cache Structure on Disk

```
output/images/
  game_20240115_143022/
    frame_001.png
    frame_002.png
    ...
    frame_final.png
    cache/
      abc123.png  (region by hash)
      def456.png
    metadata.json
```

## Acceptance Criteria
- [ ] Generated regions cached
- [ ] Canvas states persist to disk
- [ ] Replay loads cached images
- [ ] Final image exportable
- [ ] Cache reduces API calls
