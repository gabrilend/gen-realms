# 5-004: Force Description Prompts

## Current Behavior
No faction/force descriptions generated.

## Intended Behavior
Prompts that generate thematic descriptions for:
- Player's current forces (cards in play)
- Faction army composition
- Individual card narratives
- Base descriptions

## Suggested Implementation Steps

1. Create force description templates:
   ```c
   // {{{ templates
   static const char* FORCE_TEMPLATE =
       "Describe the forces of %s, who commands: %s. "
       "Their faction allegiance leans toward %s. "
       "Use vivid fantasy imagery in 2-3 sentences.";

   static const char* CARD_TEMPLATE =
       "Describe the %s joining the battle. "
       "It is a %s card from the %s faction. "
       "One dramatic sentence.";
   // }}}
   ```

2. Implement `force_description_build()`:
   ```c
   // {{{ build force description
   char* force_description_build(Player* player) {
       char* cards_list = cards_to_string(player->in_play);
       char* faction = get_dominant_faction(player);

       return prompt_build(PROMPT_FORCE_DESCRIPTION, &(PromptVars){
           .player_name = player->name,
           .cards = cards_list,
           .faction = faction
       });
   }
   // }}}
   ```

3. Implement card-specific descriptions

4. Implement base descriptions

5. Cache descriptions to avoid regeneration

6. Write tests

## Related Documents
- 5-002-prompt-network-structure.md
- 5-003-world-state-prompt.md

## Dependencies
- 5-002: Prompt Network Structure
- 5-003: World State Prompt

## Faction Themes

| Faction | Theme Keywords |
|---------|---------------|
| Merchant | Gold, caravans, trade ships, prosperity, cunning |
| Wilds | Beasts, forests, primal fury, nature's wrath |
| Kingdom | Knights, castles, honor, gleaming armor, banners |
| Artificer | Constructs, arcane, purple energy, workshops |

## Example Output

```
Card Played: Dire Bear (Wilds)

"From the shadowed depths of the Thornwood, a massive dire
bear answers the call to battle, its roar shaking the very
foundations of the contested realm."
```

## Acceptance Criteria
- [ ] Force descriptions generate correctly
- [ ] Faction themes reflected
- [ ] Card descriptions are dramatic
- [ ] Base descriptions included
- [ ] Descriptions cached appropriately
