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

1. Each player is presented with 3 randomly generated art pieces
2. Art depicts places or things (landscapes, structures, artifacts, creatures)
3. Players select one image each - no adjustments, no value judgments
4. The selections combine to form a "world seed" that influences all
   subsequent art generation during the match
5. Players are simply instructed: "Choose one" - not asked to rate or modify

### Player Experience

```
╔══════════════════════════════════════════════════════════════╗
║                    CHOOSE YOUR REALM                          ║
╠══════════════════════════════════════════════════════════════╣
║                                                               ║
║   [1]              [2]              [3]                       ║
║   ┌──────────┐    ┌──────────┐    ┌──────────┐               ║
║   │ Misty    │    │ Crystal  │    │ Ancient  │               ║
║   │ Fjord    │    │ Cavern   │    │ Library  │               ║
║   └──────────┘    └──────────┘    └──────────┘               ║
║                                                               ║
║              Select 1, 2, or 3 to continue                    ║
╚══════════════════════════════════════════════════════════════╝
```

### Art Categories

Each of the 3 options should be from different categories to ensure variety:

1. **Landscape** - Natural environments (forests, mountains, seas, deserts)
2. **Structure** - Built environments (castles, ruins, temples, markets)
3. **Object** - Significant items (artifacts, vessels, monuments, gateways)

The categories are shuffled so players don't always see landscape-structure-object.

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
       SEED_CATEGORY_LANDSCAPE,
       SEED_CATEGORY_STRUCTURE,
       SEED_CATEGORY_OBJECT
   } SeedCategory;

   typedef struct {
       char* image_prompt;      /* Prompt used to generate the image */
       char* image_path;        /* Path to generated image */
       uint32_t generation_seed; /* Reproducible generation seed */
       SeedCategory category;
   } SeedOption;

   typedef struct {
       SeedOption options[3];   /* The 3 choices shown to player */
       int selected;            /* Which one player chose (0-2, -1 if pending) */
   } PlayerSeedChoice;

   typedef struct {
       PlayerSeedChoice player_choices[MAX_PLAYERS];
       char* composite_prompt;  /* Combined prompt for art generation */
       uint32_t world_seed;     /* Final combined seed value */
       bool finalized;          /* True when all players have chosen */
   } WorldSeed;
   // }}}
   ```

2. Create seed generation prompts:
   ```c
   // {{{ prompt templates
   const char* LANDSCAPE_PROMPTS[] = {
       "a misty fjord at dawn with ancient standing stones",
       "a crystalline desert under twin moons",
       "a floating island chain connected by rope bridges",
       // ... more options
   };
   // }}}
   ```

3. Implement `world_seed_generate_options()`:
   - Select one prompt from each category
   - Shuffle the order
   - Generate images via ComfyUI (or use cached pool)
   - Return 3 SeedOption structs

4. Implement `world_seed_player_select()`:
   - Record player's choice
   - Check if all players have selected
   - If complete, call `world_seed_finalize()`

5. Implement `world_seed_finalize()`:
   - Combine selected prompts into composite world description
   - Calculate combined seed value
   - Store for use by all subsequent art generation

6. Add game phase `PHASE_WORLD_SEED`:
   - Insert before `PHASE_DRAW_ORDER`
   - Transition to draw order once all players select

7. Implement client rendering for seed selection:
   - Terminal: ASCII art placeholders with descriptions
   - Browser: Display actual generated images

8. Integrate with existing art generation:
   - Pass `WorldSeed* world` to all image generation calls
   - Prepend composite_prompt to card-specific prompts

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

- [ ] 3 art options generated and displayed to each player
- [ ] Options are from different categories (landscape/structure/object)
- [ ] Players can select with single input (1, 2, or 3)
- [ ] No UI for modification or regeneration
- [ ] Both players' choices combine into world seed
- [ ] World seed influences all subsequent art generation
- [ ] Game proceeds to draw order phase after both select
- [ ] Selection timeout (60s) auto-selects randomly if player AFK

## UI Copy

**Terminal:**
```
The mists part to reveal three visions of your realm.
Choose one to anchor reality.

[1] A misty fjord at dawn...
[2] A crystal cavern deep below...
[3] An ancient library forgotten by time...

> _
```

**Browser:**
```
Three visions emerge from the void.
Choose one.
```

No explanations of consequences. No "this will affect..." text.
Just: Choose.
