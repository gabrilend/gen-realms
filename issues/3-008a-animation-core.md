# 3-008a: Animation Core and Queue

## Current Behavior
No animation system exists.

## Intended Behavior
Core animation infrastructure that:
- Defines animation types and parameters
- Manages animation queue to prevent overlap
- Provides easing functions
- Integrates with requestAnimationFrame
- Respects speed preference

## Suggested Implementation Steps

1. Create `assets/web/animation.js`:
   ```javascript
   // {{{ animation types
   const AnimationType = {
       PLAY_CARD: 'play_card',
       BUY_CARD: 'buy_card',
       ATTACK: 'attack',
       DAMAGE: 'damage',
       AUTHORITY_CHANGE: 'authority_change',
       TURN_CHANGE: 'turn_change'
   };

   const animationConfig = {
       [AnimationType.PLAY_CARD]: { duration: 300, easing: 'easeOut' },
       [AnimationType.BUY_CARD]: { duration: 400, easing: 'easeInOut' },
       [AnimationType.ATTACK]: { duration: 500, easing: 'linear' },
       [AnimationType.DAMAGE]: { duration: 200, easing: 'shake' },
       [AnimationType.AUTHORITY_CHANGE]: { duration: 400, easing: 'easeOut' },
       [AnimationType.TURN_CHANGE]: { duration: 800, easing: 'fadeInOut' }
   };
   // }}}
   ```

2. Define easing functions:
   ```javascript
   // {{{ easing
   const easings = {
       linear: t => t,
       easeIn: t => t * t,
       easeOut: t => t * (2 - t),
       easeInOut: t => t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t,
       shake: t => Math.sin(t * Math.PI * 4) * (1 - t),
       fadeInOut: t => t < 0.5 ? 2 * t : 2 * (1 - t)
   };
   // }}}
   ```

3. Create animation queue:
   ```javascript
   // {{{ queue
   const animationQueue = [];
   let isAnimating = false;
   let animationSpeed = 1.0;

   function queueAnimation(animation) {
       animationQueue.push(animation);
       if (!isAnimating) {
           processQueue();
       }
   }

   function processQueue() {
       if (animationQueue.length === 0) {
           isAnimating = false;
           return;
       }
       isAnimating = true;
       const anim = animationQueue.shift();
       runAnimation(anim, processQueue);
   }
   // }}}
   ```

4. Implement core animation runner:
   ```javascript
   // {{{ run animation
   function runAnimation(animation, callback) {
       const config = animationConfig[animation.type];
       const duration = config.duration / animationSpeed;
       const easing = easings[config.easing];
       const start = performance.now();

       function frame(now) {
           const elapsed = now - start;
           const t = Math.min(elapsed / duration, 1);
           const progress = easing(t);

           animation.update(progress);

           if (t < 1) {
               requestAnimationFrame(frame);
           } else {
               animation.complete && animation.complete();
               callback();
           }
       }

       requestAnimationFrame(frame);
   }
   // }}}
   ```

5. Implement `setAnimationSpeed()` from preferences

6. Add `skipAllAnimations()` for accessibility

7. Write queue tests

## Related Documents
- 3-008-animation-system.md (parent issue)
- 3-006a-preferences-storage.md (speed setting)

## Dependencies
- 3-004: Browser Canvas Renderer
- Browser requestAnimationFrame API

## Easing Functions

| Name | Effect | Use Case |
|------|--------|----------|
| linear | Constant speed | Movement |
| easeOut | Fast start, slow end | Card landing |
| easeInOut | Slow-fast-slow | Smooth transitions |
| shake | Oscillating | Damage feedback |
| fadeInOut | Peak in middle | Overlays |

## Acceptance Criteria
- [ ] Animation types defined with durations
- [ ] Queue prevents animation overlap
- [ ] Easing functions work correctly
- [ ] Speed preference scales duration
- [ ] Skip option for accessibility
