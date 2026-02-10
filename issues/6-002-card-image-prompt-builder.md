# 4-002: Inpainting Region Selection

## Current Behavior
No system to determine what to generate next.

## Intended Behavior
Logic to select which canvas region to fill next:
- Based on game events (what just happened)
- Progressive reveal (center outward, or top-down)
- Narrative significance (big plays get more art)
- Efficient use of generation (batch similar regions)

## Suggested Implementation Steps

1. Create `src/visual/region-selector.lua`
2. Define region types:
   ```lua
   local regions = {
     SKY = {y = 0, h = 0.2},
     UPPER = {y = 0.2, h = 0.3},
     MIDDLE = {y = 0.5, h = 0.3},
     LOWER = {y = 0.8, h = 0.2}
   }
   ```
3. Map game events to regions:
   - Card played -> middle region (action)
   - Base placed -> lower region (fortification)
   - Attack -> varies by target
4. Implement `Region.select_next(canvas, event)` - pick region
5. Implement `Region.get_mask(region)` - return inpainting mask
6. Add priority queue for multiple pending events
7. Implement "narrative weight" scoring
8. Write tests for selection logic

## Related Documents
- 4-001-canvas-state-manager.md
- 3-007-event-hook-system.md

## Dependencies
- 4-001: Canvas State Manager

## Event to Region Mapping

```
Event                  Region      Priority
------------------------------------------
Game start            SKY          high (sets scene)
Ship played           MIDDLE       medium
Base placed           LOWER        medium
Attack                UPPER/MIDDLE high (action)
Authority low (<10)   FULL         high (desperation)
Victory               FULL         highest (finale)
```

## Acceptance Criteria
- [ ] Events map to appropriate regions
- [ ] Priority system works
- [ ] Unfilled regions selected first
- [ ] Masks generated correctly
- [ ] Queue handles multiple events
