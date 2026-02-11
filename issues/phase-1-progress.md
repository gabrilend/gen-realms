# Phase 1 Progress: Core C Engine

## Goal
Build the foundational C game engine implementing deck-building card game mechanics with Symbeline-specific modifications. Engine compiles to native binary and WebAssembly.

## Status: In Progress (ALPHA checkpoint reached)

## Issues

| ID | Description | Status |
|----|-------------|--------|
| 1-001 | Card Data Structure | **completed** |
| 1-002 | Deck Management System | **completed** |
| 1-003 | Player State Management | **completed** |
| 1-004 | Trade Row Implementation | **completed** |
| 1-005 | Turn Loop Structure | **completed** |
| 1-006 | Basic Combat Resolution | **completed** |
| 1-007 | Card Effect System | in progress |
| 1-007a | ↳ Effect Dispatch Infrastructure | **completed** |
| 1-007b | ↳ Resource Effects | **completed** |
| 1-007c | ↳ Card Manipulation Effects | **completed** |
| 1-007d | ↳ Special Effects | **completed** |
| 1-007e | ↳ Upgrade and Spawn Effects | pending |
| 1-007f | ↳ Conditional and Ally Abilities | **completed** |
| 1-008 | Auto-Draw Resolution System | **completed** |
| 1-008a | ↳ Eligibility Detection | **completed** |
| 1-008b | ↳ Chain Resolution | **completed** |
| 1-008c | ↳ Spent Flag Management | **completed** |
| 1-008d | ↳ Event Emission | **completed** |
| 1-009 | Deck Flow Tracker (d10/d4) | **completed** |
| 1-010 | Base Card Type | **completed** |
| 1-011 | Spawning Mechanics | **completed** |
| 1-012 | Gamestate Serialization | **completed** |
| 1-013 | Phase 1 Demo | **completed** |

## Completed: 21/22

## Technology Stack
- C11 with cJSON for card definitions
- Emscripten for WebAssembly compilation
- Makefile with native and wasm targets

## Notes
Phase 1 establishes the core gameplay loop before any networking or rendering. The game logic must be platform-agnostic and serialize all state to JSON for client synchronization.

## Implementation Log

### 2026-02-10: ALPHA Checkpoint Reached
Completed foundational data structures (1-001, 1-002, 1-003):
- Card/Effect/CardInstance types with upgrade support
- Deck management with draw order choice mechanic
- Player state with d10/d4 deck flow tracker
- 75 unit tests passing
- Files: src/core/01-card.{h,c}, 02-deck.{h,c}, 03-player.{h,c}

### 2026-02-10: Core Game Loop Complete
Completed game mechanics (1-004, 1-005, 1-006):
- Trade row with 5 slots, explorer, DM hook for Phase 5
- Full game state with turn phases and player switching
- Combat system with outpost priority and base destruction
- 120 unit tests passing (45 new)
- Files: src/core/04-trade-row.{h,c}, 05-game.{h,c}, 06-combat.{h,c}

### 2026-02-10: Effect System Implemented
Completed effect dispatch (1-007a, 1-007b, 1-007f):
- Dispatch table with O(1) routing from EffectType to handler
- Resource effects (trade, combat, authority) with upgrade bonuses
- Ally ability system with faction tracking
- Event callback system for Phase 5 narrative hooks
- EffectContext for stateful effects (next_ship_free, etc.)
- 140 unit tests passing (20 new)
- Files: src/core/07-effects.{h,c}

### 2026-02-10: Deck Flow and Base Zones Complete
Completed 1-009 and 1-010:
- d10/d4 deck flow tracker integrated with trade/scrap operations
- Frontier/interior zone system replacing outpost concept
- Combat enforces zone priority (frontier > interior > player)
- Bases accumulate damage across multiple attacks
- Placement affects art generation via base_placement_art_modifier()
- 160 unit tests passing (20 new)
- Files: All core modules updated for new zone system

### 2026-02-10: Spawning Mechanics Complete
Completed 1-011:
- Card type registry with game_find_card_type() lookup
- Bases with spawns_id spawn units each turn after deployment
- Deployment delay (bases must survive one turn before becoming active)
- Spawn handler implemented in effect system
- Turn flow integrated: deploy -> process_base_effects -> draw order
- 170 unit tests passing (10 new)
- Files: 05-game.{h,c}, 07-effects.c

### 2026-02-11: Auto-Draw Resolution Complete
Completed 1-008 (all sub-issues 1-008a through 1-008d):
- Eligibility detection scans hand for cards with EFFECT_DRAW
- Spent flag management prevents re-triggering until shuffle
- Chain resolution handles nested auto-draws with 20-iteration safety limit
- Event emission for narrative hooks (START, TRIGGER, CARD, COMPLETE)
- Integrated into game turn flow after draw order selection
- 192 unit tests passing (22 new)
- Files: src/core/08-auto-draw.{h,c}, updated 05-game.c

### 2026-02-11: Phase 1 Demo Complete
Completed 1-013:
- CLI demonstration showcasing all Phase 1 mechanics
- Text display for player status, hand, bases, trade row
- Input handling for play, buy, attack, end turn commands
- Auto-draw event listener displays chain resolution
- Demo card set with 12 types across all factions
- Base zone selection (frontier/interior) on play
- Two-player hot-seat mode
- Files: src/demo/phase-1-demo.c, run-phase1-demo.sh

### 2026-02-11: Card Manipulation Effects Complete
Completed 1-007c:
- Pending action system for deferred player choices
- PendingAction struct with type, player_id, count, optional flag
- Pending queue in Game struct (max 8 actions)
- Resolution functions for discard, scrap, top-deck actions
- handle_discard creates PENDING_DISCARD for opponent
- handle_scrap_trade_row creates optional PENDING_SCRAP_TRADE_ROW
- handle_scrap_hand creates optional PENDING_SCRAP_HAND_DISCARD
- handle_top_deck creates optional PENDING_TOP_DECK
- game_skip_pending_action for optional actions
- 234 unit tests passing (42 new)
- Files: src/core/05-game.{h,c}, src/core/07-effects.c

### 2026-02-11: Special Effects Complete
Completed 1-007d:
- Copy ship creates PENDING_COPY_SHIP, player selects ship, executes target's effects
- Destroy base creates PENDING_DESTROY_BASE, player selects opponent's base, base is freed
- Acquire free sets EffectContext.next_ship_free with max cost limit
- Acquire top sets EffectContext.next_ship_to_top for deck placement
- game_buy_card/game_buy_explorer wrappers check effect context flags
- Flags automatically reset after purchase
- EFFECT_ACQUIRE_TOP added to EffectType enum
- 260 unit tests passing (26 new)
- Files: src/core/01-card.h, src/core/05-game.{h,c}, src/core/07-effects.c
