# 8-010: Strategy Learning System

## Current Behavior
AI has no ability to learn from game outcomes or improve over time.

## Intended Behavior
A learning system that tracks which strategies succeed or fail in different
situations, updating strategy selection weights based on outcomes. Optionally
integrates with machine learning for deeper pattern recognition.

## Suggested Implementation Steps

1. Create `src/ai/10-learning.h` with learning types:
   ```c
   typedef struct LearningSystem LearningSystem;
   typedef struct GameRecord GameRecord;
   typedef struct StrategyStats StrategyStats;

   /* Record of a completed game */
   struct GameRecord {
       uint32_t game_id;

       /* Outcome */
       int winner_id;           /* Which player won */
       int final_authority[2];  /* Ending authority */
       int turn_count;          /* Game length */

       /* Strategy usage */
       StrategyArchetype strategies_used[32];  /* Per turn */
       int strategy_count;

       /* Key moments */
       int pivotal_turns[8];    /* Turns where outcome shifted */
       int pivotal_count;

       /* Opponent info */
       OpponentProfile opponent;
   };

   /* Statistics for a strategy */
   struct StrategyStats {
       StrategyArchetype strategy;

       /* Win rates */
       int games_played;
       int games_won;
       float win_rate;

       /* Situational win rates */
       float win_rate_when_ahead;
       float win_rate_when_behind;
       float win_rate_when_even;

       /* Matchup data */
       float win_rate_vs_aggro;
       float win_rate_vs_control;
       float win_rate_vs_combo;

       /* Timing data */
       float avg_game_length_wins;
       float avg_game_length_losses;
   };
   ```

2. Create `src/ai/10-learning.c` implementing:
   - Game recording
   - Statistics tracking
   - Weight updates

3. Implement recording system:
   ```c
   /* Create learning system */
   LearningSystem* learning_create(const char* data_path);
   void learning_destroy(LearningSystem* ls);

   /* Record game outcome */
   void learning_record_game(LearningSystem* ls, GameRecord* record);

   /* Load/save learned data */
   bool learning_save(LearningSystem* ls, const char* path);
   bool learning_load(LearningSystem* ls, const char* path);
   ```

4. Implement statistics queries:
   ```c
   /* Get strategy statistics */
   StrategyStats* learning_get_stats(LearningSystem* ls,
                                       StrategyArchetype strategy);

   /* Get recommended strategy for situation */
   StrategyArchetype learning_recommend(LearningSystem* ls,
                                          SituationAssessment* sit,
                                          OpponentProfile* opp);

   /* Get confidence in recommendation */
   float learning_confidence(LearningSystem* ls,
                              StrategyArchetype strategy,
                              SituationAssessment* sit);
   ```

5. Implement weight updates:
   ```c
   /* Update strategy weights based on outcome */
   void learning_update_weights(LearningSystem* ls,
                                  GameRecord* record);

   /* Learning rate control */
   void learning_set_rate(LearningSystem* ls, float rate);

   /* Decay old data over time */
   void learning_decay(LearningSystem* ls, float factor);
   ```

6. Implement pattern recognition (optional ML integration):
   ```c
   /* Extract features from game state */
   typedef struct {
       float features[64];  /* Numeric state representation */
       int feature_count;
   } StateFeatures;

   StateFeatures learning_extract_features(Game* state, int player_id);

   /* Train simple model on outcomes */
   void learning_train_model(LearningSystem* ls,
                              StateFeatures* features,
                              float* outcomes,
                              int count);

   /* Predict outcome from features */
   float learning_predict(LearningSystem* ls, StateFeatures* features);
   ```

## Learning Data Flow

```
┌─────────────────┐
│  Game Played    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Record Game    │
│  - Strategies   │
│  - Outcome      │
│  - Key moments  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Update Stats   │
│  - Win rates    │
│  - Matchups     │
│  - Situations   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Adjust Weights │
│  for strategy   │
│  selection      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Future Games   │
│  use updated    │
│  preferences    │
└─────────────────┘
```

## Weight Update Formula

```
new_weight = old_weight + learning_rate × (outcome - expected)

Where:
- outcome = 1.0 for win, 0.0 for loss
- expected = current win rate for this strategy in this situation
- learning_rate = how much to adjust (default 0.1)

With situation weighting:
situation_weight = 1.0 / (1.0 + similarity_to_seen_situations)
```

## Strategy Performance Table (Example)

```
Strategy    │ Overall │ Ahead │ Behind │ vs Aggro │ vs Control │
────────────┼─────────┼───────┼────────┼──────────┼────────────┤
RUSH        │  52%    │  68%  │  31%   │   45%    │    61%     │
CONTROL     │  48%    │  55%  │  42%   │   58%    │    44%     │
MIDRANGE    │  51%    │  54%  │  49%   │   52%    │    50%     │
COMBO       │  47%    │  70%  │  28%   │   40%    │    55%     │
ATTRITION   │  49%    │  45%  │  53%   │   60%    │    38%     │
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-006-strategy-orchestrator.md
- 8-008-opponent-modeling.md

## Dependencies
- 8-006: Strategy Orchestrator (strategy selection)
- 8-008: Opponent Modeling (matchup tracking)

## Acceptance Criteria
- [ ] Game outcomes recorded with strategy usage
- [ ] Statistics accurately reflect win rates
- [ ] Weight updates improve strategy selection
- [ ] Situational recommendations are meaningful
- [ ] Learning data persists between sessions
- [ ] Optional ML integration works correctly

## Notes
The learning system is what makes the AI improve over time. It should be
conservative (slow updates) to avoid overfitting to recent games, but
responsive enough to adapt to new strategies or meta shifts.
