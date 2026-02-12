# 8-002: Strategy Tree Data Structure

## Current Behavior
No data structure exists for representing branching strategic options.

## Intended Behavior
A fractal tree-like data structure that represents the space of possible
strategic choices, where:
- Nodes contain game states and available actions
- Edges represent state transitions from taking actions
- Subtrees can be dynamically generated at runtime
- The structure exhibits self-similarity (fractal property)

## Suggested Implementation Steps

1. Create `src/ai/02-strategy-tree.h` with core types:
   ```c
   typedef struct StrategyNode StrategyNode;
   typedef struct StrategyEdge StrategyEdge;
   typedef struct StrategyTree StrategyTree;

   /* Node evaluation data */
   typedef struct {
       float value;           /* Heuristic evaluation */
       float confidence;      /* How certain we are */
       int visit_count;       /* For MCTS-style updates */
       float motivation_fit;  /* How well it matches AI personality */
   } NodeEval;

   /* Strategy node - a point in decision space */
   struct StrategyNode {
       uint32_t id;
       Game* state_snapshot;  /* NULL if not materialized */
       uint64_t state_hash;   /* For transposition detection */

       NodeEval eval;

       StrategyEdge* edges;   /* Outgoing actions */
       int edge_count;
       int edge_capacity;

       StrategyNode* parent;
       int depth;

       /* Fractal metadata */
       uint32_t pattern_id;   /* Identifies similar subtree patterns */
       bool is_terminal;      /* Win/loss/draw state */
   };

   /* Edge - an action connecting nodes */
   struct StrategyEdge {
       AIAction action;
       StrategyNode* target;  /* NULL if unexpanded */
       float prior_prob;      /* Initial action probability */
   };
   ```

2. Create `src/ai/02-strategy-tree.c` implementing:
   - Node creation and memory pooling
   - Edge management
   - Tree traversal utilities

3. Implement tree operations:
   ```c
   /* Tree lifecycle */
   StrategyTree* strategy_tree_create(Game* root_state);
   void strategy_tree_destroy(StrategyTree* tree);

   /* Node operations */
   StrategyNode* strategy_node_expand(StrategyTree* tree,
                                       StrategyNode* node,
                                       AIAction* action);
   void strategy_node_prune(StrategyTree* tree, StrategyNode* node);

   /* Traversal */
   StrategyNode* strategy_tree_select_child(StrategyNode* node,
                                             SelectionPolicy policy);
   void strategy_tree_backpropagate(StrategyNode* leaf, float value);
   ```

4. Implement transposition table:
   ```c
   /* Detect equivalent positions reached by different paths */
   StrategyNode* strategy_tree_find_transposition(StrategyTree* tree,
                                                   uint64_t state_hash);
   ```

5. Implement pattern recognition for fractal property:
   ```c
   /* Identify similar subtree structures */
   uint32_t strategy_node_compute_pattern(StrategyNode* node);
   StrategyNode* strategy_tree_find_similar(StrategyTree* tree,
                                             uint32_t pattern_id);
   ```

6. Add memory management:
   ```c
   /* Memory pooling for nodes */
   void strategy_tree_set_pool_size(StrategyTree* tree, size_t max_nodes);
   void strategy_tree_gc(StrategyTree* tree); /* Garbage collect old nodes */
   ```

## Data Flow

```
Root Node (current game state)
    │
    ├── Edge: Play Card 0
    │   └── Node: State after play
    │       ├── Edge: Buy Card 1
    │       │   └── Node: ...
    │       └── Edge: Attack
    │           └── Node: ...
    │
    ├── Edge: Play Card 1
    │   └── Node: State after play
    │       └── ...
    │
    └── Edge: End Turn
        └── Node: Opponent's turn
            └── (opponent subtree - same structure)
```

## Fractal Property

The tree exhibits self-similarity because:
1. The same decision patterns (play/buy/attack) repeat at each depth
2. Similar board positions create similar subtrees
3. Pattern IDs allow reusing evaluation from similar situations
4. Learning at one level transfers to other levels

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-001-ai-runner-infrastructure.md
- 8-003-option-generation-system.md

## Dependencies
- 8-001: AI Runner Infrastructure (action types)

## Acceptance Criteria
- [ ] Nodes store game state snapshots or hashes
- [ ] Edges connect nodes with actions
- [ ] Tree can be expanded dynamically
- [ ] Transposition detection finds equivalent states
- [ ] Pattern recognition identifies similar structures
- [ ] Memory pooling prevents allocation overhead
- [ ] Tree can be pruned to stay within memory budget

## Notes
The fractal property is key to scaling - patterns learned in simple situations
inform decisions in complex ones. The pattern_id system enables this transfer.
