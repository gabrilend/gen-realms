# 6-008: Style Transfer Prompts

## Current Behavior
No style transformation for generated images.

## Intended Behavior
Prompt engineering to maintain consistent fantasy aesthetic:
- Fantasy art style keywords
- Faction-specific visual language
- Negative prompts to avoid unwanted elements
- Style reference images per faction
- Consistent color palettes

## Suggested Implementation Steps

1. Create `src/visual/style-guide.h`:
   ```c
   // {{{ style types
   typedef struct {
       char* style_keywords;
       char* negative_keywords;
       char* color_palette;
       char* reference_image_path;
   } FactionStyle;

   typedef struct {
       FactionStyle* factions;
       char* base_style;
       char* base_negative;
   } StyleGuide;
   // }}}
   ```

2. Define faction styles:
   ```c
   // {{{ faction styles
   static const FactionStyle FACTION_STYLES[] = {
       [FACTION_MERCHANT] = {
           .style_keywords = "golden, wealthy, trade ships, coins, banners, opulent",
           .negative_keywords = "poor, rusted, broken",
           .color_palette = "gold, purple, white, deep blue",
           .reference_image_path = "assets/styles/merchant_ref.png"
       },
       [FACTION_WILDS] = {
           .style_keywords = "forest, primal, beasts, nature, untamed, savage",
           .negative_keywords = "mechanical, urban, civilized",
           .color_palette = "green, brown, amber, crimson",
           .reference_image_path = "assets/styles/wilds_ref.png"
       },
       [FACTION_KINGDOM] = {
           .style_keywords = "knights, castles, heraldry, armor, noble, chivalric",
           .negative_keywords = "barbaric, monstrous, chaotic",
           .color_palette = "silver, blue, red, white",
           .reference_image_path = "assets/styles/kingdom_ref.png"
       },
       [FACTION_ARTIFICER] = {
           .style_keywords = "constructs, gears, enchanted, arcane machinery, magical tech",
           .negative_keywords = "organic, natural, primitive",
           .color_palette = "bronze, copper, blue energy, purple",
           .reference_image_path = "assets/styles/artificer_ref.png"
       }
   };
   // }}}
   ```

3. Define base style:
   ```c
   // {{{ base style
   static const char* BASE_STYLE =
       "fantasy art, medieval, magical, painterly, "
       "detailed illustration, card game art, high quality";

   static const char* BASE_NEGATIVE =
       "science fiction, modern, photograph, realistic, "
       "blurry, low quality, text, watermark, signature";
   // }}}
   ```

4. Implement `style_build_prompt()`:
   ```c
   // {{{ build prompt
   char* style_build_prompt(StyleGuide* guide, Faction faction,
                            const char* subject) {
       FactionStyle* fs = &guide->factions[faction];
       return format_string("%s, %s, %s",
           subject, fs->style_keywords, guide->base_style);
   }
   // }}}
   ```

5. Implement `style_get_negative()` for negative prompts

6. Add style interpolation for multi-faction cards

7. Implement style consistency validation

8. Write tests for prompt construction

## Related Documents
- 6-002-card-image-prompt-builder.md
- 4-003-faction-style-guides.md

## Dependencies
- 6-002: Card Image Prompt Builder
- 4-003: Faction Style Guides

## Style Examples

| Faction | Visual Motifs | Color Palette |
|---------|---------------|---------------|
| Merchant | Ships, coins, banners | Gold, purple, white |
| Wilds | Beasts, forests, claws | Green, brown, amber |
| Kingdom | Knights, castles, swords | Silver, blue, red |
| Artificer | Gears, constructs, runes | Bronze, copper, blue |

## Acceptance Criteria
- [ ] Faction styles defined completely
- [ ] Base style applied to all images
- [ ] Negative prompts suppress unwanted elements
- [ ] Output maintains fantasy aesthetic
- [ ] Multi-faction cards blend styles
