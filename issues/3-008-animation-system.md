# 3-008: Animation System

## Current Behavior
No animations in browser client.

## Intended Behavior
A client-side animation system that:
- Animates card plays (hand to played area)
- Animates purchases (trade row to discard)
- Animates attacks (visual effect)
- Respects user animation speed preference
- Can be disabled for accessibility

## Suggested Implementation Steps

1. Create `assets/web/animation.js`
2. Define animation types:
   ```javascript
   const animations = {
       PLAY_CARD: { duration: 300, easing: 'easeOut' },
       BUY_CARD: { duration: 400, easing: 'easeInOut' },
       ATTACK: { duration: 500, easing: 'linear' },
       DAMAGE: { duration: 200, easing: 'shake' }
   };
   ```
3. Implement animation queue to prevent overlap
4. Implement `animateCard(from, to, type)`:
   ```javascript
   function animateCard(card, from, to, type, callback) {
       const anim = animations[type];
       const start = performance.now();
       function frame(now) {
           const t = (now - start) / anim.duration;
           if (t >= 1) {
               card.x = to.x; card.y = to.y;
               callback();
               return;
           }
           const e = easing[anim.easing](t);
           card.x = lerp(from.x, to.x, e);
           card.y = lerp(from.y, to.y, e);
           requestAnimationFrame(frame);
       }
       requestAnimationFrame(frame);
   }
   ```
5. Apply speed multiplier from preferences
6. Add attack visual effect (flash, shake)
7. Add damage number popup
8. Write visual tests

## Related Documents
- 3-004-browser-canvas-renderer.md
- 3-006-client-style-preferences.md

## Dependencies
- 3-004: Browser Canvas Renderer
- 3-006: Client Style Preferences (speed setting)

## Animation Events

| Event | Animation |
|-------|-----------|
| Card played | Slide from hand to played area |
| Card bought | Slide from trade row to discard indicator |
| Attack player | Flash screen edge, shake opponent |
| Attack base | Target base flashes, damage number |
| Authority change | Number ticks up/down |
| Turn change | Brief overlay "Your Turn" / "Opponent's Turn" |

## Sub-Issues
This issue has been split into sub-issues for manageable implementation:
- 3-008a: Animation Core and Queue
- 3-008b: Card Movement Animations
- 3-008c: Attack and Damage Effects

## Acceptance Criteria
- [ ] Card play animates smoothly (3-008b)
- [ ] Purchase animates correctly (3-008b)
- [ ] Attack shows visual feedback (3-008c)
- [ ] Speed preference respected (3-008a)
- [ ] Animations can be disabled (3-008a)
- [ ] No animation queue backup/stutter (3-008a)
