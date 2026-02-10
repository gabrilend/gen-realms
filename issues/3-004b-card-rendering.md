# 3-004b: Card Rendering

## Parent Issue
3-004: Browser Canvas Renderer

## Current Behavior
No card rendering exists.

## Intended Behavior
Card rendering system that:
- Draws card frames with faction colors
- Displays card name and cost
- Shows placeholder art (until Phase 6)
- Renders effect icons and text
- Displays upgrade badges
- Handles card hover/selection states

## Suggested Implementation Steps

1. Define faction colors:
   ```javascript
   // {{{ faction colors
   const FACTION_COLORS = {
       merchant: { primary: '#d4af37', secondary: '#8b7355' },
       wilds: { primary: '#228b22', secondary: '#2e8b57' },
       kingdom: { primary: '#4169e1', secondary: '#191970' },
       artificer: { primary: '#9932cc', secondary: '#4b0082' },
       neutral: { primary: '#808080', secondary: '#505050' }
   };
   // }}}
   ```

2. Define card dimensions:
   ```javascript
   // {{{ card dimensions
   const CARD_WIDTH = 100;
   const CARD_HEIGHT = 140;
   const CARD_RADIUS = 8;
   // }}}
   ```

3. Implement `renderCard()`:
   ```javascript
   // {{{ renderCard
   function renderCard(ctx, card, x, y, options = {}) {
       const colors = FACTION_COLORS[card.faction] || FACTION_COLORS.neutral;
       const { selected, hovered, faceDown } = options;

       // Draw card frame
       ctx.fillStyle = colors.secondary;
       roundRect(ctx, x, y, CARD_WIDTH, CARD_HEIGHT, CARD_RADIUS);
       ctx.fill();

       // Draw border
       ctx.strokeStyle = selected ? '#fff' : colors.primary;
       ctx.lineWidth = selected ? 3 : 2;
       ctx.stroke();

       if (faceDown) {
           // Draw card back
           renderCardBack(ctx, x, y);
           return;
       }

       // Draw art placeholder
       ctx.fillStyle = '#333';
       ctx.fillRect(x + 5, y + 25, CARD_WIDTH - 10, 60);

       // Draw name
       ctx.fillStyle = '#fff';
       ctx.font = 'bold 12px sans-serif';
       ctx.textAlign = 'center';
       ctx.fillText(card.name, x + CARD_WIDTH/2, y + 18);

       // Draw cost
       renderCost(ctx, card.cost, x + CARD_WIDTH - 15, y + 10);

       // Draw effects
       renderEffects(ctx, card.effects, x + 5, y + 90);

       // Draw upgrade badges if present
       if (card.attackBonus || card.tradeBonus) {
           renderUpgradeBadges(ctx, card, x, y);
       }
   }
   // }}}
   ```

4. Implement `renderCost()` with gold coin icon

5. Implement `renderEffects()` with icons for trade/combat/authority

6. Implement `renderUpgradeBadges()` showing permanent bonuses

7. Implement `renderCardBack()` for face-down cards

8. Add hover glow effect

9. Write visual tests

## Related Documents
- 3-004a-canvas-infrastructure.md
- 1-001-card-data-structure.md

## Dependencies
- 3-004a: Canvas Infrastructure

## Acceptance Criteria
- [ ] Cards render with faction colors
- [ ] Name and cost displayed
- [ ] Placeholder art area shown
- [ ] Effects rendered with icons
- [ ] Upgrade badges visible
- [ ] Hover/selection states work
- [ ] Face-down cards show card back
