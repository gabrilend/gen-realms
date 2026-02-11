# Track C: Client Rendering - Progress

This document tracks the progress of Track C (Client Rendering) from the parallelized
development roadmap.

## Track Overview

Track C implements terminal and browser client interfaces for Symbeline Realms.
It can be developed with mock gamestate data, with protocol integration after
the GAMMA checkpoint.

## Current Status: Gamma → Delta (Integration)

### Completed Issues

| Issue | Description | Date |
|-------|-------------|------|
| 3-003 | Wasm build configuration | 2026-02-10 |
| 3-001a | Terminal UI initialization | 2026-02-10 |
| 3-004a | Canvas infrastructure | 2026-02-10 |
| 3-001b | Terminal window rendering | 2026-02-10 |
| 3-004b | Card rendering (Canvas) | 2026-02-10 |
| 3-001c | Terminal formatting | 2026-02-10 |
| 3-004c | Game zones (Canvas) | 2026-02-10 |
| 3-001d | Terminal input/resize | 2026-02-10 |
| 3-004d | Status/narrative panels | 2026-02-10 |
| 3-002 | Terminal input system | 2026-02-11 |
| 3-005 | Browser input handler | 2026-02-11 |
| 3-006a | Preferences storage | 2026-02-11 |
| 3-008a | Animation core | 2026-02-11 |
| 3-009 | Narrative display | 2026-02-11 |

### In Progress

None currently.

### Pending (Alpha → Beta: Rendering Core)

All Alpha-Beta rendering core issues completed.

### Pending (Beta → Gamma: Input Systems)

All Beta-Gamma input system issues completed.

### Pending (Gamma → Delta: Integration)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 3-006b | Preferences UI panel | 3-006a |
| 3-006c | Preferences export/import | 3-006a |
| 3-007 | Draw order interface | 3-005, 2-005* |
| 3-008b | Card movement animations | 3-008a |
| 3-008c | Attack/damage effects | 3-008a |
| 3-010 | Phase 3 Demo | All above, 2-010 |

## Checkpoint Requirements

### For GAMMA Checkpoint
- All input systems working (3-002, 3-005)
- Can render mock gamestate data
- Ready for protocol integration

### For DELTA Checkpoint
- Full client integration with server
- Both terminal and browser clients functional
- Animation and narrative systems working

## Deliverables

Expected deliverables when Track C is complete:

- `src/client/terminal.c` - ncurses TUI client
- `src/client/wasm/` - Browser client (Canvas + WebSocket)
- `assets/web/` - Static HTML/CSS/JS
- `issues/completed/demos/phase-3-demo.sh` - Dual-client game

## Notes

### 2026-02-10: Initial Setup
- Created directory structure for client code
- Implemented Wasm build configuration with Emscripten
- Set up terminal UI with ncurses and faction colors
- Built canvas layout system with demo mode for testing without Wasm
- All Pre-Alpha starting issues (3-003, 3-001a, 3-004a) are now complete

### 2026-02-10: Alpha-Beta Rendering
- Implemented terminal window rendering (3-001b) with full Track A integration
  - Status bar, hand, trade row, bases, narrative windows
  - Faction color coding, effect formatting, scroll support
- Implemented canvas card rendering (3-004b)
  - Full card visuals with faction colors and effects
  - Cost badges, upgrade indicators, defense shields
  - Hover/selection states, face-down card backs
- Demo mode now shows realistic card rendering

### 2026-02-10: Rendering Core Complete
- Implemented terminal formatting (3-001c)
  - format_faction_tag() for short faction names
  - format_card_line() with effects and ally indicator
  - terminal_highlight_card() for selection states
  - format_upgrade_badge() for permanent bonuses (+, ++, ***)
  - terminal_supports_utf8() for box drawing fallback
- Implemented game zone rendering (3-004c)
  - zone-renderer.js with full game zone support
  - renderHand() with hover lift and centering
  - renderTradeRow() with affordability dimming
  - renderBases() for player and opponent
  - renderPlayedCards() with fan layout
  - renderDeckIndicator() and renderDiscardIndicator()
  - Click bounds tracking for all interactive elements
  - Demo mode now interactive (hover and click)

### 2026-02-10: Input Systems Foundation
- Implemented terminal input handling (3-001d)
  - 03-terminal-input.h/c with command parsing
  - Command types: play, buy, attack, scrap, activate, end, help, quit
  - Input history with up/down arrow navigation
  - terminal_read_command() with line editing
  - terminal_show_help() displays command reference
  - terminal_show_error() and terminal_show_message()
- Implemented status/narrative panels (3-004d)
  - panel-renderer.js with full panel support
  - renderStatusBar() with turn, phase, authority, pools, d10/d4
  - renderAuthority() with heart icon
  - renderPool() for trade/combat display
  - renderDeckTracker() for d10/d4
  - renderOpponentSummary() shows opponent stats
  - renderNarrativePanel() with word wrapping
  - renderActionButtons() for mobile/touch support
  - Demo now shows proper status bar and narrative panel

### 2026-02-11: Input Systems Complete
- Extended terminal input system (3-002)
  - Added draw order command parsing (d 3,1,5,2,4)
  - Added buy wanderer command (b w)
  - command_to_json() converts commands to protocol messages
  - command_valid_for_phase() validates commands per game phase
  - command_validation_error() provides helpful error messages
  - command_complete() for tab completion
  - terminal_show_help_for_phase() shows phase-appropriate help
  - terminal_show_completions() displays available commands
- Implemented browser input handler (3-005)
  - input-handler.js with full mouse/touch support
  - Click detection for all game zones
  - WebSocket action sending (with demo mode fallback)
  - Keyboard shortcuts (E=end turn, A=attack, Escape=cancel)
  - Draw order selection UI with visual feedback
  - Error message overlay with auto-dismiss
  - Action buttons rendering
  - Demo now fully interactive with game state updates

### 2026-02-11: Integration Systems
- Implemented preferences storage (3-006a)
  - preferences.js with localStorage backend
  - Version migration support for future upgrades
  - Validates and clamps values to valid ranges
  - Export/import for sharing preferences
  - Settings: styleGuide, animationSpeed, narrativeFont, accessibility
- Implemented animation core (3-008a)
  - animation.js with queued animation system
  - Easing functions: linear, easeIn/Out, shake, bounce, elastic
  - Animation types: play_card, buy_card, attack, damage, etc.
  - Speed control from preferences
  - Reduce motion accessibility support
  - Helper functions: interpolate, interpolatePoint, interpolateColor
- Implemented narrative display (3-009)
  - narrative.js with scrollable history
  - Entry types: narrative, action, attack, purchase, system, turn
  - Color-coded entries for visual distinction
  - Word wrapping for canvas rendering
  - Scroll up/down with auto-scroll on new entries
  - Copy-to-clipboard for sharing stories
  - Integration with demo mode

### Dependencies on Other Tracks
- Track A (1-001): Card struct needed for card rendering
- Track B (2-005): Protocol needed for draw order interface
- Track B (2-010): Phase 2 Demo needed for Phase 3 Demo
