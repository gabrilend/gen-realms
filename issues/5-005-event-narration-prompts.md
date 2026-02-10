# 3-005: Context Window Management

## Current Behavior
No context management exists. Each LLM call is independent.

## Intended Behavior
A system that manages narrative history within token limits:
- Maintains rolling story summary
- Includes recent events in full
- Summarizes older events
- Stays within model context limits
- Preserves key narrative beats

## Suggested Implementation Steps

1. Create `src/llm/context-manager.lua`
2. Define context structure:
   ```lua
   local context = {
     max_tokens = 4000,
     story_summary = "",        -- compressed history
     recent_events = {},        -- last 5-10 events in full
     key_moments = {},          -- always-include dramatic events
     current_state = ""         -- game state summary
   }
   ```
3. Implement `Context.new(config)` - create manager
4. Implement `Context.add_event(ctx, event_text)` - append event
5. Implement `Context.summarize_old(ctx)` - compress old events
6. Implement `Context.get_prompt_context(ctx)` - build context string
7. Implement token counting (estimate by characters/4)
8. Implement key moment tagging (when to preserve events)
9. Add periodic full summarization
10. Write tests for context overflow handling

## Related Documents
- 3-004-narrative-generation-prompt.md
- 3-001-llm-api-integration-module.md

## Dependencies
- 3-001: LLM API Integration

## Context Example

```
=== Story Summary (compressed) ===
The battle began with Lady Morgaine favoring the Wilds
while Lord Theron built merchant alliances. Early skirmishes
saw both sides testing defenses.

=== Key Moments ===
Turn 8: The Sacred Grove was established, spawning wolves
Turn 12: Lord Theron's authority fell below 20

=== Recent Events (last 3 turns) ===
Turn 14: Morgaine unleashed the Primal Titan, dealing 12 damage
Turn 15: Theron fortified with the Arcane Workshop
Turn 16: (current turn)

=== Current State ===
Morgaine: 31 Authority | Theron: 18 Authority
```

## Acceptance Criteria
- [ ] Context stays within token limit
- [ ] Old events summarize correctly
- [ ] Recent events preserved in full
- [ ] Key moments always included
- [ ] Narrative continuity maintained
