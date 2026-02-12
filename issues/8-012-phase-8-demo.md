# 8-012: Phase 8 Demo

## Current Behavior
No demonstration exists showcasing the AI gameplay system.

## Intended Behavior
A comprehensive demonstration that showcases all Phase 8 AI capabilities:
- AI vs AI exhibition match with strategy visualization
- Human vs AI with adjustable difficulty
- Strategy tree visualization
- Real-time decision commentary
- Performance statistics display

## Suggested Implementation Steps

1. Create `src/demo/phase-8-demo.c` with demo framework:
   ```c
   /* Demo modes */
   typedef enum {
       DEMO_AI_VS_AI,        /* Two AIs play, human watches */
       DEMO_HUMAN_VS_AI,     /* Human plays against AI */
       DEMO_ANALYSIS,        /* Step through AI decision process */
       DEMO_TOURNAMENT       /* Multiple AI personalities compete */
   } DemoMode;

   /* Demo configuration */
   typedef struct {
       DemoMode mode;
       DifficultyLevel ai_difficulty;
       AIPersonality ai_personality;
       bool show_thinking;      /* Display AI thought process */
       bool show_tree;          /* Visualize strategy tree */
       bool pause_between_turns;
       int think_delay_ms;      /* Artificial delay for readability */
   } DemoConfig;
   ```

2. Implement AI vs AI mode:
   - Two AI players with different personalities
   - Real-time game display
   - Strategy annotations
   - Turn-by-turn commentary

3. Implement Human vs AI mode:
   - Difficulty selection
   - Personality selection
   - Hint system (optional)
   - Post-game analysis

4. Implement strategy visualization:
   ```c
   /* Strategy tree display (text-based) */
   void demo_print_tree(StrategyTree* tree, int max_depth);

   /* Current strategy display */
   void demo_print_strategy(StrategyProfile* profile);

   /* Decision breakdown */
   void demo_print_decision(SearchResult* result,
                             GeneratedOption* options,
                             int option_count);
   ```

5. Implement commentary system:
   ```c
   /* Generate commentary for AI action */
   const char* demo_generate_comment(AIAction* action,
                                       StrategyProfile* strategy,
                                       SituationAssessment* sit);

   /* Explain why AI chose this action */
   const char* demo_explain_choice(SearchResult* result,
                                     GeneratedOption* alternatives,
                                     int alt_count);
   ```

6. Implement statistics display:
   ```c
   /* Performance stats */
   void demo_print_perf_stats(PerfStats* stats);

   /* Learning stats */
   void demo_print_learning_stats(LearningSystem* ls);

   /* Match statistics */
   void demo_print_match_stats(int games_played,
                                int ai1_wins,
                                int ai2_wins);
   ```

7. Create `run-phase8-demo.sh`:
   ```bash
   #!/bin/bash
   # Phase 8 Demo - AI Gameplay System

   echo "Symbeline Realms - Phase 8 AI Demo"
   echo "==================================="
   echo ""
   echo "Demo modes:"
   echo "  1. AI vs AI Exhibition"
   echo "  2. Human vs AI"
   echo "  3. AI Decision Analysis"
   echo "  4. AI Tournament"
   echo ""
   read -p "Select mode (1-4): " mode

   ./bin/phase-8-demo --mode=$mode
   ```

## Demo Output Examples

### AI vs AI Mode
```
╔════════════════════════════════════════════════════════════════╗
║              AI vs AI Exhibition Match                          ║
║  BERSERKER (aggressive) vs MERCHANT (economic)                  ║
╠════════════════════════════════════════════════════════════════╣

Turn 5 | BERSERKER's turn
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Strategy: RUSH (aggression=0.9, risk=0.7)
Situation: Behind on economy, ahead on damage

Thinking... (247 nodes, 3 depth, 89ms)

[Decision Tree]
  ► Attack Player (score: 0.72) ◄ CHOSEN
    └─ Expected damage: 7
  • Play Dire Wolf (score: 0.68)
    └─ Build board presence
  • Buy Trade Ship (score: 0.41)
    └─ Rejected: conflicts with rush strategy

Commentary: "BERSERKER ignores the tempting Trade Ship,
staying true to its aggressive nature. With authority
at 38 vs 45, it needs to close the gap through damage."

Action: Attack Player 2 for 7 damage
Player 2 authority: 45 → 38

[Press Enter for next turn]
```

### Human vs AI Mode
```
╔════════════════════════════════════════════════════════════════╗
║                    Human vs AI                                  ║
║  Difficulty: NORMAL | AI: GUARDIAN (defensive)                  ║
╠════════════════════════════════════════════════════════════════╣

Your Turn | Authority: 42 vs AI: 48

Your Hand:
  [0] Wolf Scout (+2C)
  [1] Trade Caravan (+3T)
  [2] Village Guard (+1A, +1C)

Trade Row:
  [0] Battle Golem (3g) - Artificer
  [1] Healer's Sanctum (4g) - Kingdom
  [2] Dire Wolf (4g) - Wilds

Current resources: Trade 0 | Combat 0

> h (hint)
Hint: Playing Trade Caravan first maximizes your buying
power. Consider saving combat for their base.

> p 1
Played Trade Caravan (+3T)

> b 1
Bought Healer's Sanctum (4g)

[AI thinking...]
AI chose: Deploy Fortress (defense focus, protecting lead)
```

### Performance Stats
```
═══════════════════════════════════════════════════════════════
                    Performance Statistics
═══════════════════════════════════════════════════════════════
Average think time:    312ms
Max think time:        847ms
Nodes searched/turn:   1,247
Average depth:         4.2
Cache hit rate:        67%
Prune rate:            43%
Memory usage:          42MB
═══════════════════════════════════════════════════════════════
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- All Phase 8 issues (8-001 through 8-011)
- issues/completed/1-013-phase-1-demo.md (demo patterns)

## Dependencies
- All Phase 8 issues must be complete
- Phase 1: Core Engine (gameplay)
- Phase 4: Card Content (card data)

## Acceptance Criteria
- [ ] AI vs AI mode runs complete games
- [ ] Human vs AI mode is playable at all difficulties
- [ ] Strategy visualization clearly shows AI reasoning
- [ ] Commentary explains AI decisions meaningfully
- [ ] Performance statistics are accurate and informative
- [ ] Demo runs smoothly without performance issues
- [ ] All AI personalities demonstrate distinct behavior

## Notes
The Phase 8 demo should make the AI's "thinking" visible and understandable.
Players should come away understanding not just that the AI plays well,
but why it makes the choices it does. This transparency builds trust and
makes the AI feel like a worthy opponent rather than a black box.
