# 4-005: Frame Sequencing Logic

## Current Behavior
No ordering of generation operations.

## Intended Behavior
Logic to sequence inpainting for coherent scene building:
- Background first (sky, environment)
- Middle ground (main action)
- Foreground (details, effects)
- Respects dependencies (can't add detail to empty region)
- Creates smooth visual narrative

## Suggested Implementation Steps

1. Create `src/visual/sequencer.lua`
2. Define generation layers:
   ```lua
   local layers = {
     BACKGROUND = 1,  -- sky, distant mountains
     ENVIRONMENT = 2, -- trees, buildings
     MIDGROUND = 3,   -- main combatants
     FOREGROUND = 4,  -- effects, particles
     OVERLAY = 5      -- text, UI elements
   }
   ```
3. Map events to layers:
   - Game start -> BACKGROUND
   - Base played -> ENVIRONMENT
   - Ship played -> MIDGROUND
   - Attack -> FOREGROUND (effects)
4. Implement `Sequencer.queue_event(event)` - add to queue
5. Implement `Sequencer.get_next()` - return next generation task
6. Implement dependency checking (lower layers first)
7. Add frame timestamps for animation timing
8. Write tests for sequencing order

## Related Documents
- 4-002-inpainting-region-selection.md
- notes/vision (frame progression)

## Dependencies
- 4-002: Inpainting Region Selection

## Sequence Example

```
Turn 1: Game starts
  Frame 1: Generate BACKGROUND (empty battlefield, sky)
  Frame 2: Generate ENVIRONMENT (bases on each side)

Turn 3: Player plays Dire Bear
  Frame 3: Generate MIDGROUND (bear appears left side)

Turn 4: Attack for 5 damage
  Frame 4: Generate FOREGROUND (attack effects, motion)

Result: Coherent scene built up over 4 frames
```

## Acceptance Criteria
- [ ] Layers defined and respected
- [ ] Dependencies enforced
- [ ] Queue processes in correct order
- [ ] Frame timing recorded
- [ ] Scene builds coherently
