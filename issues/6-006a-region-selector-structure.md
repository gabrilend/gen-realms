# 6-006a: Region Selector Structure

## Current Behavior
No structure to manage region selection.

## Intended Behavior
Define the core region selector structure:
- Define region types and bounds
- Create selector data structure
- Initialize with canvas reference
- Track which regions are pending

## Suggested Implementation Steps

1. Create `src/visual/region-selector.h`:
   ```c
   // {{{ region types
   #ifndef REGION_SELECTOR_H
   #define REGION_SELECTOR_H

   #include "canvas.h"

   typedef enum {
       REGION_SKY,
       REGION_P1_FORCES,
       REGION_CENTER,
       REGION_P2_FORCES,
       REGION_P1_BASE,
       REGION_P2_BASE,
       REGION_COUNT
   } CanvasRegion;
   // }}}
   ```

2. Define region bounds:
   ```c
   // {{{ bounds
   typedef struct {
       int x, y;
       int width, height;
       const char* name;
       int layer;  // For z-ordering
   } RegionBounds;

   // For 512x512 canvas
   static const RegionBounds REGION_BOUNDS[] = {
       [REGION_SKY] = {0, 0, 512, 102, "sky", 0},
       [REGION_P1_FORCES] = {0, 102, 200, 230, "p1_forces", 2},
       [REGION_CENTER] = {150, 102, 212, 230, "center", 3},
       [REGION_P2_FORCES] = {312, 102, 200, 230, "p2_forces", 2},
       [REGION_P1_BASE] = {0, 358, 200, 154, "p1_base", 1},
       [REGION_P2_BASE] = {312, 358, 200, 154, "p2_base", 1}
   };
   // }}}
   ```

3. Define region task:
   ```c
   // {{{ task
   typedef struct {
       CanvasRegion region;
       int priority;
       GameEvent* trigger_event;
       char* prompt_context;
       time_t queued_at;
   } RegionTask;
   // }}}
   ```

4. Define selector structure:
   ```c
   // {{{ selector
   typedef struct {
       RegionTask* queue;
       int queue_size;
       int queue_capacity;
       BattleCanvas* canvas;
       bool* region_pending;  // Prevent duplicate tasks
   } RegionSelector;
   // }}}
   ```

5. Implement `selector_init()`:
   ```c
   // {{{ init
   RegionSelector* selector_init(BattleCanvas* canvas) {
       RegionSelector* rs = malloc(sizeof(RegionSelector));
       rs->canvas = canvas;
       rs->queue_capacity = 32;
       rs->queue_size = 0;
       rs->queue = calloc(rs->queue_capacity, sizeof(RegionTask));
       rs->region_pending = calloc(REGION_COUNT, sizeof(bool));
       return rs;
   }
   // }}}
   ```

6. Implement `selector_cleanup()`

7. Implement `region_get_bounds()` helper

8. Write structure tests

## Related Documents
- 6-006-inpainting-region-selection.md (parent issue)
- 6-005-battle-canvas-manager.md

## Dependencies
- 6-005: Battle Canvas Manager

## Region Layout

```
+--------------------------------------------------+
|                   SKY (layer 0)                   |
|                   0,0 - 512,102                   |
+----------+------------------+--------------------+
|          |                  |                    |
| P1_FORCES|     CENTER       |    P2_FORCES      |
| (layer 2)|    (layer 3)     |    (layer 2)      |
|          |                  |                    |
+----------+------------------+--------------------+
| P1_BASE  |                  |       P2_BASE     |
| (layer 1)|                  |      (layer 1)    |
+----------+------------------+--------------------+
```

## Acceptance Criteria
- [ ] Region enum and bounds defined
- [ ] Selector structure created
- [ ] Init allocates all resources
- [ ] Cleanup frees memory
- [ ] Region bounds accessible
