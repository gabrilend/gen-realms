# 3-004c: Game Zone Rendering

## Parent Issue
3-004: Browser Canvas Renderer

## Current Behavior
No game zone rendering exists.

## Intended Behavior
Rendering for all game zones:
- Player's hand (bottom)
- Trade row (top right)
- Player's bases (left)
- Opponent's bases (top left, partial info)
- Played cards area (center)
- Discard pile indicator

## Suggested Implementation Steps

1. Implement `renderHand()`:
   ```javascript
   // {{{ renderHand
   function renderHand(ctx, hand, region) {
       const cardSpacing = 10;
       const totalWidth = hand.length * (CARD_WIDTH + cardSpacing) - cardSpacing;
       let startX = region.x + (region.w - totalWidth) / 2;

       hand.forEach((card, i) => {
           const x = startX + i * (CARD_WIDTH + cardSpacing);
           const y = region.y + 10;
           const hovered = hoveredCard === card;
           const selected = selectedCards.includes(card);

           // Hover effect: card lifts up
           const yOffset = hovered ? -20 : 0;

           renderCard(ctx, card, x, y + yOffset, { hovered, selected });

           // Store bounds for click detection
           card.bounds = { x, y: y + yOffset, w: CARD_WIDTH, h: CARD_HEIGHT };
       });
   }
   // }}}
   ```

2. Implement `renderTradeRow()`:
   ```javascript
   // {{{ renderTradeRow
   function renderTradeRow(ctx, tradeRow, region) {
       // Draw region background
       ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
       ctx.fillRect(region.x, region.y, region.w, region.h);

       // Draw header
       ctx.fillStyle = '#d4af37';
       ctx.font = 'bold 16px sans-serif';
       ctx.fillText('Trade Row', region.x + 10, region.y + 20);

       // Draw cards
       const cardSpacing = 10;
       tradeRow.forEach((card, i) => {
           const x = region.x + 10 + i * (CARD_WIDTH + cardSpacing);
           const y = region.y + 35;
           renderCard(ctx, card, x, y, { hovered: hoveredCard === card });
           card.bounds = { x, y, w: CARD_WIDTH, h: CARD_HEIGHT };
       });

       // Draw Wanderer slot
       renderWandererSlot(ctx, region);
   }
   // }}}
   ```

3. Implement `renderBases()`:
   ```javascript
   // {{{ renderBases
   function renderBases(ctx, bases, region, isOpponent) {
       ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
       ctx.fillRect(region.x, region.y, region.w, region.h);

       const label = isOpponent ? "Opponent's Bases" : "Your Bases";
       ctx.fillStyle = '#fff';
       ctx.font = 'bold 14px sans-serif';
       ctx.fillText(label, region.x + 10, region.y + 20);

       bases.forEach((base, i) => {
           const x = region.x + 10;
           const y = region.y + 30 + i * (CARD_HEIGHT * 0.6 + 10);
           renderCard(ctx, base, x, y, { hovered: hoveredCard === base });

           // Draw defense indicator
           renderDefenseIndicator(ctx, base, x + CARD_WIDTH + 5, y + 20);
       });
   }
   // }}}
   ```

4. Implement `renderPlayedCards()` for in-play area

5. Implement `renderDiscardIndicator()` showing discard pile size

6. Implement `renderDeckIndicator()` showing cards remaining

7. Store click bounds for all interactive elements

8. Write visual tests for each zone

## Related Documents
- 3-004a-canvas-infrastructure.md
- 3-004b-card-rendering.md

## Dependencies
- 3-004a: Canvas Infrastructure (layout regions)
- 3-004b: Card Rendering (renderCard function)

## Acceptance Criteria
- [ ] Hand renders centered at bottom
- [ ] Trade row shows available cards
- [ ] Bases render in correct positions
- [ ] Played cards visible in center
- [ ] Deck/discard indicators show counts
- [ ] Click bounds stored for interaction
