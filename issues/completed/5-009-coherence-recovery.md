# 5-009: Coherence Recovery

## Current Behavior
No recovery when narrative becomes incoherent.

## Intended Behavior
A coherence system that:
- Detects narrative inconsistencies
- Recovers from context loss
- Rebuilds world state when needed
- Smoothly transitions after recovery
- Logs coherence issues for debugging

## Suggested Implementation Steps

1. Define coherence metrics:
   ```c
   // {{{ coherence types
   typedef struct {
       bool names_consistent;
       bool faction_consistent;
       bool timeline_consistent;
       float overall_score;
   } CoherenceCheck;

   typedef enum {
       COHERENCE_OK,
       COHERENCE_MINOR_ISSUE,
       COHERENCE_MAJOR_ISSUE,
       COHERENCE_RECOVERY_NEEDED
   } CoherenceLevel;
   // }}}
   ```

2. Implement coherence checking:
   ```c
   // {{{ check coherence
   CoherenceCheck check_coherence(WorldState* ws, const char* new_narrative) {
       CoherenceCheck check = {0};

       // Check player names match
       check.names_consistent = narrative_contains_names(new_narrative, ws);

       // Check faction references are valid
       check.faction_consistent = factions_valid(new_narrative);

       // Check timeline makes sense
       check.timeline_consistent = timeline_valid(new_narrative, ws);

       check.overall_score = (check.names_consistent +
                              check.faction_consistent +
                              check.timeline_consistent) / 3.0f;

       return check;
   }
   // }}}
   ```

3. Implement recovery triggers:
   ```c
   // {{{ recovery
   void trigger_recovery_if_needed(Game* game, CoherenceCheck* check) {
       if (check->overall_score < 0.5f) {
           // Rebuild world state from scratch
           world_state_rebuild(game->world_state, game);

           // Generate recovery narrative
           char* recovery = generate_recovery_narrative(game);
           narrative_add(game, recovery);
       }
   }
   // }}}
   ```

4. Implement world state rebuild:
   ```c
   // {{{ rebuild
   void world_state_rebuild(WorldState* ws, Game* game) {
       // Clear current state
       world_state_clear(ws);

       // Rebuild from game state
       ws->turn_number = game->turn;
       rebuild_force_descriptions(ws, game);
       rebuild_battlefield(ws, game);
   }
   // }}}
   ```

5. Implement recovery narrative generation

6. Add coherence logging

7. Write tests for recovery scenarios

## Related Documents
- 5-003-world-state-prompt.md
- 5-007-context-window-management.md

## Dependencies
- 5-003: World State Prompt
- 5-007: Context Window Management

## Recovery Scenarios

| Scenario | Detection | Recovery |
|----------|-----------|----------|
| Name mismatch | Names don't match players | Rebuild from game state |
| Faction confusion | Invalid faction references | Clear and regenerate |
| Timeline jump | Turn numbers inconsistent | Summarize and restart |
| Context overflow | Summarization lost detail | Rebuild world state |

## Acceptance Criteria
- [x] Coherence metrics computed
- [x] Issues detected automatically
- [x] Recovery rebuilds state
- [x] Transition narrative smooth
- [x] Issues logged for debugging

## Implementation Notes

Created `src/llm/09-coherence.h` and `src/llm/09-coherence.c`:

### Core Data Structures
- `CoherenceLevel` - Severity levels (OK, MINOR_ISSUE, MAJOR_ISSUE, RECOVERY_NEEDED)
- `CoherenceCheck` - Results with individual checks and overall score
- `CoherenceLogEntry` - History record for debugging
- `CoherenceManager` - Manages checking, recovery, and statistics

### Key Functions

**Coherence Checking:**
- `coherence_check()` - Full coherence analysis
- `coherence_check_names()` - Verify player names match game
- `coherence_check_factions()` - Validate faction references
- `coherence_check_timeline()` - Check turn number consistency
- `coherence_check_authority()` - Verify authority values plausible

**Recovery:**
- `coherence_recover()` - Trigger full recovery
- `coherence_rebuild_world_state()` - Rebuild from game data
- `coherence_generate_recovery_narrative()` - Smooth transition text

**Management:**
- `coherence_manager_create/free()` - Lifecycle
- `coherence_should_check()` - Intelligent check scheduling
- `coherence_log_entry()` - Record check results
- `coherence_get_stats()` - Retrieve statistics
- `coherence_get_recent_issues()` - Debug output

### Coherence Thresholds
```c
#define COHERENCE_THRESHOLD_MINOR 0.7f     /* Below = minor issue */
#define COHERENCE_THRESHOLD_MAJOR 0.5f     /* Below = major issue */
#define COHERENCE_THRESHOLD_RECOVERY 0.3f  /* Below = recovery needed */
```

### Check Scheduling
- Every 3rd turn
- After consecutive issues
- When player authority low (≤10)

### Unit Tests
23 tests in `tests/test-coherence.c`:
- Manager create/free
- Name consistency checking
- Faction validation
- Timeline checking
- Authority plausibility
- Full coherence checks (coherent and incoherent)
- World state rebuild
- Recovery narrative generation
- Check scheduling logic
- Statistics tracking
- Logging
- Null handling
- Consecutive issues tracking

## Completion Date
2026-02-11
