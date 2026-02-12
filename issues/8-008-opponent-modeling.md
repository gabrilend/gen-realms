# 8-008: Opponent Modeling System

## Current Behavior
AI has no understanding of opponent tendencies or play patterns.

## Intended Behavior
A system that builds a probabilistic model of opponent behavior from observed
play, enabling the AI to predict opponent actions, exploit patterns, and
adapt strategy selection accordingly.

## Suggested Implementation Steps

1. Create `src/ai/08-opponent.h` with opponent model types:
   ```c
   typedef struct OpponentModel OpponentModel;
   typedef struct ObservedAction ObservedAction;
   typedef struct PredictedBehavior PredictedBehavior;

   /* Tracked opponent characteristics */
   typedef struct {
       /* Play style indicators (0.0 to 1.0) */
       float aggression;        /* How often they attack */
       float economy_focus;     /* How much they buy */
       float risk_taking;       /* Risky vs safe plays */
       float reactiveness;      /* Responds to us vs own plan */

       /* Statistical patterns */
       float avg_cards_played;  /* Per turn */
       float avg_damage_dealt;  /* Per turn */
       float avg_cards_bought;  /* Per turn */
       float scrap_rate;        /* How often they scrap */

       /* Faction preferences */
       float faction_weights[FACTION_COUNT];

       /* Timing patterns */
       int typical_aggro_turn;  /* When they start attacking */
       int typical_big_turn;    /* When they combo off */
   } OpponentProfile;

   /* Observed action record */
   struct ObservedAction {
       AIActionType type;
       int turn_number;
       Game* state_before;       /* Optional snapshot */
       float resource_available; /* What they had */
       float resource_spent;     /* What they used */
   };

   /* Prediction output */
   struct PredictedBehavior {
       AIActionType likely_action;
       float probability;
       float expected_damage;    /* If they attack */
       float expected_buys;      /* Card acquisitions */
   };
   ```

2. Create `src/ai/08-opponent.c` implementing:
   - Observation recording
   - Profile updating
   - Behavior prediction

3. Implement observation system:
   ```c
   /* Create opponent model */
   OpponentModel* opponent_model_create(void);
   void opponent_model_destroy(OpponentModel* model);

   /* Record opponent actions */
   void opponent_observe_action(OpponentModel* model,
                                 AIActionType action,
                                 Game* state);
   void opponent_observe_turn_end(OpponentModel* model,
                                   Game* state);

   /* Get current profile */
   OpponentProfile* opponent_get_profile(OpponentModel* model);
   ```

4. Implement prediction:
   ```c
   /* Predict next action */
   PredictedBehavior opponent_predict_action(OpponentModel* model,
                                               Game* state);

   /* Predict turn outcome */
   typedef struct {
       float expected_authority_change;
       float expected_cards_drawn;
       float attack_probability;
       float big_turn_probability;
   } TurnPrediction;

   TurnPrediction opponent_predict_turn(OpponentModel* model,
                                         Game* state);
   ```

5. Implement pattern detection:
   ```c
   /* Detect recurring patterns */
   typedef struct {
       const char* pattern_name;
       float confidence;
       int occurrences;
   } DetectedPattern;

   DetectedPattern* opponent_detect_patterns(OpponentModel* model,
                                               int* out_count);

   /* Known patterns to detect */
   bool opponent_is_rushing(OpponentModel* model);
   bool opponent_is_building(OpponentModel* model);
   bool opponent_is_reactive(OpponentModel* model);
   bool opponent_has_combo_potential(OpponentModel* model, Game* state);
   ```

6. Implement exploitation suggestions:
   ```c
   /* Get counter-strategy suggestions */
   typedef struct {
       StrategyArchetype suggested;
       float confidence;
       const char* reasoning;
   } ExploitSuggestion;

   ExploitSuggestion opponent_suggest_counter(OpponentModel* model,
                                                SituationAssessment* sit);
   ```

## Opponent Profile Evolution

```
Turn 1   Turn 3   Turn 5   Turn 7
  │        │        │        │
  ▼        ▼        ▼        ▼
┌────┐  ┌────┐  ┌────┐  ┌────┐
│????│→ │aggro│→ │AGGRO│→ │AGGRO│  Confidence increases
│????│  │econ?│  │econ │  │low-e│  as more data gathered
│????│  │????│  │risk+│  │risk+│
└────┘  └────┘  └────┘  └────┘
```

## Pattern Examples

| Pattern | Indicators | Counter |
|---------|------------|---------|
| Rush | Early attacks, ignores economy | Build defense, outlast |
| Turtle | Authority gain, no attacks | Build overwhelming force |
| Combo | Hoarding faction cards | Disrupt hand, pressure early |
| Reactive | Mirrors our plays | Be unpredictable, feint |
| Random | No clear pattern | Play straightforward optimal |

## Bayesian Update Formula

```
P(style|action) = P(action|style) × P(style) / P(action)

Where:
- P(style|action) = Updated probability of play style given observed action
- P(action|style) = Likelihood of this action given style (from statistics)
- P(style) = Prior probability of style (from history)
- P(action) = Total probability of this action
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-006-strategy-orchestrator.md
- 8-010-strategy-learning.md

## Dependencies
- 8-006: Strategy Orchestrator (counter-strategy selection)

## Acceptance Criteria
- [ ] Actions are recorded and aggregated into profile
- [ ] Profile reflects opponent play style accurately
- [ ] Predictions improve with more observations
- [ ] Patterns detected reliably
- [ ] Counter-strategy suggestions are actionable
- [ ] Model resets appropriately for new opponents

## Notes
Opponent modeling is crucial for high-level play. A strong AI doesn't just
play its own game - it adapts to exploit opponent weaknesses and avoid
their strengths. The model should be conservative early (low confidence)
and refine over time.
