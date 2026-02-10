# 5-007a: Context Manager Structure

## Current Behavior
No structure to track context window usage.

## Intended Behavior
Define the core context manager structure:
- Track entries with their token counts
- Maintain total token count
- Configure maximum tokens
- Initialize and cleanup properly

## Suggested Implementation Steps

1. Create `src/llm/context-manager.h`:
   ```c
   // {{{ context types
   #ifndef CONTEXT_MANAGER_H
   #define CONTEXT_MANAGER_H

   typedef struct {
       char* text;
       int token_count;
       int priority;
       time_t added_at;
   } ContextEntry;

   typedef struct {
       ContextEntry* entries;
       int entry_count;
       int max_entries;
       int max_tokens;
       int current_tokens;
   } ContextManager;
   // }}}
   ```

2. Define priority levels:
   ```c
   // {{{ priorities
   typedef enum {
       PRIORITY_SYSTEM = 0,      // Highest: system prompts
       PRIORITY_CURRENT_TURN = 1,
       PRIORITY_WORLD_STATE = 2,
       PRIORITY_RECENT_EVENTS = 3,
       PRIORITY_FORCE_DESC = 4,
       PRIORITY_OLD_EVENTS = 5   // Lowest: old events
   } ContextPriority;
   // }}}
   ```

3. Implement `context_init()`:
   ```c
   // {{{ init
   ContextManager* context_init(int max_tokens) {
       ContextManager* cm = malloc(sizeof(ContextManager));
       cm->max_tokens = max_tokens;
       cm->max_entries = 100;
       cm->entries = calloc(cm->max_entries, sizeof(ContextEntry));
       cm->entry_count = 0;
       cm->current_tokens = 0;
       return cm;
   }
   // }}}
   ```

4. Implement token estimation:
   ```c
   // {{{ estimate tokens
   int estimate_tokens(const char* text) {
       // Rough estimate: ~4 characters per token for English
       // More accurate for LLM-specific tokenizers would require
       // integrating a tokenizer library
       if (!text) return 0;
       return (strlen(text) + 3) / 4;
   }
   // }}}
   ```

5. Implement `context_cleanup()`:
   ```c
   // {{{ cleanup
   void context_cleanup(ContextManager* cm) {
       for (int i = 0; i < cm->entry_count; i++) {
           free(cm->entries[i].text);
       }
       free(cm->entries);
       free(cm);
   }
   // }}}
   ```

6. Implement `context_get_usage()` for statistics

7. Write initialization tests

## Related Documents
- 5-007-context-window-management.md (parent issue)
- 5-001-llm-api-client.md

## Dependencies
- None (standalone module)

## Token Budget Example

| Model | Max Tokens | Reserved for Response |
|-------|------------|----------------------|
| GPT-4 | 8192 | 1024 |
| Llama 3 | 4096 | 512 |
| Claude | 100000 | 4096 |

Typical context budget: max_tokens - response_reserved

## Acceptance Criteria
- [ ] ContextManager structure defined
- [ ] Init allocates resources correctly
- [ ] Token estimation works reasonably
- [ ] Cleanup frees all memory
- [ ] Usage statistics available
