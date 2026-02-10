# 3-004: Browser Canvas Renderer

## Current Behavior
No browser rendering exists.

## Intended Behavior
An HTML5 Canvas renderer for browser clients that:
- Renders game state visually
- Shows cards with placeholder art (until Phase 6)
- Displays player info, trade row, bases
- Shows narrative text panel
- Supports responsive layout
- Provides click targets for interaction

## Suggested Implementation Steps

1. Create `assets/web/index.html` with canvas element
2. Create `assets/web/game.js` with rendering logic
3. Create `assets/web/style.css` for layout
4. Define render regions:
   ```javascript
   const layout = {
       hand: { x: 0, y: 300, w: 400, h: 200 },
       tradeRow: { x: 400, y: 0, w: 400, h: 200 },
       narrative: { x: 400, y: 300, w: 400, h: 200 },
       status: { x: 0, y: 0, w: 400, h: 50 },
       bases: { x: 0, y: 50, w: 400, h: 250 }
   };
   ```
5. Implement `renderGame(gamestate)` function
6. Implement `renderCard(ctx, card, x, y)` with placeholder art
7. Implement `renderNarrative(ctx, text)` with scroll
8. Add faction color coding
9. Handle canvas resize
10. Make cards clickable (store bounds)

## Related Documents
- docs/04-architecture-c-server.md
- 3-003-wasm-build-configuration.md

## Dependencies
- 3-003: Wasm Build Configuration
- Modern browser with Canvas support

## Canvas Layout

```
┌────────────────────────────────────────────────────────────────┐
│ STATUS: Turn 12 | Authority 42 | d10:7 | Trade:3 Combat:5     │
├────────────────────────────────┬───────────────────────────────┤
│                                │                               │
│     YOUR BASES                 │      TRADE ROW                │
│     ┌──────┐                   │      ┌──────┐ ┌──────┐        │
│     │ Base │                   │      │Card1 │ │Card2 │ ...    │
│     └──────┘                   │      └──────┘ └──────┘        │
│                                │                               │
├────────────────────────────────┼───────────────────────────────┤
│                                │                               │
│     YOUR HAND                  │      NARRATIVE                │
│     ┌──────┐ ┌──────┐ ┌──────┐ │      The dire bear emerges   │
│     │Card1 │ │Card2 │ │Card3 │ │      from the Thornwood...   │
│     └──────┘ └──────┘ └──────┘ │                               │
│                                │                               │
└────────────────────────────────┴───────────────────────────────┘
```

## Acceptance Criteria
- [ ] Canvas renders game state
- [ ] Cards display with faction colors
- [ ] Narrative panel shows text
- [ ] Layout adapts to window size
- [ ] Click regions defined for interaction
