# 3-011e: WASM Panel Rendering

## Current Behavior

Status bar and narrative panel rendered by JavaScript (`panel-renderer.js`,
`narrative.js`). Status shows player stats, turn info, action buttons.
Narrative displays LLM-generated story text with scrolling.

## Intended Behavior

All panel rendering implemented in C. Status bar and narrative panel render
entirely within canvas, no DOM elements.

## Dependencies

- 3-011a: Core Canvas Infrastructure (provides draw2d API)
- 3-011b: Theme and Layout Constants (provides colors and layout)

## Suggested Implementation Steps

1. Create `src/client/wasm/panel-renderer.h` with:
   - PlayerStats structure (name, authority, trade, combat, deck counts)
   - TurnInfo structure (turn number, phase, player turn flag)
   - ActionButton structure and enum
   - StatusBarData combining player, opponent, turn, buttons
   - NarrativeLine structure (text, color, style flags)
   - NarrativeData with lines array, scroll offset, streaming state

2. Create `src/client/wasm/panel-renderer.c` implementing:
   - `panel_render_status_bar()` - Full status bar
   - `panel_render_player_stats()` - Single player stat block
   - `panel_render_action_buttons()` - Centered button row
   - `panel_render_narrative()` - Narrative panel with scroll
   - `render_stat_value()` - Icon + value display
   - `render_button()` - Single button with states
   - `panel_hit_test_button()` - Button click detection
   - `panel_handle_narrative_scroll()` - Scroll handling

## Files Created

- `src/client/wasm/panel-renderer.h` - Panel data structures and API
- `src/client/wasm/panel-renderer.c` - Panel rendering implementation

## JS Files Replaced

- `assets/web/panel-renderer.js`
- `assets/web/narrative.js`

## Acceptance Criteria

- [x] Status bar shows player and opponent stats
- [x] Turn number and phase displayed
- [x] Action buttons render with enabled/disabled/hover states
- [x] Button hotkeys displayed
- [x] Narrative panel shows text lines
- [x] Scroll indicators when content exceeds visible area
- [x] Streaming text indicator for LLM responses
- [x] Different styles for headings, actions, regular text

## Implementation Notes

Status bar layout:
- Player stats on left (name, authority, trade, combat, deck info)
- Opponent stats on right (mirrored alignment)
- Turn info in center top
- Action buttons in center bottom

Narrative panel:
- Header with title
- Scrollable content area
- Lines rendered with appropriate colors/styles
- Streaming partial line with cursor indicator
