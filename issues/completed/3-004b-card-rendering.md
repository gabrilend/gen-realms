# 3-004b: Card Rendering

## Parent Issue
3-004: Browser Canvas Renderer

## Current Behavior
Card rendering is implemented with faction colors, effects, and interaction states.

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
- [x] Cards render with faction colors
- [x] Name and cost displayed
- [x] Placeholder art area shown
- [x] Effects rendered with icons
- [x] Upgrade badges visible
- [x] Hover/selection states work
- [x] Face-down cards show card back

## Implementation Notes (2026-02-10)

### Files Created
- `assets/web/card-renderer.js` - Complete card rendering module

### Card Renderer API (window.cardRenderer)
```javascript
renderCard(ctx, card, x, y, options)  // Main render function
renderCost(ctx, cost, x, y, scale)    // Gold coin cost badge
renderEffects(ctx, effects, x, y, w, scale)  // Effect icons
renderUpgradeBadges(ctx, card, x, y, w, scale)  // +bonus indicators
renderDefense(ctx, defense, isOutpost, x, y, scale)  // Base defense
renderCardBack(ctx, x, y, w, h, r, scale)  // Face-down card
```

### Card Data Structure Expected
```javascript
{
    name: 'Card Name',
    cost: 3,
    faction: 'merchant' | 'wilds' | 'kingdom' | 'artificer' | 'neutral',
    kind: 'ship' | 'base' | 'unit',
    effects: [{ type: 'trade', value: 2 }, ...],
    allyEffects: [...],       // Optional
    defense: 5,               // For bases
    isOutpost: true,          // For bases
    attackBonus: 1,           // Upgrades
    tradeBonus: 1,            // Upgrades
    authorityBonus: 0         // Upgrades
}
```

### Visual Features
1. Rounded card frames with faction-specific colors
2. Gold coin badge for cost in top-right corner
3. Placeholder art area with faction symbol (⚖ ⚔ 👑 ⚙ ◆)
4. Effect summary with colored values (+T, +C, +A, D#)
5. Upgrade badges as colored circles with bonus values
6. Defense hexagon for outposts, circle for regular bases
7. Shadow glow on hover/selection
8. Face-down cards with decorative pattern

### Demo Mode Enhanced
Updated index.html demo mode to use card renderer:
- 5 sample trade row cards (one per faction)
- 5 hand cards with selection indicator
- Base card in player bases area
- Face-down card in opponent area
- Narrative text panel with sample story
