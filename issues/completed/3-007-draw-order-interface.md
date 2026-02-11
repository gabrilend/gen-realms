# 3-007: Draw Order Interface

## Current Behavior
No UI for choosing draw order.

## Intended Behavior
A specialized UI for the draw order selection phase that:
- Shows card backs (positions in deck)
- Allows reordering via drag-and-drop or numbering
- Confirms selection before proceeding
- Works in both terminal and browser
- Explains the mechanic to new players

## Suggested Implementation Steps

1. **Terminal version** in `src/client/02-input.c`:
   - Show: "Draw order: You have 5 cards to draw"
   - Prompt: "Enter order (e.g., 3,1,5,2,4): "
   - Parse comma-separated indices
   - Validate indices

2. **Browser version** in `assets/web/draw-order.js`:
   - Show card backs arranged horizontally
   - Drag-and-drop to reorder
   - Number badges show selection order
   - "Confirm" button to submit
   ```javascript
   function showDrawOrderUI(cardCount) {
       // Create draggable card back elements
       // Track selection order
       // On confirm, send to server
   }
   ```

3. Implement `d 3,1,5,2,4` protocol message
4. Add tutorial tooltip for first-time users
5. Handle timeout (auto-select sequential if no input)

## Related Documents
- docs/02-game-mechanics.md (draw order choice)
- 3-002-terminal-input-system.md
- 3-005-browser-input-handler.md

## Dependencies
- 3-002: Terminal Input System
- 3-005: Browser Input Handler

## Browser UI Design

```
┌────────────────────────────────────────────────────┐
│         Choose Your Draw Order                      │
│                                                     │
│    ┌───┐   ┌───┐   ┌───┐   ┌───┐   ┌───┐          │
│    │ ? │   │ ? │   │ ? │   │ ? │   │ ? │          │
│    │   │   │   │   │   │   │   │   │   │          │
│    │   │   │   │   │   │   │   │   │   │          │
│    └───┘   └───┘   └───┘   └───┘   └───┘          │
│      1       2       3       4       5             │
│                                                     │
│    Click cards in the order you want to draw them  │
│                                                     │
│    Selected order: 3, 1, 5, 2, 4                   │
│                                                     │
│    [Confirm]  [Default Order]                       │
└────────────────────────────────────────────────────┘
```

## Acceptance Criteria
- [ ] Terminal input parses draw order
- [ ] Browser shows draggable cards
- [ ] Order transmitted correctly
- [ ] Invalid orders rejected with error
- [ ] Default available for quick play
