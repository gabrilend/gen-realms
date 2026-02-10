# 3-004d: Status and Narrative Panels

## Parent Issue
3-004: Browser Canvas Renderer

## Current Behavior
No status bar or narrative display exists.

## Intended Behavior
UI panels for game information:
- Status bar showing turn, authority, pools, d10/d4 tracker
- Narrative panel showing LLM-generated text
- Opponent info bar (authority, hand count)
- Action buttons/indicators

## Suggested Implementation Steps

1. Implement `renderStatusBar()`:
   ```javascript
   // {{{ renderStatusBar
   function renderStatusBar(ctx, gameState, region) {
       // Background
       ctx.fillStyle = 'rgba(0, 0, 0, 0.7)';
       ctx.fillRect(region.x, region.y, region.w, region.h);

       // Turn indicator
       ctx.fillStyle = '#fff';
       ctx.font = 'bold 18px sans-serif';
       ctx.fillText(`Turn ${gameState.turn}`, region.x + 10, region.y + 35);

       // Phase indicator
       ctx.fillStyle = '#aaa';
       ctx.font = '14px sans-serif';
       ctx.fillText(gameState.phase.toUpperCase(), region.x + 100, region.y + 35);

       // Authority
       renderAuthority(ctx, gameState.player.authority, region.x + 200, region.y + 10);

       // Trade/Combat pools
       renderPools(ctx, gameState.player, region.x + 350, region.y + 10);

       // d10/d4 tracker
       renderDeckTracker(ctx, gameState.player.deckTracker, region.x + 550, region.y + 10);

       // Opponent summary (right side)
       renderOpponentSummary(ctx, gameState.opponent, region.x + region.w - 250, region.y + 10);
   }
   // }}}
   ```

2. Implement `renderAuthority()` with heart icon:
   ```javascript
   // {{{ renderAuthority
   function renderAuthority(ctx, authority, x, y) {
       // Draw heart icon
       ctx.fillStyle = '#ff4444';
       drawHeart(ctx, x, y + 15, 12);

       // Draw value
       ctx.fillStyle = '#fff';
       ctx.font = 'bold 24px sans-serif';
       ctx.fillText(authority.toString(), x + 20, y + 30);
   }
   // }}}
   ```

3. Implement `renderPools()` showing trade/combat:
   ```javascript
   // {{{ renderPools
   function renderPools(ctx, player, x, y) {
       // Trade pool (gold)
       ctx.fillStyle = '#d4af37';
       ctx.font = 'bold 20px sans-serif';
       ctx.fillText(`${player.tradePool}`, x, y + 25);
       ctx.fillStyle = '#aaa';
       ctx.font = '12px sans-serif';
       ctx.fillText('Trade', x, y + 40);

       // Combat pool (red)
       ctx.fillStyle = '#ff6666';
       ctx.font = 'bold 20px sans-serif';
       ctx.fillText(`${player.combatPool}`, x + 60, y + 25);
       ctx.fillStyle = '#aaa';
       ctx.font = '12px sans-serif';
       ctx.fillText('Combat', x + 60, y + 40);
   }
   // }}}
   ```

4. Implement `renderDeckTracker()` showing d10/d4:
   ```javascript
   // {{{ renderDeckTracker
   function renderDeckTracker(ctx, tracker, x, y) {
       // d10 die
       ctx.fillStyle = '#4a90d9';
       ctx.fillText(`d10: ${tracker.d10}`, x, y + 25);

       // d4 die
       ctx.fillStyle = '#9b59b6';
       ctx.fillText(`d4: ${tracker.d4}`, x + 70, y + 25);
   }
   // }}}
   ```

5. Implement `renderNarrativePanel()`:
   ```javascript
   // {{{ renderNarrativePanel
   function renderNarrativePanel(ctx, narrativeHistory, region) {
       // Background with fantasy styling
       ctx.fillStyle = 'rgba(20, 15, 10, 0.9)';
       ctx.fillRect(region.x, region.y, region.w, region.h);

       // Border
       ctx.strokeStyle = '#8b7355';
       ctx.lineWidth = 2;
       ctx.strokeRect(region.x, region.y, region.w, region.h);

       // Title
       ctx.fillStyle = '#d4c4a8';
       ctx.font = 'italic 14px Georgia, serif';
       ctx.fillText('The Tale Unfolds...', region.x + 10, region.y + 20);

       // Render last N narrative entries
       ctx.font = '13px Georgia, serif';
       let y = region.y + 45;
       const visibleEntries = narrativeHistory.slice(-5);
       visibleEntries.forEach(entry => {
           const lines = wrapText(ctx, entry.text, region.w - 20);
           lines.forEach(line => {
               ctx.fillText(line, region.x + 10, y);
               y += 18;
           });
           y += 10; // gap between entries
       });
   }
   // }}}
   ```

6. Implement `wrapText()` helper for word wrapping

7. Implement scroll indicator for long narrative

8. Write visual tests

## Related Documents
- 3-004a-canvas-infrastructure.md
- 1-009-deck-flow-tracker.md
- 3-009-narrative-display.md

## Dependencies
- 3-004a: Canvas Infrastructure (layout regions)

## Acceptance Criteria
- [ ] Status bar shows all player info
- [ ] Authority displayed with icon
- [ ] Trade/combat pools visible
- [ ] d10/d4 tracker displayed
- [ ] Opponent info shown (limited)
- [ ] Narrative panel renders with styling
- [ ] Text wraps correctly in narrative
