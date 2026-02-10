# 6-004: Upgrade Visualization

## Current Behavior
Card upgrades have no visual representation.

## Intended Behavior
A system to visualize card upgrades and enhancements:
- Shows upgrade effects on card art
- Displays ally bonuses visually
- Animates scrap effects
- Indicates empowered states
- Supports overlay compositing

## Suggested Implementation Steps

1. Create `src/visual/upgrade-viz.h`:
   ```c
   // {{{ upgrade types
   typedef enum {
       UPGRADE_ALLY_ACTIVE,
       UPGRADE_EMPOWERED,
       UPGRADE_SCRAPPED,
       UPGRADE_TARGETED
   } UpgradeType;

   typedef struct {
       unsigned char* base_image;
       unsigned char* overlay;
       float intensity;
       int width;
       int height;
   } UpgradeVisualization;
   // }}}
   ```

2. Define upgrade overlays:
   ```c
   // {{{ overlays
   typedef struct {
       const char* glow_color;
       float glow_radius;
       const char* particle_type;
       float overlay_opacity;
   } UpgradeOverlay;

   static const UpgradeOverlay OVERLAYS[] = {
       [UPGRADE_ALLY_ACTIVE] = {"#FFD700", 10.0f, "sparkle", 0.3f},
       [UPGRADE_EMPOWERED] = {"#FF4444", 15.0f, "flame", 0.4f},
       [UPGRADE_SCRAPPED] = {"#888888", 5.0f, "dust", 0.6f},
       [UPGRADE_TARGETED] = {"#FF0000", 8.0f, "target", 0.5f}
   };
   // }}}
   ```

3. Implement `upgrade_apply_overlay()`:
   ```c
   // {{{ apply overlay
   void upgrade_apply_overlay(UpgradeVisualization* viz, UpgradeType type) {
       UpgradeOverlay* overlay = &OVERLAYS[type];
       // Apply glow effect
       // Add particle overlay
       // Composite onto base image
   }
   // }}}
   ```

4. Implement `upgrade_animate_transition()` for state changes

5. Implement `upgrade_generate_particles()` for effects

6. Add alpha blending for overlay compositing

7. Support multiple simultaneous upgrades

8. Write tests for visual composition

## Related Documents
- 6-002-card-image-prompt-builder.md
- 1-007-card-effect-system.md

## Dependencies
- 6-002: Card Image Prompt Builder
- 1-007: Card Effect System

## Upgrade Visual Effects

| Upgrade | Visual Effect |
|---------|---------------|
| Ally Bonus | Golden glow, faction sparkles |
| Empowered | Red aura, energy particles |
| Scrapped | Fade to gray, dust particles |
| Targeted | Red outline, crosshair overlay |
| Combo | Multi-colored energy trails |

## Acceptance Criteria
- [ ] Ally activation shows visually
- [ ] Empowered state has distinct look
- [ ] Scrap animation plays on removal
- [ ] Multiple upgrades can combine
- [ ] Effects render performantly
