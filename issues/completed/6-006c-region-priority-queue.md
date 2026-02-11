# 6-006c: Region Priority Queue

## Current Behavior
No queue for managing region generation order.

## Intended Behavior
Priority queue for region generation tasks:
- Queue events in priority order
- Respect region dependencies (background first)
- Skip already-filled regions
- Generate masks for inpainting

## Suggested Implementation Steps

1. Implement `selector_queue_event()`:
   ```c
   // {{{ queue event
   bool selector_queue_event(RegionSelector* rs, GameEvent* event) {
       CanvasRegion region = map_event_to_region(event);

       // Skip if region already filled or pending
       if (rs->canvas->region_filled[region]) return false;
       if (rs->region_pending[region]) return false;

       // Check capacity
       if (rs->queue_size >= rs->queue_capacity) {
           // Could expand or evict lowest priority
           return false;
       }

       // Create task
       RegionTask task = {
           .region = region,
           .priority = get_event_priority(event),
           .trigger_event = event,
           .prompt_context = build_region_prompt(event, NULL),
           .queued_at = time(NULL)
       };

       // Insert in priority order (highest priority first)
       int insert_idx = rs->queue_size;
       for (int i = 0; i < rs->queue_size; i++) {
           if (task.priority > rs->queue[i].priority) {
               insert_idx = i;
               break;
           }
       }

       // Shift and insert
       for (int i = rs->queue_size; i > insert_idx; i--) {
           rs->queue[i] = rs->queue[i - 1];
       }
       rs->queue[insert_idx] = task;
       rs->queue_size++;
       rs->region_pending[region] = true;

       return true;
   }
   // }}}
   ```

2. Implement dependency checking:
   ```c
   // {{{ check dependencies
   typedef struct {
       CanvasRegion region;
       CanvasRegion depends_on;
   } RegionDependency;

   static const RegionDependency DEPENDENCIES[] = {
       {REGION_P1_FORCES, REGION_SKY},
       {REGION_P2_FORCES, REGION_SKY},
       {REGION_CENTER, REGION_SKY},
       {REGION_P1_BASE, REGION_SKY},
       {REGION_P2_BASE, REGION_SKY}
   };

   bool region_dependencies_met(RegionSelector* rs, CanvasRegion region) {
       for (int i = 0; i < ARRAY_SIZE(DEPENDENCIES); i++) {
           if (DEPENDENCIES[i].region == region) {
               if (!rs->canvas->region_filled[DEPENDENCIES[i].depends_on]) {
                   return false;
               }
           }
       }
       return true;
   }
   // }}}
   ```

3. Implement `selector_get_next()`:
   ```c
   // {{{ get next
   RegionTask* selector_get_next(RegionSelector* rs) {
       // Find highest priority task with met dependencies
       for (int i = 0; i < rs->queue_size; i++) {
           if (region_dependencies_met(rs, rs->queue[i].region)) {
               RegionTask* task = &rs->queue[i];

               // Remove from queue
               for (int j = i; j < rs->queue_size - 1; j++) {
                   rs->queue[j] = rs->queue[j + 1];
               }
               rs->queue_size--;

               return task;
           }
       }
       return NULL;  // No ready tasks
   }
   // }}}
   ```

4. Implement `selector_generate_mask()`:
   ```c
   // {{{ generate mask
   Image* selector_generate_mask(RegionSelector* rs, CanvasRegion region) {
       RegionBounds bounds = REGION_BOUNDS[region];

       // Create white mask for target region on black background
       Image* mask = image_create(rs->canvas->current->width,
                                   rs->canvas->current->height, 1);

       // Fill black
       memset(mask->pixels, 0, mask->width * mask->height);

       // Fill region white
       for (int y = bounds.y; y < bounds.y + bounds.height; y++) {
           for (int x = bounds.x; x < bounds.x + bounds.width; x++) {
               mask->pixels[y * mask->width + x] = 255;
           }
       }

       return mask;
   }
   // }}}
   ```

5. Implement `selector_mark_complete()` to clear pending flag

6. Add queue statistics (pending count, oldest task)

7. Write queue operation tests

## Related Documents
- 6-006a-region-selector-structure.md
- 6-006b-event-region-mapping.md
- 6-006-inpainting-region-selection.md (parent issue)

## Dependencies
- 6-006a: Region Selector Structure
- 6-006b: Event-to-Region Mapping

## Dependency Graph

```
        SKY (must generate first)
       / | \
      /  |  \
     v   v   v
P1_FORCES CENTER P2_FORCES
     |           |
     v           v
  P1_BASE    P2_BASE
```

## Queue Operations

| Operation | Description |
|-----------|-------------|
| queue_event | Add task in priority order |
| get_next | Pop highest ready task |
| mark_complete | Flag region as filled |
| generate_mask | Create inpainting mask |

## Acceptance Criteria
- [ ] Tasks queue in priority order
- [ ] Dependencies block premature generation
- [ ] get_next returns ready task
- [ ] Masks correctly mark target region
- [ ] Duplicate regions rejected
