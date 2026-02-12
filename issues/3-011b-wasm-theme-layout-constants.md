# 3-011b: WASM Theme and Layout Constants

## Current Behavior

Theme colors and layout positioning defined in CSS (`style.css`) and JavaScript
constants. The WASM module has no awareness of visual styling.

## Intended Behavior

All theme constants (colors, spacing, fonts) and layout calculations implemented
in C headers. No external CSS file required.

## Dependencies

- 3-011a: Core Canvas Infrastructure (provides canvas/draw2d API)

## Suggested Implementation Steps

1. Create `src/client/wasm/theme.h` with:
   - RGB macro for color definitions
   - Faction colors (merchant, wilds, kingdom, artificer, neutral)
   - Value colors (authority, combat, trade, draw)
   - UI colors (background, border, text)
   - Font size constants
   - Spacing/padding constants
   - Zone layout structure

2. Create `src/client/wasm/theme.c` implementing:
   - `theme_calculate_layout()` - Responsive zone positioning
   - `theme_get_faction_color()` - Color lookup by faction name
   - `theme_get_value_color()` - Color lookup by value type

## Files Created

- `src/client/wasm/theme.h` - Theme constants and Layout struct
- `src/client/wasm/theme.c` - Layout calculation and color lookup

## CSS Replaced

- All of `style.css` color and spacing definitions

## Acceptance Criteria

- [x] All faction colors defined as RGB uint32_t
- [x] All value colors (authority, combat, trade, draw) defined
- [x] Zone layout structure with responsive calculation
- [x] Layout adapts to canvas size (small/normal card sizes)
- [x] No CSS file required for colors or spacing

## Implementation Notes

Created theme.h with:
- RGB macro: `#define RGB(r, g, b) ((r << 16) | (g << 8) | b)`
- Zone structure with x, y, w, h
- Layout structure containing all zones
- Card dimensions (normal and small)
- All spacing/padding constants

Created theme.c with:
- `theme_calculate_layout()` - Calculates all zone positions based on canvas size
- `theme_get_faction_color()` / `theme_get_faction_color_dark()`
- `theme_get_value_color()` - Returns color for value type
