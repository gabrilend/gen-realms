# 4-007: Battle Scene Composition

## Current Behavior
No rules for element placement in canvas.

## Intended Behavior
Composition rules for placing elements in the battle scene:
- Player 1 forces on left, Player 2 on right
- Bases at bottom, ships in middle
- Action lines show conflict direction
- Visual balance across canvas
- Space for narrative moments

## Suggested Implementation Steps

1. Create `src/visual/composition.lua`
2. Define composition zones:
   ```lua
   local zones = {
     P1_BASE = {x = 0.0, y = 0.7, w = 0.3, h = 0.3},
     P1_FORCES = {x = 0.1, y = 0.3, w = 0.3, h = 0.4},
     CENTER = {x = 0.3, y = 0.2, w = 0.4, h = 0.6},
     P2_FORCES = {x = 0.6, y = 0.3, w = 0.3, h = 0.4},
     P2_BASE = {x = 0.7, y = 0.7, w = 0.3, h = 0.3},
     SKY = {x = 0.0, y = 0.0, w = 1.0, h = 0.2}
   }
   ```
3. Implement `Compose.get_zone(player, card_type)`
4. Implement `Compose.place_element(canvas, element, zone)`
5. Implement collision detection (don't overlap elements)
6. Add action zone for attacks (between forces)
7. Implement visual density tracking
8. Add composition templates for special moments
9. Write tests for placement logic

## Related Documents
- 4-002-inpainting-region-selection.md
- 4-005-frame-sequencing-logic.md

## Dependencies
- 4-001: Canvas State Manager
- 4-002: Inpainting Region Selection

## Composition Layout

```
+--------------------------------------------------+
|                      SKY                          |
+----------+------------------+--------------------+
|          |                  |                    |
| P1_FORCES|     CENTER       |    P2_FORCES       |
|          |   (conflict)     |                    |
|          |                  |                    |
+----------+------------------+--------------------+
| P1_BASE  |                  |       P2_BASE      |
+----------+------------------+--------------------+
```

## Acceptance Criteria
- [ ] Elements placed in correct zones
- [ ] Player sides consistent (left/right)
- [ ] No overlapping elements
- [ ] Attacks show in center/action zone
- [ ] Composition visually balanced
