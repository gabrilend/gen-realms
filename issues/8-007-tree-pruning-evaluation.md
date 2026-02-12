# 8-007: Tree Pruning and Evaluation

## Current Behavior
No system exists for efficiently traversing and evaluating strategy trees.

## Intended Behavior
An evaluation system that efficiently searches the strategy tree using:
- Heuristic evaluation functions for leaf/intermediate nodes
- Alpha-beta style pruning to cut unproductive branches
- Monte Carlo sampling for uncertain situations
- Budget-aware search depth management

## Suggested Implementation Steps

1. Create `src/ai/07-eval.h` with evaluation types:
   ```c
   typedef struct Evaluator Evaluator;
   typedef struct EvalConfig EvalConfig;
   typedef struct SearchResult SearchResult;

   /* Evaluation configuration */
   struct EvalConfig {
       int max_depth;            /* Maximum tree depth */
       int time_budget_ms;       /* Computation time limit */
       int node_budget;          /* Maximum nodes to explore */

       /* Pruning thresholds */
       float alpha_beta_window;  /* Initial aspiration window */
       float futility_margin;    /* Prune clearly losing moves */
       float null_move_reduction; /* Depth reduction for null move */

       /* Heuristic weights */
       float weight_authority;
       float weight_tempo;
       float weight_economy;
       float weight_disruption;
   };

   /* Search result */
   struct SearchResult {
       AIAction best_action;
       float score;
       float confidence;
       int nodes_searched;
       int depth_reached;
       const char* principal_variation; /* Best line found */
   };
   ```

2. Create `src/ai/07-eval.c` implementing:
   - Heuristic evaluation
   - Tree search algorithms
   - Pruning strategies

3. Implement heuristic evaluation:
   ```c
   /* Create evaluator */
   Evaluator* evaluator_create(EvalConfig* config);
   void evaluator_destroy(Evaluator* eval);

   /* Static position evaluation */
   float evaluator_heuristic(Evaluator* eval,
                              Game* state,
                              int player_id,
                              MotivationModel* motivation);

   /* Component evaluations */
   float eval_authority_component(Game* state, int player_id);
   float eval_tempo_component(Game* state, int player_id);
   float eval_economy_component(Game* state, int player_id);
   float eval_threat_component(Game* state, int player_id);
   ```

4. Implement minimax with alpha-beta:
   ```c
   /* Core search function */
   SearchResult evaluator_search(Evaluator* eval,
                                  StrategyTree* tree,
                                  StrategyOrchestrator* orch,
                                  MotivationModel* motivation);

   /* Alpha-beta search */
   float evaluator_alphabeta(Evaluator* eval,
                              StrategyNode* node,
                              int depth,
                              float alpha,
                              float beta,
                              bool maximizing);
   ```

5. Implement advanced pruning:
   ```c
   /* Futility pruning - skip clearly bad moves */
   bool evaluator_futility_prune(Evaluator* eval,
                                  StrategyNode* node,
                                  float alpha,
                                  int depth);

   /* Late move reduction - search later moves shallowly */
   int evaluator_late_move_reduction(Evaluator* eval,
                                       StrategyNode* node,
                                       int move_index,
                                       int base_depth);

   /* Null move pruning - test if doing nothing is good enough */
   float evaluator_null_move(Evaluator* eval,
                              StrategyNode* node,
                              int depth,
                              float beta);
   ```

6. Implement Monte Carlo sampling:
   ```c
   /* Random playout for evaluation */
   float evaluator_monte_carlo(Evaluator* eval,
                                Game* state,
                                int player_id,
                                int num_simulations);

   /* Hybrid MCTS + heuristic */
   SearchResult evaluator_mcts_search(Evaluator* eval,
                                        StrategyTree* tree,
                                        int iterations);
   ```

7. Implement budget management:
   ```c
   /* Time management */
   void evaluator_start_search(Evaluator* eval);
   bool evaluator_time_remaining(Evaluator* eval);
   void evaluator_adjust_depth(Evaluator* eval); /* Iterative deepening */
   ```

## Evaluation Components

| Component | What it measures | Score range |
|-----------|------------------|-------------|
| Authority | Life total ratio | -1.0 to +1.0 |
| Tempo | Board presence, resources | -1.0 to +1.0 |
| Economy | Deck quality, card advantage | -1.0 to +1.0 |
| Threat | Opponent danger level | -1.0 to 0.0 |

## Search Visualization

```
                Root (current state)
                     score: 0.35
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
     Play Card 0    Buy Card 2      Attack
     score: 0.42    score: 0.28    score: 0.51 ◄── Best
          │              │              │
     ┌────┴────┐    [pruned]      ┌────┴────┐
     ▼         ▼                  ▼         ▼
  (depth 2)  (depth 2)        Counter    No Counter
  score:0.38 score:0.45      score:0.48  score:0.51
                                  │
                             [continue search...]
```

## Pruning Strategies

1. **Alpha-Beta**: Skip branches that can't affect the result
2. **Futility**: Skip moves that are clearly losing
3. **Late Move Reduction**: Search unpromising moves shallowly
4. **Null Move**: Test if passing is acceptable (usually not in this game)
5. **Transposition**: Reuse evaluation from equivalent positions

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-002-strategy-tree-data-structure.md
- 8-006-strategy-orchestrator.md

## Dependencies
- 8-002: Strategy Tree Data Structure (tree traversal)
- 8-006: Strategy Orchestrator (strategy-aware evaluation)

## Acceptance Criteria
- [ ] Heuristic evaluation captures key game factors
- [ ] Alpha-beta pruning reduces search space
- [ ] Advanced pruning techniques work correctly
- [ ] Monte Carlo provides evaluation for uncertain positions
- [ ] Time budget respected without early termination issues
- [ ] Iterative deepening maximizes depth within budget

## Notes
Evaluation quality determines AI playing strength. The heuristic must balance:
- Accuracy (correctly ordering moves)
- Speed (fast enough for deep search)
- Stability (similar positions get similar scores)
