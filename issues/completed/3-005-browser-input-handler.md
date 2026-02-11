# 3-005: Browser Input Handler

## Current Behavior
No browser input handling exists.

## Intended Behavior
An input handler for browser clients that:
- Handles click events on cards
- Translates clicks to game actions
- Sends actions via WebSocket
- Provides visual feedback on hover
- Shows available actions

## Suggested Implementation Steps

1. Extend `assets/web/game.js` with input handling
2. Track clickable regions from renderer
3. Implement click detection:
   ```javascript
   canvas.addEventListener('click', (e) => {
       const rect = canvas.getBoundingClientRect();
       const x = e.clientX - rect.left;
       const y = e.clientY - rect.top;
       handleClick(x, y);
   });
   ```
4. Map regions to actions:
   - Hand card click → play card
   - Trade row click → buy card
   - Attack button → attack
   - End turn button → end turn
5. Implement draw order selection UI
6. Add hover highlighting
7. Show action confirmation dialogs
8. Send action JSON via WebSocket
9. Handle action responses (error display)

## Related Documents
- docs/02-game-mechanics.md
- 3-004-browser-canvas-renderer.md

## Dependencies
- 3-004: Browser Canvas Renderer
- 2-003: WebSocket Handler (for sending)

## Action UI Elements

```javascript
const actionButtons = {
    attack: { x: 10, y: 500, w: 80, h: 30, label: "Attack" },
    endTurn: { x: 100, y: 500, w: 80, h: 30, label: "End Turn" }
};

function handleClick(x, y) {
    // Check hand cards
    for (const card of handCards) {
        if (pointInRect(x, y, card.bounds)) {
            sendAction({ type: "action", action: "play_card",
                         card_instance_id: card.instanceId });
            return;
        }
    }
    // Check trade row
    // Check buttons
    // ...
}
```

## Acceptance Criteria
- [ ] Card clicks register correctly
- [ ] Actions send via WebSocket
- [ ] Hover feedback visible
- [ ] Draw order UI works
- [ ] Error messages display
