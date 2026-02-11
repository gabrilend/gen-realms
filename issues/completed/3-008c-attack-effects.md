# 3-008c: Attack and Damage Effects

## Current Behavior
No visual feedback for attacks or damage.

## Intended Behavior
Visual effects for combat:
- Attack player shows screen flash
- Attack base shows base highlight and damage
- Damage numbers pop up and fade
- Authority changes tick visually
- Turn change announcement

## Suggested Implementation Steps

1. Implement `animateAttackPlayer()`:
   ```javascript
   // {{{ attack player
   function animateAttackPlayer(damage, targetSide, callback) {
       // Flash the screen edge on opponent side
       const overlay = document.createElement('div');
       overlay.className = 'attack-flash ' + targetSide;
       document.body.appendChild(overlay);

       const animation = {
           type: AnimationType.ATTACK,
           update: (progress) => {
               const opacity = Math.sin(progress * Math.PI);
               overlay.style.opacity = opacity * 0.3;
           },
           complete: () => {
               overlay.remove();
               showDamageNumber(damage, targetSide);
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

2. Implement `animateAttackBase()`:
   ```javascript
   // {{{ attack base
   function animateAttackBase(baseElement, damage, callback) {
       const originalBorder = baseElement.style.border;

       const animation = {
           type: AnimationType.DAMAGE,
           update: (progress) => {
               const shake = Math.sin(progress * Math.PI * 4) * 5;
               baseElement.style.transform = `translateX(${shake}px)`;
               baseElement.style.border = '2px solid red';
           },
           complete: () => {
               baseElement.style.transform = '';
               baseElement.style.border = originalBorder;
               showDamageNumber(damage, baseElement);
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

3. Implement `showDamageNumber()`:
   ```javascript
   // {{{ damage number
   function showDamageNumber(damage, target) {
       const num = document.createElement('div');
       num.className = 'damage-number';
       num.textContent = '-' + damage;

       // Position near target
       const rect = typeof target === 'string'
           ? getTargetRect(target)
           : target.getBoundingClientRect();

       num.style.left = rect.x + rect.width / 2 + 'px';
       num.style.top = rect.y + 'px';
       document.body.appendChild(num);

       // Animate upward and fade
       const startY = rect.y;
       const animation = {
           type: AnimationType.DAMAGE,
           update: (progress) => {
               num.style.top = (startY - progress * 50) + 'px';
               num.style.opacity = 1 - progress;
               num.style.transform = `scale(${1 + progress * 0.5})`;
           },
           complete: () => num.remove()
       };

       queueAnimation(animation);
   }
   // }}}
   ```

4. Implement `animateAuthorityChange()`:
   ```javascript
   // {{{ authority change
   function animateAuthorityChange(element, from, to, callback) {
       const diff = to - from;
       const color = diff > 0 ? 'green' : 'red';
       element.style.color = color;

       const animation = {
           type: AnimationType.AUTHORITY_CHANGE,
           update: (progress) => {
               const current = Math.round(from + diff * progress);
               element.textContent = current;
           },
           complete: () => {
               element.style.color = '';
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

5. Implement `animateTurnChange()`:
   ```javascript
   // {{{ turn change
   function animateTurnChange(isYourTurn, callback) {
       const overlay = document.createElement('div');
       overlay.className = 'turn-overlay';
       overlay.textContent = isYourTurn ? 'Your Turn' : "Opponent's Turn";
       document.body.appendChild(overlay);

       const animation = {
           type: AnimationType.TURN_CHANGE,
           update: (progress) => {
               overlay.style.opacity = progress < 0.5
                   ? progress * 2
                   : (1 - progress) * 2;
           },
           complete: () => {
               overlay.remove();
               callback && callback();
           }
       };

       queueAnimation(animation);
   }
   // }}}
   ```

6. Add CSS for damage-number and overlays

7. Write effect tests

## Related Documents
- 3-008a-animation-core.md
- 3-008b-card-animations.md
- 3-008-animation-system.md (parent issue)

## Dependencies
- 3-008a: Animation Core and Queue

## Effect Styles (CSS)

```css
/* {{{ effect styles */
.attack-flash {
    position: fixed;
    top: 0;
    bottom: 0;
    width: 100px;
    background: radial-gradient(ellipse, red, transparent);
    pointer-events: none;
}
.attack-flash.left { left: 0; }
.attack-flash.right { right: 0; }

.damage-number {
    position: fixed;
    font-size: 32px;
    font-weight: bold;
    color: red;
    text-shadow: 2px 2px black;
    pointer-events: none;
}

.turn-overlay {
    position: fixed;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    font-size: 48px;
    color: white;
    text-shadow: 3px 3px black;
    pointer-events: none;
}
/* }}} */
```

## Acceptance Criteria
- [ ] Attack player shows screen flash
- [ ] Attack base shakes target
- [ ] Damage numbers float up and fade
- [ ] Authority ticks between values
- [ ] Turn change shows announcement
