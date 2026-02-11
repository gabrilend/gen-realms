# 7-001: World Seed Selection

## Phase 7: Post-MVP Enhancements

This issue introduces the world seed selection mechanic that occurs at the
start of each match, allowing players to collectively shape the visual
identity of the game world.

## Current Behavior

Games begin with no world context. Art generation uses default or random
seeds without player input.

## Intended Behavior

At the start of each match, before the first turn:

1. Each player is privately presented with 3 randomly generated art pieces
2. Players cannot see each other's options - each set is unique and private
3. Players select one image - or right-click to "move away" from unwanted options
4. The selections combine to form a "world seed" that influences all
   subsequent art generation during the match
5. Players are simply instructed: "Choose one" - not asked to rate or explain

### Privacy

Each player's three visions are their own. You never see what the other
player was offered. You only see what they chose, and only after both
have committed. This creates asymmetric world-building - each player
contributes something the other never witnessed being offered.

### Moving Away (Right-Click)

Right-clicking an image dismisses it - you "move away" from that vision.
This is not rating or rejection with explanation. You simply turn from it.
Dismissed options fade. You must eventually choose from what remains.
If you dismiss all three, three new visions emerge.

### Visual Style

Each image depicts a single object or scene, lit as if by a spotlight with
dust motes visible in the streaming light. The focus is intimate and specific:

- A worn leather journal open on a stone altar, annotations in three hands
- A brass astrolabe suspended by chains over a reflecting pool
- A throne of twisted driftwood in a salt-crusted cave
- Shelves of glass bottles containing captured storms
- A marble pedestal in the center of a great lair, supporting annotated tomes
- Statues and bookshelves and trailing greenery, rotating in and out of sunlight

The style evokes a moment of discovery - you've entered a space and your eye
has fallen upon this one thing that defines it.

### Image Generation Source

Images are derived from card descriptions, but reframed as tangential questions:

| Card Concept | Reframed As |
|--------------|-------------|
| A faction's warriors | Where do they sleep? What marks their territory? |
| A beast of the wilds | What does their wilderness look like at dawn? |
| An artificer's creation | What tools hang on their workshop wall? |
| A kingdom's authority | What object represents their law? |
| A merchant's wealth | What single treasure defines their hoard? |

Abstract concepts are made tangible:

| Concept | Tangible Representation |
|---------|------------------------|
| Knowledge | Annotated books on marble, suspended in a scholar's cave |
| Power | A cracked crown resting on velvet in a locked cabinet |
| Time | An hourglass frozen mid-flow, dust settled on its frame |
| Memory | A child's toy on a windowsill, sun-bleached and forgotten |
| Hope | A single green shoot in cracked stone, lit from above |

### Art Categories

Each of the 3 options comes from a different perspective on the world:

1. **Habitation** - Where beings dwell (lairs, camps, sanctuaries, ruins)
2. **Wilderness** - Untamed spaces (forests, storms, depths, heights)
3. **Artifact** - Objects of significance (tools, relics, symbols, vessels)

The categories are shuffled so players don't always see the same ordering.

### World Seed Mechanics

- Both players' choices are combined into a composite world seed
- The seed influences: color palette, architectural style, creature design,
  artifact aesthetics, narrative tone
- If players choose the same category, that aspect is emphasized
- If players choose different categories, the world blends both influences

## Suggested Implementation Steps

1. Create `src/core/08-world-seed.h`:
   ```c
   // {{{ world seed types
   typedef enum {
       SEED_CATEGORY_HABITATION,  /* Where beings dwell */
       SEED_CATEGORY_WILDERNESS,  /* Untamed spaces */
       SEED_CATEGORY_ARTIFACT     /* Objects of significance */
   } SeedCategory;

   typedef struct {
       char* source_card_id;     /* Card this image derives from */
       char* reframed_question;  /* "Where do they sleep?" etc. */
       char* image_prompt;       /* Full prompt for generation */
       char* image_path;         /* Path to generated image */
       uint32_t generation_seed; /* Reproducible generation seed */
       SeedCategory category;
       bool dismissed;           /* True if player moved away from this */
   } SeedOption;

   typedef struct {
       SeedOption* options;      /* Dynamic array - grows if all dismissed */
       int option_count;
       int visible_count;        /* Options not yet dismissed */
       int selected;             /* Which one player chose (-1 if pending) */
       int generation_round;     /* How many times new options were needed */
   } PlayerSeedChoice;

   typedef struct {
       PlayerSeedChoice player_choices[MAX_PLAYERS];
       char* composite_prompt;   /* Combined prompt for art generation */
       uint32_t world_seed;      /* Final combined seed value */
       bool finalized;           /* True when all players have chosen */
       bool revealed;            /* True when choices shown to both players */
   } WorldSeed;
   // }}}
   ```

