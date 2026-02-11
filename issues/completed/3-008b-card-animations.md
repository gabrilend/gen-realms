# 3-008b: Card Movement Animations

## Current Behavior
Cards appear instantly without animation.

## Intended Behavior
Smooth card movement animations:
- Card plays animate from hand to played area
- Card purchases animate from trade row to discard indicator
- Draw animations show card appearing in hand
- Scrap animations show card leaving play

## Suggested Implementation Steps

1. Implement `animatePlayCard()`:
   ```javascript
   // {{{ play card
   function animatePlayCard(cardElement, fromRect, toRect, callback) {
       const animation = {
           type: AnimationType.PLAY_CARD,
           update: (progress) => {
               const x = lerp(fromRect.x, toRect.x, progress);
               const y = lerp(fromRect.y, toRect.y, progress);
               const scale = lerp(1.0, 0.8, progress * (1 - progress) * 4);

               cardElement.style.transform =
                   `translate(${x}px, ${y}px) scale(${scale})`;
               cardElement.style.zIndex = 1000;
           },
           complete: () => {
               cardElement.style.transform = '';
               cardElement.style.zIndex = '';
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

2. Implement `animateBuyCard()`:
   ```javascript
   // {{{ buy card
   function animateBuyCard(cardElement, fromRect, discardRect, callback) {
       // First phase: card lifts up
       // Second phase: card moves to discard
       const animation = {
           type: AnimationType.BUY_CARD,
           update: (progress) => {
               const x = lerp(fromRect.x, discardRect.x, progress);
               const y = lerp(fromRect.y, discardRect.y, progress);
               // Arc upward in middle
               const arc = Math.sin(progress * Math.PI) * -50;

               cardElement.style.transform =
                   `translate(${x}px, ${y + arc}px)`;
               cardElement.style.opacity = lerp(1, 0.5, progress);
           },
           complete: () => {
               cardElement.style.transform = '';
               cardElement.style.opacity = '';
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

3. Implement `animateDrawCard()`:
   ```javascript
   // {{{ draw card
   function animateDrawCard(cardElement, deckRect, handRect, callback) {
       cardElement.style.opacity = '0';
       cardElement.style.transform =
           `translate(${deckRect.x - handRect.x}px, ${deckRect.y - handRect.y}px)`;

       const animation = {
           type: AnimationType.PLAY_CARD, // Reuse timing
           update: (progress) => {
               const x = lerp(deckRect.x - handRect.x, 0, progress);
               const y = lerp(deckRect.y - handRect.y, 0, progress);
               cardElement.style.transform = `translate(${x}px, ${y}px)`;
               cardElement.style.opacity = progress;
           },
           complete: () => {
               cardElement.style.transform = '';
               cardElement.style.opacity = '';
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

4. Implement `animateScrapCard()` with fade and shrink

5. Helper function `lerp()`:
   ```javascript
   // {{{ lerp
   function lerp(a, b, t) {
       return a + (b - a) * t;
   }
   // }}}
   ```

6. Add card flip animation for reveals

7. Write visual movement tests

## Related Documents
- 3-008a-animation-core.md
- 3-008-animation-system.md (parent issue)

## Dependencies
- 3-008a: Animation Core and Queue
- 3-004: Browser Canvas Renderer (card elements)

## Animation Paths

| Animation | Path | Duration |
|-----------|------|----------|
| Play card | Hand → Played area | 300ms |
| Buy card | Trade row → Discard (arc) | 400ms |
| Draw card | Deck → Hand | 300ms |
| Scrap card | Play area → fade out | 250ms |

## Acceptance Criteria
- [ ] Play card animates smoothly
- [ ] Buy card follows arc path
- [ ] Draw card fades in from deck
- [ ] Scrap shows removal effect
- [ ] All respect animation speed setting
