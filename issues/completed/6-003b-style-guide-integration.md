# 6-003b: Style Guide Integration

## Parent Issue
6-003: Dynamic Art Regeneration

## Current Behavior
No style guide applied to image generation.

## Intended Behavior
Apply user style preferences to generation requests:
- Load style guide from localStorage
- Load negative prompts from localStorage
- Merge with card-specific prompts
- Support per-faction style overrides

## Suggested Implementation Steps

1. Load preferences from storage (uses 3-006):
   ```javascript
   // {{{ load preferences
   function loadStylePreferences() {
       const prefs = JSON.parse(localStorage.getItem('symbeline_prefs') || '{}');
       return {
           styleGuide: prefs.styleGuide || 'dark fantasy, oil painting, dramatic lighting',
           negativePrompts: prefs.negativePrompts || 'cartoon, anime, bright colors, modern',
           cardFrameStyle: prefs.cardFrameStyle || 'ornate-gold'
       };
   }
   // }}}
   ```

2. Create style merger:
   ```javascript
   // {{{ merge styles
   function mergeStyleWithCard(cardPrompt, stylePrefs) {
       return {
           positive: `${cardPrompt.positive}, ${stylePrefs.styleGuide}`,
           negative: `${cardPrompt.negative}, ${stylePrefs.negativePrompts}`,
           style: stylePrefs.cardFrameStyle
       };
   }
   // }}}
   ```

3. Support faction-specific styles:
   ```javascript
   // {{{ faction styles
   const FACTION_STYLES = {
       merchant: 'golden accents, wealthy, trading posts, coins',
       wilds: 'forest, primal, natural, beasts, green tones',
       kingdom: 'noble, castles, knights, blue and silver',
       artificer: 'mechanical, purple energy, constructs, arcane'
   };

   function getFactionStyle(faction) {
       return FACTION_STYLES[faction] || '';
   }
   // }}}
   ```

4. Build complete prompt:
   ```javascript
   // {{{ build complete prompt
   function buildGenerationPrompt(card, stylePrefs) {
       // Base card prompt
       let prompt = buildCardPrompt(card);

       // Add faction style
       prompt.positive += `, ${getFactionStyle(card.faction)}`;

       // Merge with user preferences
       prompt = mergeStyleWithCard(prompt, stylePrefs);

       // Add upgrade visualization if upgraded
       if (card.attack_bonus || card.trade_bonus) {
           prompt.positive += ', glowing runes, enhanced, magical aura';
       }

       return prompt;
   }
   // }}}
   ```

5. Handle preference changes:
   ```javascript
   // {{{ preference change handler
   function onPreferencesChanged() {
       // Mark all visible cards for regeneration with new style
       visibleCards.forEach(card => {
           if (card.needs_regen || userRequestedStyleUpdate) {
               artTracker.markForRegeneration(card);
           }
       });
   }
   // }}}
   ```

6. Write tests for style merging

## Related Documents
- 6-003a-regeneration-tracking.md
- 3-006-client-style-preferences.md
- 6-002-card-image-prompt-builder.md

## Dependencies
- 3-006: Client Style Preferences (localStorage structure)
- 6-002: Card Image Prompt Builder (base prompts)

## Example Prompt Build

```
Card: Dire Bear (Wilds faction, +1 attack upgrade)

User Style: "dark fantasy, oil painting, dramatic lighting"
User Negative: "cartoon, anime"

Final Prompt:
Positive: "A fierce dire bear, claws raised, in a forest clearing,
           forest, primal, natural, beasts, green tones,
           dark fantasy, oil painting, dramatic lighting,
           glowing runes, enhanced, magical aura"

Negative: "cartoon, anime, bright colors, modern"
```

## Acceptance Criteria
- [ ] Style guide loaded from localStorage
- [ ] Negative prompts included
- [ ] Faction styles applied
- [ ] Upgrade visuals added for upgraded cards
- [ ] Preference changes trigger updates
