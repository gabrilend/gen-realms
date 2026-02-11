# 1-013: Phase 1 Demo

## Current Behavior
No playable demonstration exists.

## Intended Behavior
A command-line demonstration that:
- Showcases all Phase 1 mechanics working together
- Allows two human players to play via stdin/stdout
- Displays game state clearly (hands, trade row, authority, d10/d4)
- Shows auto-draw resolution sequence
- Demonstrates base deployment and spawning
- Demonstrates draw order choice
- Runs until game completion (one player at 0 authority)
- Outputs game log for debugging
- Does NOT require networking (local single-process testing)

## Suggested Implementation Steps

1. Create `src/demo/phase-1-demo.c`
2. Create `run-phase1-demo.sh` in project root
3. Implement simple text display functions:
   - Display player status (authority, d10, d4, hand size)
   - Display hand with card details and upgrade status
   - Display trade row with costs
   - Display in-play bases
   - Display opponent's public info
4. Implement input handling via stdin:
   - `p N` - play card at index N
   - `b N` - buy card at slot N
   - `a` - attack player
   - `a N` - attack base at index N
   - `d 3,1,5,2,4` - set draw order
   - `e` - end turn
5. Create simple random AI opponent for single-player testing
6. Implement game loop with text refresh
7. Add game log output to file
8. Create sample card set JSON for demo (`assets/cards/demo/`)
9. Write usage instructions

## Related Documents
- All Phase 1 issues
- docs/02-game-mechanics.md
- docs/04-architecture-c-server.md

## Dependencies
- All previous Phase 1 issues (1-001 through 1-012)

## Demo Output Example

```
=== SYMBELINE REALMS - Phase 1 Demo ===
Turn 5 | Phase: MAIN | Active: Player 1

Player 1: Authority 42 | d10: 7 | d4: 0 | Trade: 3 | Combat: 5
Your Hand:
  [0] Dire Bear (+5C, Ally: Draw) - Wilds
  [1] Guild Courier (+2T, Ally: Draw) - Merchant
  [2] Village Scout (+1T) - Neutral

Your Bases:
  [0] Trading Post (Deployed, 4 def) - +2T each turn

Player 2: Authority 38 | Hand: 4 | Bases: 1

Trade Row:
  [0] Knight Commander (4g) - Kingdom
  [1] Battle Golem (3g) - Artificer
  [2] Wolf Scout (1g) - Wilds
  [3] Sellsword (3g) - Neutral
  [4] Trading Post (3g) - Merchant
  [W] Wandering Merchant (2g) - Always available

Commands: (p)lay (b)uy (a)ttack (e)nd turn
>
```

## Acceptance Criteria
- [x] Demo compiles and runs with `./run-phase1-demo.sh`
- [x] Complete game can be played to conclusion
- [x] All mechanics visible and functional
- [x] Auto-draw sequence displays correctly
- [x] Draw order choice works
- [x] Base deployment delay works
- [x] d10/d4 tracking visible
- [x] Card upgrades display correctly
- [ ] Game log captures all actions (not implemented - optional for basic demo)

## Implementation Notes (2026-02-11)

### Files Created:
- `src/demo/phase-1-demo.c` - Main demo source file (~600 lines)
- `run-phase1-demo.sh` - Script to build and run the demo

### Features Implemented:
1. **Text display functions** - Shows player status, hand, bases, trade row
2. **Input handling** - Commands: play (p), buy (b), explorer (e), attack (a), end turn
3. **Auto-draw listener** - Displays auto-draw events in real-time
4. **Demo card set** - 12 card types across all factions with various effects
5. **Base zone selection** - Player chooses frontier or interior when playing bases

### Commands:
- `p N` - Play card at index N from hand
- `b N` - Buy card at slot N from trade row
- `e` - Buy Explorer (always available)
- `a` - Attack opponent player
- `a N` - Attack base at index N
- `end` - End turn
- `q` - Quit game

### Build:
```
make demo
./bin/phase-1-demo
# OR
./run-phase1-demo.sh
```

### Notes:
- Two-player hot-seat mode (no AI opponent yet)
- Demonstrates all Phase 1 mechanics working together
- Auto-draw events displayed clearly during resolution
- Base deployment delay visible (shows "deploying" vs "active")
