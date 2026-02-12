# 8-005: Movement Interaction Analyzer

## Current Behavior
No system exists for tracking how cards interact and influence strategic choices.

## Intended Behavior
An analyzer that tracks card interactions and "movement patterns" - how pieces
flow through the game space - to inform strategic choices. Identifies synergies,
threats, and opportunity costs based on observed and predicted interactions.

## Suggested Implementation Steps

1. Create `src/ai/05-interaction.h` with interaction types:
   ```c
   typedef struct InteractionAnalyzer InteractionAnalyzer;
   typedef struct MovementPattern MovementPattern;
   typedef struct Interaction Interaction;

   /* Types of card interactions */
   typedef enum {
       INTERACT_SYNERGY,      /* Cards enhance each other */
       INTERACT_ANTI_SYNERGY, /* Cards work against each other */
       INTERACT_ENABLES,      /* Card A enables card B */
       INTERACT_COUNTERS,     /* Card A counters card B */
       INTERACT_COMPETES,     /* Cards compete for resources */
       INTERACT_CHAINS        /* Cards trigger in sequence */
   } InteractionType;

   /* Movement pattern - how cards flow through game */
   typedef enum {
       MOVE_DECK_TO_HAND,     /* Draw */
       MOVE_HAND_TO_PLAY,     /* Play */
       MOVE_PLAY_TO_DISCARD,  /* End of turn */
       MOVE_DISCARD_TO_DECK,  /* Shuffle */
       MOVE_TRADE_TO_DISCARD, /* Purchase */
       MOVE_HAND_TO_TOP_DECK, /* Top-deck effect */
       MOVE_DISCARD_TO_HAND,  /* Retrieval effect */
       MOVE_ANY_TO_SCRAP      /* Scrapped */
   } MovementType;

   /* Observed interaction record */
   struct Interaction {
       CardType* card_a;
       CardType* card_b;
       InteractionType type;
       float strength;         /* How impactful */
       int observation_count;  /* Times observed */
   };

   /* Movement pattern tracking */
   struct MovementPattern {
       MovementType movement;
       CardType* card;
       int turn_number;
       int sequence_position;  /* Order within turn */
       float value_generated;  /* Resources/damage from this */
   };
   ```

2. Create `src/ai/05-interaction.c` implementing:
   - Interaction detection
   - Pattern recording
   - Synergy scoring

3. Implement interaction detection:
   ```c
   /* Create analyzer */
   InteractionAnalyzer* interaction_create(void);
   void interaction_destroy(InteractionAnalyzer* ia);

   /* Record observed interactions */
   void interaction_observe_play(InteractionAnalyzer* ia,
                                  CardInstance* card,
                                  Game* state);
   void interaction_observe_effect(InteractionAnalyzer* ia,
                                    Effect* effect,
                                    CardInstance* source,
                                    Game* state);

   /* Query interactions */
   float interaction_synergy_score(InteractionAnalyzer* ia,
                                    CardType* a, CardType* b);
   Interaction* interaction_find_synergies(InteractionAnalyzer* ia,
                                            CardType* card,
                                            int* out_count);
   ```

4. Implement pattern analysis:
   ```c
   /* Record movement pattern */
   void interaction_record_movement(InteractionAnalyzer* ia,
                                     MovementType type,
                                     CardInstance* card,
                                     Game* state);

   /* Analyze deck flow */
   typedef struct {
       float draw_rate;        /* Cards drawn per turn */
       float cycle_speed;      /* Turns to see full deck */
       float scrap_rate;       /* Cards scrapped per turn */
       float upgrade_density;  /* Upgraded cards as % */
   } DeckFlowMetrics;

   DeckFlowMetrics interaction_analyze_flow(InteractionAnalyzer* ia,
                                             int player_id);
   ```

5. Implement threat detection:
   ```c
   /* Identify threatening patterns */
   typedef struct {
       CardType* threat_card;
       float threat_level;
       int turns_until_threat;  /* Estimated */
       const char* description;
   } ThreatAssessment;

   ThreatAssessment* interaction_assess_threats(InteractionAnalyzer* ia,
                                                 Game* state,
                                                 int* out_count);
   ```

6. Implement opportunity cost analysis:
   ```c
   /* What are we giving up by taking this action? */
   float interaction_opportunity_cost(InteractionAnalyzer* ia,
                                       AIAction* action,
                                       Game* state);
   ```

## Interaction Matrix Example

```
              │ Wolf Pack │ Trade Hub │ Knight │ Scrap Bot │
──────────────┼───────────┼───────────┼────────┼───────────┤
Wolf Pack     │    -      │    0.0    │  -0.2  │   -0.3    │
Trade Hub     │   0.0     │    -      │   0.2  │    0.5    │
Knight        │  -0.2     │   0.2     │    -   │    0.0    │
Scrap Bot     │  -0.3     │   0.5     │   0.0  │     -     │

Positive = synergy, Negative = anti-synergy
```

## Movement Flow Diagram

```
        ┌──────────────────────────────────────┐
        │                                      │
        ▼                                      │
    ┌───────┐    draw    ┌───────┐   play   ┌─────┐
    │ Deck  │──────────▶│ Hand  │─────────▶│Play │
    └───┬───┘           └───┬───┘          └──┬──┘
        │                   │                  │
        │ shuffle           │ scrap            │ end turn
        │                   ▼                  │
    ┌───┴────┐         ┌────────┐             │
    │Discard │◀────────│ Scrap  │             │
    └────────┘         └────────┘             │
        ▲                                     │
        │              discard                │
        └─────────────────────────────────────┘
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-003-option-generation-system.md
- 8-006-strategy-orchestrator.md

## Dependencies
- 8-001: AI Runner Infrastructure (action observation)
- 8-003: Option Generation System (synergy integration)

## Acceptance Criteria
- [ ] Interactions tracked between card pairs
- [ ] Movement patterns recorded and analyzed
- [ ] Synergy scores influence option generation
- [ ] Threat assessment identifies opponent dangers
- [ ] Opportunity cost helps avoid suboptimal plays
- [ ] Deck flow metrics available for strategy decisions

## Notes
The interaction analyzer provides the AI with "game sense" - understanding
how cards work together and against each other. This is especially important
for evaluating card purchases and building coherent deck strategies.
