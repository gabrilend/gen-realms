# 5-004: Force Description Prompts

## Status: COMPLETED

## Current Behavior
Force description system generates thematic descriptions with faction themes and caching.

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
- [x] Force descriptions generate correctly
- [x] Faction themes reflected
- [x] Card descriptions are dramatic
- [x] Base descriptions included
- [x] Descriptions cached appropriately

## Implementation Notes

### Files Created
- `src/llm/04-force-description.h` - Force description interface
- `src/llm/04-force-description.c` - Implementation with faction themes
- `tests/test-force-desc.c` - Unit tests (18 tests, all passing)

### Key Features
- FactionTheme struct with adjectives, nouns, verbs for each faction
- ForceDescCache with circular buffer for description caching
- Prompt builders for player forces, cards played, bases, attacks
- Faction dominance detection from played cards
- Force summarization for narrative context

### Faction Themes Defined
- Neutral: versatile, adaptable (scouts, explorers)
- Merchant: gilded, prosperous (caravans, trade ships)
- Wilds: primal, savage (beasts, dire wolves)
- Kingdom: noble, valiant (knights, banners)
- Artificer: arcane, mechanical (constructs, gears)

### Test Coverage
- Cache creation, set/get, update, circular buffer
- Faction theme retrieval
- Faction adjective/noun random selection
- Dominant faction detection
- Force summarization
- Prompt building (player, card, base, attack)

Completed: 2026-02-11
