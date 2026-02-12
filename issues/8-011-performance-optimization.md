# 8-011: Performance Optimization

## Current Behavior
AI system has no specific performance optimizations.

## Intended Behavior
Optimized AI execution through caching, parallelization, incremental tree
updates, and budget-based computation limiting to ensure real-time play
without noticeable delays.

## Suggested Implementation Steps

1. Create `src/ai/11-perf.h` with performance types:
   ```c
   typedef struct PerfManager PerfManager;
   typedef struct PerfStats PerfStats;
   typedef struct TreeCache TreeCache;

   /* Performance statistics */
   struct PerfStats {
       /* Timing */
       double avg_think_time_ms;
       double max_think_time_ms;
       double min_think_time_ms;

       /* Search efficiency */
       int avg_nodes_searched;
       int avg_depth_reached;
       float cache_hit_rate;
       float prune_rate;

       /* Memory */
       size_t tree_memory_used;
       size_t cache_memory_used;
       int live_node_count;
   };

   /* Cache configuration */
   typedef struct {
       size_t max_size_bytes;
       int max_entries;
       float eviction_threshold;  /* When to start evicting */
   } CacheConfig;
   ```

2. Create `src/ai/11-perf.c` implementing:
   - Caching system
   - Parallel search
   - Memory management

3. Implement evaluation cache:
   ```c
   /* Create performance manager */
   PerfManager* perf_create(void);
   void perf_destroy(PerfManager* pm);

   /* Evaluation cache */
   TreeCache* perf_create_cache(PerfManager* pm, CacheConfig* config);
   void perf_cache_store(TreeCache* cache, uint64_t hash, float eval);
   bool perf_cache_lookup(TreeCache* cache, uint64_t hash, float* out_eval);
   void perf_cache_clear(TreeCache* cache);
   ```

4. Implement parallel search:
   ```c
   /* Thread pool for parallel evaluation */
   typedef struct ThreadPool ThreadPool;

   ThreadPool* perf_create_thread_pool(PerfManager* pm, int num_threads);
   void perf_destroy_thread_pool(ThreadPool* pool);

   /* Parallel tree search */
   typedef struct {
       StrategyNode* root;
       int depth;
       float alpha;
       float beta;
   } SearchTask;

   void perf_parallel_search(ThreadPool* pool,
                              SearchTask* tasks,
                              int task_count,
                              float* results);

   /* Lazy SMP (simplified parallel search) */
   SearchResult perf_lazy_smp(ThreadPool* pool,
                               Evaluator* eval,
                               StrategyTree* tree,
                               int num_threads);
   ```

5. Implement incremental updates:
   ```c
   /* Incremental tree maintenance */
   void perf_tree_prune_old(StrategyTree* tree, StrategyNode* new_root);
   void perf_tree_reuse_subtree(StrategyTree* tree,
                                  StrategyNode* old_root,
                                  AIAction* taken_action);

   /* Incremental evaluation updates */
   void perf_update_eval_incremental(StrategyNode* node,
                                       AIAction* action,
                                       float delta);
   ```

6. Implement budget management:
   ```c
   /* Computation budget */
   typedef struct {
       int time_budget_ms;
       int node_budget;
       int depth_budget;

       /* Adaptive budgeting */
       bool adaptive;
       float urgency_multiplier;  /* More time when critical */
   } ComputeBudget;

   void perf_set_budget(PerfManager* pm, ComputeBudget* budget);
   bool perf_budget_remaining(PerfManager* pm);
   void perf_budget_start(PerfManager* pm);

   /* Iterative deepening with time management */
   SearchResult perf_iterative_deepening(Evaluator* eval,
                                           StrategyTree* tree,
                                           ComputeBudget* budget);
   ```

7. Implement memory pooling:
   ```c
   /* Node memory pool */
   typedef struct NodePool NodePool;

   NodePool* perf_create_node_pool(size_t initial_size);
   StrategyNode* perf_pool_alloc(NodePool* pool);
   void perf_pool_free(NodePool* pool, StrategyNode* node);
   void perf_pool_reset(NodePool* pool);  /* Mass free */

   /* Statistics */
   size_t perf_pool_used(NodePool* pool);
   size_t perf_pool_capacity(NodePool* pool);
   ```

## Performance Targets

| Metric | Target | Acceptable |
|--------|--------|------------|
| Think time (normal) | <500ms | <1000ms |
| Think time (hard) | <1000ms | <2000ms |
| Cache hit rate | >60% | >40% |
| Memory usage | <100MB | <200MB |
| Parallel speedup | 2-3x | 1.5x |

## Caching Strategy

```
┌─────────────────────────────────────────────────────────────┐
│                      Evaluation Cache                        │
├─────────────────────────────────────────────────────────────┤
│  Key: Zobrist hash of game state                            │
│  Value: (evaluation, depth, flag, best_move)                │
│                                                              │
│  Replacement: LRU with depth preference                     │
│  (keep deeper evaluations longer)                           │
└─────────────────────────────────────────────────────────────┘

Cache lookup flow:
1. Compute hash of current state
2. Check cache for entry
3. If found and depth >= needed:
   - Use cached evaluation
   - Return cached best move
4. If not found or shallow:
   - Compute fresh evaluation
   - Store in cache
```

## Parallel Search Architecture

```
                    Main Thread
                         │
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
    ┌─────────┐    ┌─────────┐    ┌─────────┐
    │Thread 1 │    │Thread 2 │    │Thread 3 │
    │Search A │    │Search B │    │Search C │
    └────┬────┘    └────┬────┘    └────┬────┘
         │              │              │
         └──────────────┼──────────────┘
                        ▼
                  Best Result
                  (from any thread)
```

## Related Documents
- docs/03-roadmap.md (Phase 8)
- 8-007-tree-pruning-evaluation.md
- 8-002-strategy-tree-data-structure.md

## Dependencies
- 8-002: Strategy Tree Data Structure (tree operations)
- 8-007: Tree Pruning and Evaluation (search algorithm)

## Acceptance Criteria
- [ ] Evaluation cache reduces redundant computation
- [ ] Parallel search provides measurable speedup
- [ ] Incremental updates reuse previous work
- [ ] Memory stays within budget
- [ ] Think time meets targets at all difficulties
- [ ] Performance stats available for tuning

## Notes
Performance optimization is critical for player experience. The AI must
respond quickly enough to feel responsive but think deeply enough to
play well. The budget system ensures consistent behavior across hardware.
