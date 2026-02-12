# 8-009: AI Difficulty Profiles

## Current Behavior
No system exists for configuring AI strength or creating varied difficulty levels.

## Intended Behavior
A system that creates configurable AI difficulty levels from "chaotic beginner"
to "optimal calculator", where difficulty affects search depth, evaluation
accuracy, and introduces deliberate imperfection for lower levels.

## Suggested Implementation Steps

1. Create `src/ai/09-difficulty.h` with difficulty types:
   ```c
   typedef struct DifficultyProfile DifficultyProfile;
   typedef struct DifficultyManager DifficultyManager;

   /* Preset difficulty levels */
   typedef enum {
       DIFF_TUTORIAL,     /* Makes obvious mistakes, teaches basics */
       DIFF_EASY,         /* Beatable by new players */
       DIFF_NORMAL,       /* Reasonable challenge */
       DIFF_HARD,         /* Requires good play to beat */
       DIFF_EXPERT,       /* Near-optimal play */
       DIFF_MASTER,       /* Maximum strength */
       DIFF_CUSTOM        /* User-defined parameters */
   } DifficultyLevel;

   /* Difficulty configuration */
   struct DifficultyProfile {
       DifficultyLevel level;

       /* Search limitations */
       int max_depth;           /* Tree search depth */
       int time_budget_ms;      /* Thinking time */
       int node_budget;         /* Nodes to explore */

       /* Deliberate imperfection */
       float mistake_rate;      /* Chance to pick suboptimal (0-1) */
       float mistake_magnitude; /* How bad mistakes are (0-1) */
       float randomness;        /* Exploration vs exploitation */

       /* Evaluation blindspots */
       float authority_blindness;   /* Undervalue authority (0-1) */
       float tempo_blindness;       /* Undervalue tempo (0-1) */
       float long_term_blindness;   /* Ignore future (0-1) */

       /* Personality variance */
       float personality_strength;  /* How much motivation matters */
       float adaptability;          /* How well it reads opponent */

       /* Tutorial features */
       bool hint_when_ahead;        /* Give player hints */
       bool explain_moves;          /* Explain AI decisions */
       bool allow_takebacks;        /* Let player undo */
   };
   ```

2. Create `src/ai/09-difficulty.c` implementing:
   - Preset profiles
   - Mistake injection
   - Difficulty scaling

3. Implement preset profiles:
   ```c
   /* Create difficulty manager */
   DifficultyManager* difficulty_create(void);
   void difficulty_destroy(DifficultyManager* dm);

   /* Get preset profile */
   DifficultyProfile* difficulty_get_preset(DifficultyManager* dm,
                                             DifficultyLevel level);

   /* Create custom profile */
   DifficultyProfile* difficulty_create_custom(DifficultyManager* dm);
   ```

4. Implement mistake injection:
   ```c
   /* Possibly inject a mistake into action selection */
   AIAction* difficulty_filter_action(DifficultyManager* dm,
                                       DifficultyProfile* profile,
                                       AIAction* optimal,
                                       AIAction* alternatives,
                                       int alt_count);

   /* Types of mistakes */
   typedef enum {
       MISTAKE_WRONG_ORDER,    /* Play cards in bad sequence */
       MISTAKE_MISSED_ATTACK,  /* Forget to deal damage */
       MISTAKE_WRONG_BUY,      /* Buy suboptimal card */
       MISTAKE_EARLY_END,      /* End turn with actions left */
       MISTAKE_TUNNEL_VISION   /* Focus on one thing, miss other */
   } MistakeType;
   ```

5. Implement adaptive difficulty:
   ```c
   /* Dynamic difficulty adjustment */
   typedef struct {
       float player_skill_estimate;
       float recent_game_outcomes;  /* Win/loss ratio */
       float frustration_estimate;  /* From play patterns */
   } AdaptiveState;

   void difficulty_adaptive_update(DifficultyManager* dm,
                                    AdaptiveState* state,
                                    bool player_won);

   DifficultyProfile* difficulty_get_adaptive(DifficultyManager* dm,
                                               AdaptiveState* state);
   ```

6. Implement tutorial mode:
   ```c
   /* Tutorial helper functions */
   const char* difficulty_get_hint(DifficultyManager* dm,
                                    Game* state,
                                    int player_id);

   const char* difficulty_explain_ai_move(DifficultyManager* dm,
                                           AIAction* action,
                                           Game* state);
   ```

## Difficulty Presets

| Level | Depth | Think Time | Mistakes | Notes |
|-------|-------|------------|----------|-------|
| TUTORIAL | 1 | 100ms | 50% obvious | Explains moves, gives hints |
| EASY | 2 | 200ms | 30% | Misses some synergies |
| NORMAL | 3 | 500ms | 15% | Solid but beatable |
| HARD | 4 | 1000ms | 5% | Few mistakes |
| EXPERT | 5 | 2000ms | 1% | Near-optimal |
| MASTER | 6+ | 5000ms | 0% | Maximum strength |

## Mistake Distribution

```
Tutorial: ████████████████████░░░░░░░░░░  50% mistake rate
Easy:     ████████████░░░░░░░░░░░░░░░░░░  30% mistake rate
Normal:   ██████░░░░░░░░░░░░░░░░░░░░░░░░  15% mistake rate
Hard:     ██░░░░░░░░░░░░░░░░░░░░░░░░░░░░   5% mistake rate
Expert:   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   1% mistake rate
Master:   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░   0% mistake rate
```

## Adaptive Difficulty Flow

```
┌─────────────────────────────────────────┐
│          Player Skill Estimate          │
└────────────────────┬────────────────────┘
                     │
        ┌────────────┼────────────┐
        ▼            ▼            ▼
   Win Streak    Mixed Results   Lose Streak
        │            │            │
        ▼            ▼            ▼
   Increase      Maintain      Decrease
   Difficulty    Difficulty    Difficulty
        │            │            │
        └────────────┴────────────┘
                     │
                     ▼
          ┌─────────────────────┐
          │  Adjusted Profile   │
          │  (smooth transition)│
          └─────────────────────┘
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-006-strategy-orchestrator.md
- 8-007-tree-pruning-evaluation.md

## Dependencies
- 8-007: Tree Pruning and Evaluation (search depth control)
- 8-006: Strategy Orchestrator (strategy selection)

## Acceptance Criteria
- [ ] Preset difficulties have distinct skill levels
- [ ] Mistakes are injected realistically
- [ ] Tutorial mode provides helpful explanations
- [ ] Adaptive difficulty responds to player skill
- [ ] Custom profiles allow fine-tuning
- [ ] Lower difficulties feel "human-like" not "random"

## Notes
Good difficulty scaling is crucial for player enjoyment. The AI should feel
like a worthy opponent at all levels - not frustratingly perfect or
insultingly random. Mistakes should be plausible, not obviously artificial.
