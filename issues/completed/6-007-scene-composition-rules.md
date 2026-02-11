# 6-007: Scene Composition Rules

## Current Behavior
No rules for placing elements in the battle canvas.

## Intended Behavior
Composition rules that govern element placement:
- Player 1 forces on left, Player 2 on right
- Bases at bottom of canvas
- Action occurs in center
- Visual balance maintained
- Depth cues for layering

## Suggested Implementation Steps

1. Create `src/visual/composition.h`:
   ```c
   // {{{ composition types
   typedef struct {
       float x, y;
       float width, height;
       int z_layer;
   } PlacementZone;

   typedef struct {
       PlacementZone* zones;
       int zone_count;
       int canvas_width;
       int canvas_height;
   } CompositionRules;
   // }}}
   ```

2. Define standard zones:
   ```c
   // {{{ zones
   static const PlacementZone STANDARD_ZONES[] = {
       // Player 1 side (left)
       {.x = 0.0f, .y = 0.7f, .width = 0.3f, .height = 0.3f, .z_layer = 1},  // P1 base
       {.x = 0.05f, .y = 0.25f, .width = 0.3f, .height = 0.45f, .z_layer = 2}, // P1 forces

       // Player 2 side (right)
       {.x = 0.7f, .y = 0.7f, .width = 0.3f, .height = 0.3f, .z_layer = 1},   // P2 base
       {.x = 0.65f, .y = 0.25f, .width = 0.3f, .height = 0.45f, .z_layer = 2}, // P2 forces

       // Shared zones
       {.x = 0.0f, .y = 0.0f, .width = 1.0f, .height = 0.25f, .z_layer = 0},  // Sky
       {.x = 0.3f, .y = 0.2f, .width = 0.4f, .height = 0.5f, .z_layer = 3}    // Center (action)
   };
   // }}}
   ```

3. Implement `composition_get_zone()`:
   ```c
   // {{{ get zone
   PlacementZone* composition_get_zone(CompositionRules* rules,
                                        int player, CardSubtype type) {
       // Map player + card type to appropriate zone
       // Return zone bounds for prompt construction
   }
   // }}}
   ```

4. Implement `composition_check_overlap()`:
   ```c
   // {{{ check overlap
   bool composition_check_overlap(CompositionRules* rules,
                                   PlacementZone* a, PlacementZone* b) {
       // Check if two elements would overlap
       // Return true if conflict exists
   }
   // }}}
   ```

5. Implement `composition_suggest_position()` for dynamic placement

6. Add density tracking to prevent overcrowding

7. Implement z-layer sorting for proper depth

8. Write tests for composition logic

## Related Documents
- 6-005-battle-canvas-manager.md
- 6-006-inpainting-region-selection.md

## Dependencies
- 6-005: Battle Canvas Manager
- 6-006: Inpainting Region Selection

## Composition Layout

```
+--------------------------------------------------+
|                      SKY (z=0)                    |
+----------+------------------+--------------------+
|          |                  |                    |
| P1_FORCES|     CENTER       |    P2_FORCES      |
|  (z=2)   |     (z=3)        |     (z=2)         |
|          |    [Action]      |                    |
+----------+------------------+--------------------+
| P1_BASE  |                  |       P2_BASE     |
|  (z=1)   |                  |        (z=1)      |
+----------+------------------+--------------------+
```

## Placement Rules

| Element | Zone | Z-Layer | Notes |
|---------|------|---------|-------|
| Base card | P1/P2_BASE | 1 | Bottom corners |
| Creature | P1/P2_FORCES | 2 | Mid-sides |
| Attack effect | CENTER | 3 | Foreground |
| Background | SKY | 0 | Always behind |

## Acceptance Criteria
- [x] Elements placed in correct zones
- [x] Player sides consistent (left/right)
- [x] No overlapping elements
- [x] Z-layers render correctly
- [x] Composition visually balanced

## Implementation Notes

Implemented as `assets/web/scene-composition.js`:

### SceneComposition Class
- Z-layer constants (BACKGROUND=0, BASE=1, FORCES=2, ACTION=3, OVERLAY=4)
- Zone definitions with normalized 0-1 coordinates
- Element tracking per zone with density management

### Key Functions
- `getZoneForPlayer(playerId, elementType)` - Maps player+type to zone
- `suggestPosition(zoneName, elementSize)` - Non-overlapping placement
- `_calculateOverlap(a, b)` - Rectangle intersection calculation
- `addElement()/removeElement()` - Element tracking
- `getSortedElements()` - Z-layer sorted for rendering
- `buildPromptForZone()` - Composition-aware prompt additions

### Zone Layout
```
+--------------------------------------------------+
|                      SKY (z=0)                    |
+----------+------------------+--------------------+
|          |                  |                    |
| P1_FORCES|     CENTER       |    P2_FORCES      |
|  (z=2)   |     (z=3)        |     (z=2)         |
+----------+------------------+--------------------+
| P1_BASE  |                  |       P2_BASE     |
|  (z=1)   |                  |        (z=1)      |
+----------+------------------+--------------------+
```

## Completion Date
2026-02-11
