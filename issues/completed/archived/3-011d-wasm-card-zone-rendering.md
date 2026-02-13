# 3-011d: WASM Card and Zone Rendering

## Current Behavior

Card and zone rendering handled by JavaScript (`card-renderer.js`, `zone-renderer.js`).
Cards display faction colors, effect badges, upgrade indicators, and selection states.

## Intended Behavior

All card and zone rendering implemented in C using the draw2d API. Maintains all
visual features from JS implementation.

## Dependencies

- 3-011a: Core Canvas Infrastructure (provides draw2d API)
- 3-011b: Theme and Layout Constants (provides colors and zone structure)
- 3-011c: Input Handling (provides hover/selection state)

## Suggested Implementation Steps

1. Create `src/client/wasm/card-renderer.h` with:
   - CardRenderState enum (normal, hover, selected, disabled, ally_active, empowered, targeted)
   - CardRenderData structure with all card properties

2. Create `src/client/wasm/card-renderer.c` implementing:
   - `card_render()` - Main card rendering with faction colors
   - `card_render_back()` - Face-down card
   - `card_render_empty_slot()` - Empty slot indicator
   - `render_effect_badge()` - Value badges (trade, combat, authority, draw)
   - `render_upgrade_badge()` - Upgrade bonus indicators

3. Create `src/client/wasm/zone-renderer.h` with:
   - Zone data structures (HandZoneData, TradeRowData, BasesZoneData, PlayAreaData)
   - Maximum card constants per zone

4. Create `src/client/wasm/zone-renderer.c` implementing:
   - `zone_render_hand()` - Player hand with selection/hover
   - `zone_render_trade_row()` - Trade row with deck pile
   - `zone_render_bases()` - Player/opponent bases with targeting
   - `zone_render_play_area()` - Cards in play
   - Card positioning helpers

## Files Created

- `src/client/wasm/card-renderer.h` - Card rendering API
- `src/client/wasm/card-renderer.c` - Card rendering implementation
- `src/client/wasm/zone-renderer.h` - Zone data structures
- `src/client/wasm/zone-renderer.c` - Zone rendering implementation

## JS Files Replaced

- `assets/web/card-renderer.js`
- `assets/web/zone-renderer.js`

## Acceptance Criteria

- [x] Cards render with correct faction colors
- [x] Effect badges display (trade, combat, authority, draw)
- [x] Upgrade badges display (+attack, +trade, +authority)
- [x] Selected/hover states render correctly (lifted position, border change)
- [x] Card back pattern renders
- [x] Empty slot with dashed border renders
- [x] Trade row shows deck pile with count
- [x] Bases render with overlap (stacked appearance)
- [x] Play area handles variable card counts

## Implementation Notes

Card rendering uses:
- Rounded rectangles for card body
- Faction color stripe at top
- Circular badges for values
- Different border styles for states
- Alpha for disabled state
