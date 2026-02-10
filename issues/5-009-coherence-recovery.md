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
- [ ] Coherence metrics computed
- [ ] Issues detected automatically
- [ ] Recovery rebuilds state
- [ ] Transition narrative smooth
- [ ] Issues logged for debugging
