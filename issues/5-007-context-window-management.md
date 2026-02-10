# 5-007: Context Window Management

## Current Behavior
No context window management for LLM calls.

## Intended Behavior
A context management system that:
- Tracks token usage across prompts
- Summarizes old content to fit window
- Prioritizes recent and important events
- Manages sliding window of context
- Prevents context overflow

## Suggested Implementation Steps

1. Create context manager structure:
   ```c
   // {{{ context manager
   typedef struct {
       char** entries;
       int* token_counts;
       int* priorities;
       int entry_count;
       int max_entries;
       int max_tokens;
       int current_tokens;
   } ContextManager;
   // }}}
   ```

2. Implement `context_init()`:
   ```c
   // {{{ init
   ContextManager* context_init(int max_tokens) {
       ContextManager* cm = malloc(sizeof(ContextManager));
       cm->max_tokens = max_tokens;
       cm->max_entries = 100;
       cm->entries = calloc(cm->max_entries, sizeof(char*));
       cm->token_counts = calloc(cm->max_entries, sizeof(int));
       cm->priorities = calloc(cm->max_entries, sizeof(int));
       return cm;
   }
   // }}}
   ```

3. Implement `context_add()`:
   ```c
   // {{{ add entry
   void context_add(ContextManager* cm, const char* text, int priority) {
       int tokens = estimate_tokens(text);

       // If would overflow, summarize old entries
       while (cm->current_tokens + tokens > cm->max_tokens) {
           context_summarize_oldest(cm);
       }

       // Add new entry
       cm->entries[cm->entry_count] = strdup(text);
       cm->token_counts[cm->entry_count] = tokens;
       cm->priorities[cm->entry_count] = priority;
       cm->current_tokens += tokens;
       cm->entry_count++;
   }
   // }}}
   ```

4. Implement `context_summarize_oldest()`:
   ```c
   // {{{ summarize
   void context_summarize_oldest(ContextManager* cm) {
       // Find N oldest low-priority entries
       // Send to LLM for summarization
       // Replace with summary
   }
   // }}}
   ```

5. Implement token estimation:
   ```c
   // {{{ estimate tokens
   int estimate_tokens(const char* text) {
       // Rough estimate: ~4 chars per token
       return strlen(text) / 4;
   }
   // }}}
   ```

6. Implement `context_build_prompt()`:
   ```c
   // {{{ build prompt
   char* context_build_prompt(ContextManager* cm) {
       // Concatenate entries in priority order
       // Return full context string
   }
   // }}}
   ```

7. Add priority levels for different content types

8. Write tests

## Related Documents
- 5-001-llm-api-client.md
- 5-002-prompt-network-structure.md

## Dependencies
- 5-001: LLM API Client (for summarization calls)

## Priority Levels

| Priority | Content Type |
|----------|-------------|
| 1 (highest) | Current turn events |
| 2 | World state |
| 3 | Recent events (last 3 turns) |
| 4 | Force descriptions |
| 5 (lowest) | Old events |

## Sub-Issues
This issue has been split into sub-issues for manageable implementation:
- 5-007a: Context Manager Structure
- 5-007b: Context Entry Management
- 5-007c: Context Summarization

## Acceptance Criteria
- [ ] Token counting works (5-007a)
- [ ] Entries added with priorities (5-007b)
- [ ] Overflow triggers summarization (5-007c)
- [ ] Context builds correctly (5-007b)
- [ ] Window stays within limits (5-007a, 5-007c)
