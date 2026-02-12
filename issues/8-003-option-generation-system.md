# 8-003: Option Generation System

## Current Behavior
No system exists for dynamically generating strategic options at runtime.

## Intended Behavior
A system that generates strategy tree branches based on current game state,
creating an eternally evolving options list that adapts as the game progresses.
Options are generated lazily - only expanded when the AI considers them.

## Suggested Implementation Steps

1. Create `src/ai/03-option-gen.h` with option types:
   ```c
   typedef struct OptionGenerator OptionGenerator;
   typedef struct OptionContext OptionContext;

   /* Categories of strategic options */
   typedef enum {
       OPT_CAT_TEMPO,       /* Immediate board impact */
       OPT_CAT_ECONOMY,     /* Trade/resource building */
       OPT_CAT_DAMAGE,      /* Authority pressure */
       OPT_CAT_DEFENSE,     /* Authority/base protection */
       OPT_CAT_DISRUPTION,  /* Opponent interference */
       OPT_CAT_SETUP        /* Future turn preparation */
   } OptionCategory;

   /* Generated option with metadata */
   typedef struct {
       AIAction action;
       OptionCategory category;
       float urgency;        /* Time-sensitive importance */
       float synergy_score;  /* How well it combines with other options */
       const char* rationale; /* Human-readable explanation */
   } GeneratedOption;

   /* Generator interface */
   OptionGenerator* option_gen_create(void);
   void option_gen_destroy(OptionGenerator* gen);

   /* Generate options for a game state */
   int option_gen_generate(OptionGenerator* gen,
                           Game* state,
                           int player_id,
                           OptionContext* ctx,
                           GeneratedOption** out_options);
   ```

2. Create `src/ai/03-option-gen.c` implementing option generators:
   - `generate_tempo_options()` - immediate impact plays
   - `generate_economy_options()` - trade row purchases
   - `generate_damage_options()` - attack sequences
   - `generate_defense_options()` - protective plays
   - `generate_disruption_options()` - discard/scrap effects
   - `generate_setup_options()` - deck positioning

3. Implement context-aware generation:
   ```c
   /* Context influences which options are generated */
   struct OptionContext {
       float aggression_weight;
       float economy_weight;
       float defense_weight;
       float disruption_weight;

       int turns_remaining_estimate;
       float authority_pressure;  /* How threatened we are */
       float tempo_advantage;     /* Board control measure */

       /* From opponent model */
       float opponent_aggression;
       float opponent_threat_level;
   };
   ```

4. Implement synergy detection:
   ```c
   /* Calculate how well options work together */
   float option_gen_synergy(GeneratedOption* opt_a,
                            GeneratedOption* opt_b,
                            Game* state);

   /* Find option combinations */
   int option_gen_find_combos(OptionGenerator* gen,
                              GeneratedOption* options,
                              int count,
                              GeneratedOption*** out_combos);
   ```

5. Implement urgency calculation:
   ```c
   /* Time-sensitive option scoring */
   float option_gen_urgency(GeneratedOption* opt,
                            Game* state,
                            int player_id);
   ```

6. Add rationale generation (for debugging/explanation):
   ```c
   /* Generate human-readable explanation */
   void option_gen_explain(GeneratedOption* opt,
                           char* buffer,
                           size_t size);
   ```

## Option Categories Explained

| Category | Description | Example |
|----------|-------------|---------|
| TEMPO | Immediate board presence | Play high-impact ship |
| ECONOMY | Resource accumulation | Buy expensive card |
| DAMAGE | Authority pressure | Attack opponent |
| DEFENSE | Survival focus | Deploy defensive base |
| DISRUPTION | Opponent interference | Force discard |
| SETUP | Future positioning | Top-deck good card |

## Generation Flow

```
Game State
    │
    ▼
┌───────────────────────┐
│  Context Analysis     │
│  - Authority levels   │
│  - Tempo assessment   │
│  - Threat detection   │
└───────────┬───────────┘
            │
            ▼
┌───────────────────────┐
│  Category Generators  │
│  ┌─────┐ ┌─────────┐  │
│  │Tempo│ │Economy  │  │
│  └─────┘ └─────────┘  │
│  ┌──────┐ ┌────────┐  │
│  │Damage│ │Defense │  │
│  └──────┘ └────────┘  │
└───────────┬───────────┘
            │
            ▼
┌───────────────────────┐
│  Synergy Analysis     │
│  - Combo detection    │
│  - Urgency scoring    │
└───────────┬───────────┘
            │
            ▼
     Generated Options
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-002-strategy-tree-data-structure.md
- 8-004-character-motivation-model.md

## Dependencies
- 8-001: AI Runner Infrastructure (action enumeration)
- 8-002: Strategy Tree Data Structure (option storage)

## Acceptance Criteria
- [ ] Options generated for each category
- [ ] Context influences option generation
- [ ] Synergy scores identify good combinations
- [ ] Urgency reflects time-sensitivity
- [ ] Rationale provides human-readable explanations
- [ ] Generation is efficient (lazy expansion)

## Notes
The option generator creates the "raw material" that the strategy tree
organizes and the orchestrator selects from. It bridges the gap between
low-level actions and high-level strategic thinking.
