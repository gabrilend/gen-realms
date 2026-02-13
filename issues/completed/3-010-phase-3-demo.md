# 3-010: Phase 3 Demo

## Current Behavior
Phase 2 demo has networking but minimal UI.

## Intended Behavior
A polished demo showcasing both client types:
- Full ncurses terminal experience via SSH
- Full browser experience with canvas
- Both clients playing simultaneously
- All UI features working
- Visual feedback and narrative display

## Suggested Implementation Steps

1. Create `run-phase3-demo.sh` in project root
2. Update server to serve browser assets
3. Test complete flow:
   - Start server
   - Connect SSH client (see ncurses UI)
   - Open browser (see canvas UI)
   - Play full game with both
4. Verify all UI elements:
   - Hand display
   - Trade row
   - Bases
   - Narrative panel
   - Draw order selection
   - Animations (browser)
5. Test preferences save/load
6. Create demo video or screenshots

## Related Documents
- All Phase 3 issues
- 2-010-phase-2-demo.md

## Dependencies
- All previous Phase 3 issues (3-001 through 3-009)
- Phase 2 complete

## Demo Experience

### Terminal Client (SSH)
```
┌─────────────────────────────────────────────────────────────────┐
│ SYMBELINE REALMS                    Turn 12 | MAIN              │
│ You: Auth 42 d10:7 d4:1           Opp: Auth 28 Hand:4           │
├─────────────────────────────────┬───────────────────────────────┤
│ YOUR HAND                       │ TRADE ROW                     │
│ [0] Dire Bear +5C (Wilds)       │ [0] Knight Cmdr 4g    Kingdom │
│ [1] Guild Courier +2T (Merch)   │ [1] Battle Golem 3g   Artif.  │
│     +1 ATK upgrade              │ [2] Wolf Scout 1g     Wilds   │
│ [2] Village Scout +1T           │ [3] Sellsword 3g      Neutral │
│                                 │ [4] Trading Post 3g   Merch   │
│ Trade: 3  Combat: 5             │ [W] Wanderer 2g                │
├─────────────────────────────────┼───────────────────────────────┤
│ BASES (You)                     │ NARRATIVE                     │
│ [0] Trading Post 4def +2T/turn  │ The dire bear emerges from    │
│                                 │ the depths of Thornwood, its  │
│ BASES (Opponent)                │ roar echoing across fields... │
│ [0] Watchtower 3def (outpost)   │                               │
├─────────────────────────────────┴───────────────────────────────┤
│ > p 0                                                            │
└─────────────────────────────────────────────────────────────────┘
```

### Browser Client
- Canvas with card art placeholders
- Animated card movements
- Scrollable narrative panel
- Preferences panel accessible

## Acceptance Criteria
- [ ] SSH client renders full ncurses UI
- [ ] Browser client renders full canvas UI
- [ ] Both can play together
- [ ] Draw order works in both clients
- [ ] Narrative displays in both
- [ ] Animations work in browser
- [ ] Preferences persist in browser
- [ ] Complete game playable to victory
