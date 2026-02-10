# 6-005: Battle Canvas Manager

## Current Behavior
No system to manage the battle scene image.

## Intended Behavior
A canvas manager that tracks the full battle scene:
- Maintains current battle image state
- Tracks which regions have been generated
- Supports progressive inpainting
- Stores canvas history for replay
- Exports final battle scenes

## Suggested Implementation Steps

1. Create `src/visual/canvas.h`:
   ```c
   // {{{ canvas types
   typedef struct {
       unsigned char* pixels;
       int width;
       int height;
       int channels;
   } Image;

   typedef struct {
       Image* current;
       Image** history;
       int history_count;
       int history_max;
       bool* region_filled;
       int region_count;
   } BattleCanvas;
   // }}}
   ```

2. Define canvas regions:
   ```c
   // {{{ regions
   typedef enum {
       REGION_SKY,
       REGION_P1_FORCES,
       REGION_CENTER,
       REGION_P2_FORCES,
       REGION_P1_BASE,
       REGION_P2_BASE,
       REGION_COUNT
   } CanvasRegion;

   typedef struct {
       int x, y, width, height;
       const char* name;
   } RegionBounds;
   // }}}
   ```

3. Implement `canvas_init()`:
   ```c
   // {{{ init
   BattleCanvas* canvas_init(int width, int height) {
       BattleCanvas* bc = malloc(sizeof(BattleCanvas));
       bc->current = image_create(width, height, 4);
       bc->history_max = 100;
       bc->history = calloc(bc->history_max, sizeof(Image*));
       bc->region_filled = calloc(REGION_COUNT, sizeof(bool));
       return bc;
   }
   // }}}
   ```

4. Implement `canvas_set_region()`:
   ```c
   // {{{ set region
   void canvas_set_region(BattleCanvas* bc, CanvasRegion region, Image* img) {
       RegionBounds bounds = get_region_bounds(region);
       image_blit(bc->current, img, bounds.x, bounds.y);
       bc->region_filled[region] = true;
       canvas_save_history(bc);
   }
   // }}}
   ```

5. Implement `canvas_get_mask()` for inpainting

6. Implement `canvas_save_history()` for undo/replay

7. Implement `canvas_export()` to save final image

8. Write tests for canvas operations

## Related Documents
- 6-006-inpainting-region-selection.md
- 6-007-scene-composition-rules.md

## Dependencies
- 6-001: ComfyUI API Client

## Canvas Layout (512x512)

```
+--------------------------------------------------+
|                      SKY (0-20%)                  |
+----------+------------------+--------------------+
|          |                  |                    |
| P1_FORCES|     CENTER       |    P2_FORCES      |
| (20-50%) |   (30-70%)       |    (50-80%)       |
|          |                  |                    |
+----------+------------------+--------------------+
| P1_BASE  |                  |       P2_BASE     |
| (70-100%)|                  |    (70-100%)      |
+----------+------------------+--------------------+
```

## Acceptance Criteria
- [ ] Canvas initializes correctly
- [ ] Regions can be updated individually
- [ ] Fill state tracked per region
- [ ] History preserved for replay
- [ ] Final image exports properly
