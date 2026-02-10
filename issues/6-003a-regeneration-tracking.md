# 6-003a: Regeneration Tracking

## Parent Issue
6-003: Dynamic Art Regeneration

## Current Behavior
No tracking of which cards need art regeneration.

## Intended Behavior
System to track regeneration needs:
- needs_regen flag on CardInstance
- Set flag on upgrade, first draw, or explicit request
- Clear flag when new art generated
- Track image_seed for reproducibility

## Suggested Implementation Steps

1. Ensure CardInstance has regen fields (from 1-001):
   ```c
   // In CardInstance struct:
   typedef struct CardInstance {
       // ... other fields ...
       bool needs_regen;
       int image_seed;
   } CardInstance;
   ```

2. Create `assets/web/art-tracker.js`:
   ```javascript
   // {{{ ArtTracker class
   class ArtTracker {
       constructor() {
           this.pendingCards = new Map();  // card_id -> CardInstance
       }

       needsRegeneration(card) {
           return card.needs_regen === true;
       }

       markForRegeneration(card) {
           card.needs_regen = true;
           this.pendingCards.set(card.instance_id, card);
       }

       markComplete(card) {
           card.needs_regen = false;
           this.pendingCards.delete(card.instance_id);
       }

       getPendingCards() {
           return Array.from(this.pendingCards.values());
       }
   }
   // }}}
   ```

3. Set needs_regen on card events:
   ```javascript
   // {{{ trigger conditions
   // When card is upgraded
   function onCardUpgraded(card) {
       artTracker.markForRegeneration(card);
   }

   // When card drawn for first time this session
   function onCardDrawn(card) {
       if (!imageCache.has(card.instance_id)) {
           artTracker.markForRegeneration(card);
       }
   }

   // When user requests new art
   function onRegenerateRequested(card) {
       card.image_seed = Math.floor(Math.random() * 1000000);
       artTracker.markForRegeneration(card);
   }
   // }}}
   ```

4. Track image_seed for deterministic regeneration:
   ```javascript
   // {{{ seed management
   function getOrCreateSeed(card) {
       if (!card.image_seed) {
           card.image_seed = Math.floor(Math.random() * 1000000);
       }
       return card.image_seed;
   }
   // }}}
   ```

5. Include in gamestate protocol:
   ```json
   {
     "cards": [{
       "instance_id": "abc123",
       "needs_regen": true,
       "image_seed": 42
     }]
   }
   ```

6. Write tests for tracking states

## Related Documents
- 6-003-dynamic-art-regeneration.md (parent)
- 1-001-card-data-structure.md

## Dependencies
- 1-001: Card Data Structure (CardInstance fields)

## Acceptance Criteria
- [ ] needs_regen flag correctly tracked
- [ ] Flag set on upgrade
- [ ] Flag set on first draw
- [ ] Flag cleared after generation
- [ ] image_seed provides reproducibility
- [ ] Pending cards queryable
