# 4-003: Reference Image Mapping

## Current Behavior
Cards have art_ref but no actual reference images.

## Intended Behavior
A system that maps game cards to reference images for inpainting:
- Each faction has visual style references
- Each card type has shape/silhouette references
- References guide the inpainting model
- Supports fallbacks for missing references

## Suggested Implementation Steps

1. Create `assets/references/` directory structure:
   ```
   assets/references/
     factions/
       merchant/style.png
       wilds/style.png
       kingdom/style.png
       artificer/style.png
     cards/
       {card_id}.png
     silhouettes/
       ship.png
       base.png
       unit.png
   ```
2. Create `src/visual/references.lua`
3. Implement `Ref.get_for_card(card)` - return reference image(s)
4. Implement `Ref.get_faction_style(faction)` - return style ref
5. Implement `Ref.get_silhouette(card_type)` - return shape ref
6. Implement fallback chain (card -> faction -> silhouette)
7. Support reference blending (for multi-faction scenes)
8. Write reference contribution guidelines
9. Create placeholder references for development

## Related Documents
- 2-009-card-art-placeholder-system.md
- notes/vision (card references)

## Dependencies
- 2-009: Card Art Placeholder System

## Reference Usage

```
Generating: Dire Bear in battle

References:
1. Card reference: dire_bear.png (if exists)
2. Faction style: wilds/style.png (forest, primal)
3. Silhouette: ship.png (creature shape)

Prompt influence:
"A dire bear in the style of [wilds], with shape like [ship]"
```

## Acceptance Criteria
- [ ] Reference lookup works for all cards
- [ ] Faction styles defined
- [ ] Silhouettes provide shape guidance
- [ ] Fallbacks handle missing images
- [ ] References loadable by inpainting module
