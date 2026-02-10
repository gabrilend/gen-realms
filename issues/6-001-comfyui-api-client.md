# 4-001: Canvas State Manager

## Current Behavior
No visual canvas exists. Game has no image generation capability.

## Intended Behavior
A canvas manager that tracks the battle scene state:
- Maintains current canvas image (pixels)
- Tracks which regions have been filled
- Stores generation history
- Supports multiple canvas sizes
- Provides region querying

## Suggested Implementation Steps

1. Create `src/visual/` directory
2. Create `src/visual/canvas.lua`
3. Define canvas structure:
   ```lua
   local canvas = {
     width = 512,
     height = 512,
     image_data = nil,        -- actual pixel data
     region_mask = {},        -- which regions are filled
     history = {},            -- previous states
     generation_queue = {}    -- pending regions
   }
   ```
4. Implement `Canvas.new(width, height)` - create empty canvas
5. Implement `Canvas.get_region(x, y, w, h)` - extract region
6. Implement `Canvas.set_region(x, y, image)` - update region
7. Implement `Canvas.is_filled(x, y, w, h)` - check if region done
8. Implement `Canvas.save_state()` - add to history
9. Implement `Canvas.export(path)` - save to file
10. Write tests for region operations

## Related Documents
- notes/vision (progressive inpainting concept)
- docs/01-architecture-overview.md

## Dependencies
- None (standalone module)

## Canvas Regions (per vision)

```
.----------------.
| >- 0  = .  @ * |  <- Top region (sky, distant forces)
|  *  \  / \  *  |
| vv    o=.     jc|  <- Middle region (main battle)
| *-H    \\ < <  |
|  /   *  ))     |  <- Bottom region (ground, bases)
'----------------'
```

## Acceptance Criteria
- [ ] Canvas initializes with correct dimensions
- [ ] Regions can be queried and updated
- [ ] Fill state tracked per region
- [ ] History preserved for undo/export
- [ ] Canvas exports to image file
