# 6-002: Card Image Prompt Builder

## Current Behavior
No system to generate prompts for card art.

## Intended Behavior
A prompt builder that creates ComfyUI-compatible prompts for card art:
- Uses card metadata (name, faction, type)
- Applies faction style guides
- Includes fantasy aesthetic keywords
- Supports different art styles
- Generates negative prompts

## Suggested Implementation Steps

1. Create `src/visual/prompt-builder.h`:
   ```c
   // {{{ prompt types
   typedef struct {
       char* positive;
       char* negative;
       int width;
       int height;
       int steps;
       float cfg_scale;
   } ImagePrompt;

   typedef enum {
       STYLE_PAINTERLY,
       STYLE_DETAILED,
       STYLE_STYLIZED,
       STYLE_ICON
   } ArtStyle;
   // }}}
   ```

2. Define faction style templates:
   ```c
   // {{{ faction styles
   static const char* FACTION_STYLES[] = {
       [FACTION_MERCHANT] = "golden banners, coins, trade ships, wealthy, opulent",
       [FACTION_WILDS] = "forest creatures, primal, nature, beasts, untamed",
       [FACTION_KINGDOM] = "knights, castles, heraldry, armor, noble",
       [FACTION_ARTIFICER] = "constructs, gears, enchanted machines, arcane tech"
   };
   // }}}
   ```

3. Implement `prompt_build_card()`:
   ```c
   // {{{ build card prompt
   ImagePrompt* prompt_build_card(CardType* card, ArtStyle style) {
       ImagePrompt* p = malloc(sizeof(ImagePrompt));

       // Build positive prompt
       p->positive = format_prompt(
           "%s, %s, fantasy card art, %s style, detailed",
           card->name,
           FACTION_STYLES[card->faction],
           STYLE_KEYWORDS[style]
       );

       // Standard negative prompt
       p->negative = strdup(NEGATIVE_PROMPT_BASE);

       return p;
   }
   // }}}
   ```

4. Implement `prompt_build_creature()` for creature cards

5. Implement `prompt_build_base()` for base/location cards

6. Implement `prompt_build_action()` for action/event cards

7. Add reference image support for style consistency

8. Write tests for prompt generation

## Related Documents
- 5-002-prompt-network-structure.md
- 4-002-card-database-schema.md

## Dependencies
- 4-002: Card Database Schema (card metadata)

## Prompt Templates

| Card Type | Prompt Elements |
|-----------|-----------------|
| Creature | Name, faction style, creature type, action pose |
| Base | Location name, faction style, architectural |
| Action | Effect visualization, magical energy, dynamic |
| Champion | Portrait, faction style, heroic pose |

## Acceptance Criteria
- [ ] Generates prompts for all card types
- [ ] Faction styles applied consistently
- [ ] Negative prompts suppress unwanted elements
- [ ] Prompts produce cohesive fantasy art
- [ ] Style parameter affects output
