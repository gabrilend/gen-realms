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
- [x] Faction styles defined completely
- [x] Base style applied to all images
- [x] Negative prompts suppress unwanted elements
- [x] Output maintains fantasy aesthetic
- [x] Multi-faction cards blend styles

## Implementation Notes

Implemented as `assets/web/style-transfer.js`:

### Complete Faction Style Definitions
Each faction has complete style definition including:
- `keywords` - Positive prompt additions
- `negative` - Things to avoid
- `colorPalette` - Array of hex color values
- `colorNames` - Human-readable color names
- `visualMotifs` - Key visual elements
- `environmentHints` - Scene/location suggestions
- `lightingStyle` - Lighting direction
- `referenceImage` - Path to style reference

### Factions Defined
- **Merchant** (Trade Federation): Gold, purple, white, navy - ships, coins, banners
- **Wilds** (The Wilds): Green, brown, amber, red - beasts, forests, claws
- **Kingdom** (The Kingdom): Silver, blue, crimson, white - knights, castles, swords
- **Artificer** (The Artificers): Bronze, copper, blue, purple - gears, constructs, runes
- **Neutral** (Unaligned): Grey, silver, indigo, white - symbols, portals, mist

### Art Style Presets
- `painterly` - Traditional oil painting style (cfg 7.5, 30 steps)
- `detailed` - Intricate rendering (cfg 8.0, 35 steps)
- `stylized` - Bold graphic novel aesthetic (cfg 7.0, 28 steps)
- `icon` - Simplified iconic style (cfg 6.5, 25 steps)
- `cinematic` - Epic movie poster quality (cfg 8.5, 35 steps)

### Key Functions
- `blendFactionStyles(factions, weights)` - Multi-faction style blending
- `buildStylePrompt(card, options)` - Complete styled prompt generation
- `validateStyleConsistency(prompt, expectedFaction)` - Style validation
- `loadReferenceImage(faction)` - Load reference images
- `generateColorCSS()` - Generate CSS custom properties

### Integration
Works alongside `style-merger.js` which handles:
- User preference merging
- Art regeneration triggering
- Queue integration

## Completion Date
2026-02-11
