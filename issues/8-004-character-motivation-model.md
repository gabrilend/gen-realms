# 8-004: Character Motivation Model

## Current Behavior
No system exists for defining AI "personalities" or motivation-driven behavior.

## Intended Behavior
A motivation model that defines AI personalities through weighted vectors,
influencing how options are evaluated and strategies selected. Motivations
create diverse, thematically-appropriate AI opponents.

## Suggested Implementation Steps

1. Create `src/ai/04-motivation.h` with motivation types:
   ```c
   typedef struct MotivationModel MotivationModel;
   typedef struct MotivationVector MotivationVector;

   /* Core motivation dimensions */
   typedef enum {
       MOTIV_AGGRESSION,    /* Desire to deal damage */
       MOTIV_ACCUMULATION,  /* Desire for resources/cards */
       MOTIV_PRESERVATION,  /* Desire to protect authority */
       MOTIV_DISRUPTION,    /* Desire to interfere with opponent */
       MOTIV_TEMPO,         /* Desire for board presence */
       MOTIV_SYNERGY,       /* Desire to build combos */
       MOTIV_RISK,          /* Willingness to gamble */
       MOTIV_PATIENCE,      /* Long-term vs short-term focus */
       MOTIV_COUNT
   } MotivationType;

   /* Motivation vector - personality definition */
   struct MotivationVector {
       float weights[MOTIV_COUNT];  /* 0.0 to 1.0 each */
       float volatility;            /* How much mood swings */
   };

   /* Dynamic motivation state */
   struct MotivationModel {
       MotivationVector base;       /* Personality baseline */
       MotivationVector current;    /* Current state (can drift) */
       MotivationVector triggers;   /* Event-based adjustments */

       /* Emotional state */
       float frustration;           /* Builds when plans fail */
       float confidence;            /* Rises with success */
       float desperation;           /* Rises when losing badly */
   };
   ```

2. Create `src/ai/04-motivation.c` implementing:
   - Motivation vector operations
   - State transitions
   - Evaluation integration

3. Implement preset personalities:
   ```c
   /* Built-in AI personalities */
   typedef enum {
       PERSONALITY_BERSERKER,    /* Max aggression */
       PERSONALITY_MERCHANT,     /* Max accumulation */
       PERSONALITY_GUARDIAN,     /* Max preservation */
       PERSONALITY_TRICKSTER,    /* Max disruption */
       PERSONALITY_BALANCED,     /* Even weights */
       PERSONALITY_ADAPTIVE,     /* Shifts based on state */
       PERSONALITY_CHAOTIC,      /* Random/unpredictable */
       PERSONALITY_COUNT
   } AIPersonality;

   MotivationModel* motivation_create_preset(AIPersonality preset);
   ```

4. Implement motivation-based evaluation:
   ```c
   /* Score an option based on motivations */
   float motivation_score_option(MotivationModel* model,
                                  GeneratedOption* option,
                                  Game* state);

   /* Score a strategy tree node */
   float motivation_score_node(MotivationModel* model,
                                StrategyNode* node);
   ```

5. Implement emotional dynamics:
   ```c
   /* Update motivation based on game events */
   void motivation_on_damage_dealt(MotivationModel* model, int amount);
   void motivation_on_damage_taken(MotivationModel* model, int amount);
   void motivation_on_card_bought(MotivationModel* model, CardType* card);
   void motivation_on_plan_failed(MotivationModel* model);
   void motivation_on_plan_succeeded(MotivationModel* model);

   /* Decay towards baseline over time */
   void motivation_tick(MotivationModel* model);
   ```

6. Add faction affinity:
   ```c
   /* Certain personalities favor certain factions */
   float motivation_faction_affinity(MotivationModel* model, Faction faction);
   ```

## Personality Profiles

| Personality | Key Traits | Play Style |
|-------------|------------|------------|
| BERSERKER | High aggro, high risk | Rushes damage, ignores defense |
| MERCHANT | High accum, patience | Builds economy, late game power |
| GUARDIAN | High preserv, low risk | Defensive, authority gain focus |
| TRICKSTER | High disrupt, high tempo | Denies opponent, opportunistic |
| BALANCED | Even weights | Adapts to situation |
| ADAPTIVE | Dynamic weights | Shifts personality mid-game |
| CHAOTIC | Random shifts | Unpredictable, makes mistakes |

## Motivation Flow

```
Base Personality
       │
       ▼
┌─────────────────┐
│ Current State   │◄─── Game Events
│ - Frustration   │     - Damage dealt/taken
│ - Confidence    │     - Cards bought
│ - Desperation   │     - Plans success/fail
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Effective       │
│ Motivation      │──── Used for scoring
│ Vector          │     options/strategies
└─────────────────┘
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-003-option-generation-system.md
- 8-006-strategy-orchestrator.md

## Dependencies
- 8-003: Option Generation System (option categories)

## Acceptance Criteria
- [ ] Motivation vectors define AI personality
- [ ] Preset personalities have distinct behaviors
- [ ] Emotional state influences decisions
- [ ] Game events update motivation state
- [ ] Motivation scoring integrates with option evaluation
- [ ] Faction affinity creates thematic preferences

## Notes
Motivation models create the "why" behind AI decisions. Two AIs with the
same strategic knowledge but different motivations will play very differently.
This is crucial for creating interesting, varied opponents.
