# 5-007b: Context Entry Management

## Current Behavior
No way to add or manage context entries.

## Intended Behavior
Manage entries in the context window:
- Add new entries with priorities
- Track token counts automatically
- Build final prompt from entries
- Sort entries by priority for prompt construction

## Suggested Implementation Steps

1. Implement `context_add()`:
   ```c
   // {{{ add entry
   bool context_add(ContextManager* cm, const char* text, ContextPriority priority) {
       if (cm->entry_count >= cm->max_entries) {
           return false;  // Would need to evict first
       }

       int tokens = estimate_tokens(text);

       // Check if we need to make room
       while (cm->current_tokens + tokens > cm->max_tokens &&
              cm->entry_count > 0) {
           if (!context_evict_lowest(cm)) {
               return false;  // Can't make room
           }
       }

       ContextEntry* entry = &cm->entries[cm->entry_count++];
       entry->text = strdup(text);
       entry->token_count = tokens;
       entry->priority = priority;
       entry->added_at = time(NULL);
       cm->current_tokens += tokens;

       return true;
   }
   // }}}
   ```

2. Implement `context_evict_lowest()`:
   ```c
   // {{{ evict lowest
   bool context_evict_lowest(ContextManager* cm) {
       if (cm->entry_count == 0) return false;

       // Find lowest priority (highest number) entry
       int lowest_idx = 0;
       int lowest_priority = cm->entries[0].priority;

       for (int i = 1; i < cm->entry_count; i++) {
           if (cm->entries[i].priority > lowest_priority) {
               lowest_priority = cm->entries[i].priority;
               lowest_idx = i;
           }
       }

       // Remove entry
       cm->current_tokens -= cm->entries[lowest_idx].token_count;
       free(cm->entries[lowest_idx].text);

       // Shift remaining entries
       for (int i = lowest_idx; i < cm->entry_count - 1; i++) {
           cm->entries[i] = cm->entries[i + 1];
       }
       cm->entry_count--;

       return true;
   }
   // }}}
   ```

3. Implement `context_clear()`:
   ```c
   // {{{ clear
   void context_clear(ContextManager* cm) {
       for (int i = 0; i < cm->entry_count; i++) {
           free(cm->entries[i].text);
       }
       cm->entry_count = 0;
       cm->current_tokens = 0;
   }
   // }}}
   ```

4. Implement `context_build_prompt()`:
   ```c
   // {{{ build prompt
   char* context_build_prompt(ContextManager* cm) {
       // Sort entries by priority (ascending = highest priority first)
       qsort(cm->entries, cm->entry_count, sizeof(ContextEntry),
             compare_priority);

       // Calculate total size
       size_t total = 1;  // null terminator
       for (int i = 0; i < cm->entry_count; i++) {
           total += strlen(cm->entries[i].text) + 2;  // + newlines
       }

       // Build concatenated prompt
       char* prompt = malloc(total);
       prompt[0] = '\0';

       for (int i = 0; i < cm->entry_count; i++) {
           strcat(prompt, cm->entries[i].text);
           strcat(prompt, "\n\n");
       }

       return prompt;
   }
   // }}}
   ```

5. Implement `context_update_priority()` for dynamic reprioritization

6. Add entry age tracking for LRU within same priority

7. Write entry management tests

## Related Documents
- 5-007a-context-manager-structure.md
- 5-007-context-window-management.md (parent issue)

## Dependencies
- 5-007a: Context Manager Structure

## Entry Lifecycle

```
Add Entry
    ↓
Estimate Tokens
    ↓
Check Capacity → Evict if needed
    ↓
Store Entry
    ↓
Update Token Count
```

## Acceptance Criteria
- [ ] Entries add correctly with token tracking
- [ ] Eviction removes lowest priority
- [ ] Clear removes all entries
- [ ] Prompt builds in priority order
- [ ] Token count stays accurate
