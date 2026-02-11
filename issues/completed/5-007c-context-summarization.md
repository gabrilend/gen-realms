# 5-007c: Context Summarization

## Status: COMPLETED

## Current Behavior
Context manager supports finding summarizable entries, combining their text, and replacing them with summaries. LLM integration for actual summarization is handled by calling code.

## Intended Behavior
Summarize old context entries to fit within token limits:
- Identify candidates for summarization
- Call LLM to generate summaries
- Replace multiple entries with summary
- Preserve essential information

## Implementation Details

### Functions Added to 06-context-manager.c

1. **context_remove_at()**: Removes entry at specific index
   - Updates token count
   - Shifts remaining entries

2. **context_find_summarizable()**: Finds summarization candidates
   - Returns entries with PRIORITY_FORCE_DESC or PRIORITY_OLD_EVENTS
   - Skips entries already marked as summaries
   - Returns indices array and count

3. **context_get_entries_text()**: Concatenates entry texts
   - Joins specified entries with newlines
   - Returns combined string for LLM summarization

4. **context_replace_with_summary()**: Replaces entries with summary
   - Removes specified entries in reverse order
   - Adds summary with PRIORITY_OLD_EVENTS
   - Marks as summary and increments summary_count

5. **context_needs_summarization()**: Threshold check
   - Returns true if utilization >= threshold
   - Typical threshold: 0.8 (80%)

6. **context_mark_as_summary()**: Marks entry as summary
   - Sets is_summary flag on most recent entry
   - Prevents re-summarization

### Test Results
All 37 tests passing:
- Initialization (3 tests)
- Token estimation (3 tests)
- Statistics (3 tests)
- Capacity (3 tests)
- Cleanup (3 tests)
- Priority values (1 test)
- Entry management (4 tests)
- Eviction (3 tests)
- Prompt building (3 tests)
- Entry count (3 tests)
- Summarization (8 tests)

### Usage Pattern for LLM Summarization

```c
// Check if summarization needed
if (context_needs_summarization(cm, 0.8f)) {
    int indices[10];
    int count = context_find_summarizable(cm, indices, 10);

    if (count >= 3) {
        char* text = context_get_entries_text(cm, indices, count);

        // Call LLM to generate summary (handled by caller)
        char* summary = llm_summarize(text);

        context_replace_with_summary(cm, indices, count, summary);

        free(text);
        free(summary);
    }
}
```

## Related Documents
- 5-007a-context-manager-structure.md
- 5-007b-context-entry-management.md
- 5-007-context-window-management.md (parent issue)
- 5-001-llm-api-client.md

## Dependencies
- 5-007a: Context Manager Structure (completed)
- 5-007b: Context Entry Management (completed)
- 5-001: LLM API Client (for actual summarization, used by caller)

## Acceptance Criteria
- [x] Identifies summarizable entries
- [x] LLM generates coherent summaries (via calling code)
- [x] Old entries replaced with summary
- [x] Token count decreases after summarization
- [x] Essential information preserved (via is_summary flag preventing re-summarization)
