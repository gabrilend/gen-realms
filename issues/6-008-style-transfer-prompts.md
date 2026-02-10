# 4-008: Progressive Reveal Animation

## Current Behavior
No animation or reveal effect for generated images.

## Intended Behavior
Animated reveal as canvas regions are generated:
- Fade in newly generated regions
- Highlight active generation area
- Show progress indicator
- Support frame export for video
- Real-time display updates

## Suggested Implementation Steps

1. Create `src/visual/animation.lua`
2. Define animation states:
   ```lua
   local animation = {
     current_frame = nil,
     pending_regions = {},
     reveal_progress = {},  -- region -> 0.0-1.0
     frame_rate = 30,
     reveal_duration = 1.0  -- seconds
   }
   ```
3. Implement `Anim.start_reveal(region)` - begin fade in
4. Implement `Anim.update(dt)` - advance animation state
5. Implement `Anim.render()` - compose current frame
6. Implement `Anim.is_complete()` - check if reveals done
7. Add loading indicator for pending generation
8. Add frame buffer for smooth playback
9. Implement frame export to PNG sequence
10. Write tests for animation timing

## Related Documents
- notes/vision (frame-by-frame reveal)
- 4-001-canvas-state-manager.md

## Dependencies
- 4-001: Canvas State Manager
- 4-005: Frame Sequencing Logic

## Animation Flow

```
1. Region selected for generation
2. Placeholder/loading shown in region
3. API returns generated image
4. Fade-in animation starts (1 second)
5. Region blends from 0% to 100% opacity
6. Animation complete, move to next region

Frame export:
frame_001.png (empty canvas)
frame_002.png (sky 25% revealed)
frame_003.png (sky 50% revealed)
frame_004.png (sky 100%, base starting)
...
```

## Acceptance Criteria
- [ ] New regions fade in smoothly
- [ ] Loading state visible during generation
- [ ] Animation timing configurable
- [ ] Frames exportable for video
- [ ] Display updates in real-time