2. Create reframing question templates:
   ```c
   // {{{ reframing questions
   /* Questions that derive world-building from card concepts */
   const char* HABITATION_QUESTIONS[] = {
       "Where do they rest when the battle is done?",
       "What marks the boundary of their territory?",
       "What light illuminates their sanctuary?",
   };

   const char* WILDERNESS_QUESTIONS[] = {
       "What does their wilderness look like at dawn?",
       "What grows in the places they have touched?",
       "What weather follows in their wake?",
   };

   const char* ARTIFACT_QUESTIONS[] = {
       "What single object defines their purpose?",
       "What tool hangs on their workshop wall?",
       "What treasure would they never trade?",
   };
   // }}}

   // {{{ visual style prefix
   /* Prepended to all prompts for consistent spotlight aesthetic */
   const char* STYLE_PREFIX =
       "A single spotlight illuminates the scene, dust motes visible "
       "in the streaming light. Intimate focus on one defining object "
       "or space. The moment of discovery - entering and seeing this "
       "one thing that defines the place. ";
   // }}}
   ```

3. Implement `world_seed_generate_options()`:
   - Select one card from each faction for variety
   - Apply reframing question to card description
   - Prepend visual style prefix
   - Generate images via ComfyUI
   - Return 3 SeedOption structs (one per category)

4. Implement `world_seed_dismiss_option()`:
   - Mark option as dismissed, fade visually
   - If all 3 dismissed, call `world_seed_generate_options()` again
   - Increment generation_round counter

5. Implement `world_seed_player_select()`:
   - Record player's choice from non-dismissed options
   - Check if all players have selected
   - If complete, call `world_seed_finalize()`

6. Implement `world_seed_finalize()`:
   - Combine selected prompts into composite world description
   - Calculate combined seed value
   - Mark as finalized but not yet revealed

7. Implement `world_seed_reveal()`:
   - Called when both players ready to see results
   - Each player sees: their choice + opponent's choice
   - Never sees: opponent's other options

8. Add game phase `PHASE_WORLD_SEED`:
   - Insert before `PHASE_DRAW_ORDER`
   - Sub-phases: CHOOSING → FINALIZING → REVEALING → COMPLETE
   - Transition to draw order once revealed

9. Implement client rendering:
   - Browser: spotlight-lit images, right-click to dismiss (fade animation)
   - Terminal: evocative text descriptions with [X] to dismiss

10. Integrate with existing art generation:
    - Pass `WorldSeed* world` to all image generation calls
    - Prepend composite_prompt to card-specific prompts
    - Use world_seed for consistent style transfer

## Related Documents

- 6-002-card-image-prompt-builder.md
- 6-008-style-transfer-prompts.md
- 5-003-world-state-prompt.md

## Dependencies

- Phase 6 complete (art generation pipeline)
- ComfyUI API client (6-001)

## Design Principles

1. **No Value Judgments**: Players are never asked "which do you prefer" or
   "rate these options". They simply choose.

2. **No Adjustments**: Players cannot modify, regenerate, or fine-tune options.
   The 3 presented are the 3 available.

3. **Meaningful Choice**: Each option meaningfully affects the game's visual
   identity, but no choice is "better" - they're different.

4. **Quick Selection**: This should take under 30 seconds per player. The
   images are pre-generated or generated in parallel during matchmaking.

5. **Reproducibility**: Given the same seeds, the same world should be
   generated, allowing replay of memorable matches.

## Acceptance Criteria

- [ ] 3 art options generated privately for each player
- [ ] Players cannot see each other's options
- [ ] Options derived from card descriptions via reframing questions
- [ ] Visual style: spotlight, dust, intimate focus
- [ ] Options are from different categories (habitation/wilderness/artifact)
- [ ] Players can select with click/enter (1, 2, or 3)
- [ ] Players can dismiss with right-click/backspace
- [ ] Dismissing all 3 generates 3 new options
- [ ] Both players' choices combine into world seed
- [ ] After both select: reveal phase shows both chosen visions
- [ ] World seed influences all subsequent art generation
- [ ] Game proceeds to draw order phase after reveal
- [ ] Selection timeout (90s) auto-selects randomly if player AFK

## UI Copy

**Terminal:**
```
Three visions, yours alone.

[1] A worn journal on a stone altar, annotations in three hands...
[2] Shelves of glass bottles, each containing a captured storm...
[3] A brass astrolabe suspended by chains over still water...

Choose one. Or press [X] to turn away.

> _
```

**Terminal (after dismissing one):**
```
Two visions remain.

[1] A worn journal on a stone altar, annotations in three hands...
[  ] (you turned away)
[3] A brass astrolabe suspended by chains over still water...

> _
```

**Terminal (reveal phase):**
```
The world takes shape.

Your vision:
  A worn journal on a stone altar...

Their vision:
  A throne of twisted driftwood in a salt-crusted cave...

The game begins.
```

**Browser:**
```
Three visions, yours alone.
Choose one.
```

(Right-click instruction appears on hover, not before)

No explanations of consequences. No "this will affect..." text.
No indication of what the other player is seeing or doing.
Just: Choose.
