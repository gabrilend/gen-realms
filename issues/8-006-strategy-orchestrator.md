# 8-006: Strategy Orchestrator

## Current Behavior
No meta-level system exists for choosing between different strategic approaches.

## Intended Behavior
A strategy orchestrator that acts as a "general" commanding "armies" of strategies.
It evaluates the current situation and selects which strategic approach (aggressive,
economic, defensive, etc.) to pursue, potentially switching mid-game as conditions
change.

## Suggested Implementation Steps

1. Create `src/ai/06-orchestrator.h` with orchestrator types:
   ```c
   typedef struct StrategyOrchestrator StrategyOrchestrator;
   typedef struct StrategyProfile StrategyProfile;
   typedef struct SituationAssessment SituationAssessment;

   /* Pre-defined strategy archetypes */
   typedef enum {
       STRAT_RUSH,        /* Fast damage, end game quickly */
       STRAT_CONTROL,     /* Deny opponent, slow game down */
       STRAT_MIDRANGE,    /* Balanced tempo and value */
       STRAT_COMBO,       /* Build towards big turns */
       STRAT_ATTRITION,   /* Outlast through defense */
       STRAT_ADAPTIVE,    /* Dynamic switching */
       STRAT_COUNT
   } StrategyArchetype;

   /* Strategy profile - defines a strategic approach */
   struct StrategyProfile {
       StrategyArchetype archetype;
       MotivationVector motivation_bias;  /* How it affects decisions */

       /* Strategy-specific parameters */
       float aggression_threshold;   /* When to attack vs build */
       float economy_target;         /* Desired deck strength */
       int horizon_depth;            /* How many turns to plan */
       float risk_tolerance;         /* Acceptable variance */

       /* Activation conditions */
       float min_authority_ratio;    /* Our auth / their auth */
       float max_authority_ratio;
       float min_tempo_score;
       float max_tempo_score;
   };

   /* Current situation analysis */
   struct SituationAssessment {
       float authority_ratio;        /* Our authority / opponent's */
       float tempo_score;            /* Board presence measure */
       float economy_score;          /* Deck quality measure */
       float threat_level;           /* Danger from opponent */
       int estimated_turns_left;     /* Until someone wins */

       /* Derived indicators */
       bool is_ahead;
       bool is_behind;
       bool is_desperate;
       bool has_initiative;
   };
   ```

2. Create `src/ai/06-orchestrator.c` implementing:
   - Strategy selection logic
   - Situation assessment
   - Transition management

3. Implement situation assessment:
   ```c
   /* Create orchestrator */
   StrategyOrchestrator* orchestrator_create(void);
   void orchestrator_destroy(StrategyOrchestrator* orch);

   /* Assess current situation */
   SituationAssessment orchestrator_assess(StrategyOrchestrator* orch,
                                            Game* state,
                                            int player_id);
   ```

4. Implement strategy selection:
   ```c
   /* Select best strategy for situation */
   StrategyProfile* orchestrator_select_strategy(StrategyOrchestrator* orch,
                                                   SituationAssessment* sit,
                                                   MotivationModel* motivation);

   /* Get strategy-adjusted action scores */
   void orchestrator_score_options(StrategyOrchestrator* orch,
                                    StrategyProfile* strategy,
                                    GeneratedOption* options,
                                    int count);
   ```

5. Implement strategy transitions:
   ```c
   /* Track strategy changes */
   typedef struct {
       StrategyArchetype from;
       StrategyArchetype to;
       int turn_number;
       const char* reason;
   } StrategyTransition;

   /* Check if strategy switch is warranted */
   bool orchestrator_should_switch(StrategyOrchestrator* orch,
                                    StrategyProfile* current,
                                    SituationAssessment* sit);

   /* Get transition history */
   StrategyTransition* orchestrator_get_history(StrategyOrchestrator* orch,
                                                  int* out_count);
   ```

6. Implement multi-strategy blending:
   ```c
   /* Blend multiple strategies for hybrid approach */
   void orchestrator_blend_strategies(StrategyOrchestrator* orch,
                                       StrategyProfile** strategies,
                                       float* weights,
                                       int count,
                                       StrategyProfile* out_blended);
   ```

## Strategy Archetype Details

| Archetype | Goal | Early Game | Mid Game | Late Game |
|-----------|------|------------|----------|-----------|
| RUSH | Win fast | Max aggro | All-in damage | Desperation |
| CONTROL | Deny opponent | Remove threats | Maintain control | Value wins |
| MIDRANGE | Flexible | Build board | Trade efficiently | Close out |
| COMBO | Big turns | Setup pieces | Protect combo | Execute |
| ATTRITION | Outlast | Authority gain | Survive | Exhaust opponent |

## Decision Flow

```
┌─────────────────────────────────────────────────────────────┐
│                  Strategy Orchestrator                       │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Situation Assessment  │
              │  - Authority ratio     │
              │  - Tempo score         │
              │  - Threat level        │
              └───────────┬────────────┘
                          │
              ┌───────────┴───────────┐
              ▼                       ▼
    ┌─────────────────┐     ┌─────────────────┐
    │ Strategy Match  │     │ Motivation Fit  │
    │ (Situation fit) │     │ (Personality)   │
    └────────┬────────┘     └────────┬────────┘
             │                       │
             └───────────┬───────────┘
                         ▼
              ┌─────────────────────┐
              │ Selected Strategy   │
              │ (with confidence)   │
              └──────────┬──────────┘
                         │
                         ▼
              ┌─────────────────────┐
              │ Option Scoring      │
              │ Strategy-adjusted   │
              └─────────────────────┘
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-004-character-motivation-model.md
- 8-007-tree-pruning-evaluation.md

## Dependencies
- 8-003: Option Generation System (options to score)
- 8-004: Character Motivation Model (personality influence)
- 8-005: Movement Interaction Analyzer (situation data)

## Acceptance Criteria
- [ ] Situation assessment accurately evaluates game state
- [ ] Strategy profiles define distinct approaches
- [ ] Selection matches strategy to situation and personality
- [ ] Transitions occur at appropriate moments
- [ ] Strategy blending creates hybrid approaches
- [ ] Action scoring reflects chosen strategy

## Notes
The orchestrator is the "brains" of the AI - it doesn't decide individual
actions but sets the strategic direction. Think of it as choosing which
"playbook" to use, while other components execute the plays.
