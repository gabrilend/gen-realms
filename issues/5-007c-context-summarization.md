# 5-007c: Context Summarization

## Current Behavior
No summarization when context exceeds limits.

## Intended Behavior
Summarize old context entries to fit within token limits:
- Identify candidates for summarization
- Call LLM to generate summaries
- Replace multiple entries with summary
- Preserve essential information

## Suggested Implementation Steps

1. Create summarization prompt template:
   ```c
   // {{{ summarization prompt
   static const char* SUMMARIZE_PROMPT =
       "Summarize the following game events into a brief paragraph "
       "that preserves key narrative elements, character names, and "
       "important game state changes:\n\n%s\n\n"
       "Summary (1-2 sentences):";
   // }}}
   ```

2. Implement `context_find_summarizable()`:
   ```c
   // {{{ find summarizable
   int context_find_summarizable(ContextManager* cm, int* indices, int max_count) {
       int count = 0;

       // Find old, low-priority entries
       for (int i = 0; i < cm->entry_count && count < max_count; i++) {
           if (cm->entries[i].priority >= PRIORITY_OLD_EVENTS) {
               indices[count++] = i;
           }
       }

       return count;
   }
   // }}}
   ```

3. Implement `context_summarize_entries()`:
   ```c
   // {{{ summarize entries
   bool context_summarize_entries(ContextManager* cm, LLMClient* llm,
                                   int* indices, int count) {
       // Concatenate entries to summarize
       size_t total_len = 0;
       for (int i = 0; i < count; i++) {
           total_len += strlen(cm->entries[indices[i]].text) + 2;
       }

       char* combined = malloc(total_len + 1);
       combined[0] = '\0';
       for (int i = 0; i < count; i++) {
           strcat(combined, cm->entries[indices[i]].text);
           strcat(combined, "\n");
       }

       // Build and send summarization request
       char* prompt = format_string(SUMMARIZE_PROMPT, combined);
       LLMResponse* resp = llm_request(llm, prompt, NULL);
       free(prompt);
       free(combined);

       if (!resp || resp->error) {
           return false;
       }

       // Remove old entries (in reverse order to maintain indices)
       for (int i = count - 1; i >= 0; i--) {
           context_remove_at(cm, indices[i]);
       }

       // Add summary as single entry
       context_add(cm, resp->text, PRIORITY_OLD_EVENTS);

       llm_response_free(resp);
       return true;
   }
   // }}}
   ```

4. Implement `context_auto_summarize()`:
   ```c
   // {{{ auto summarize
   void context_auto_summarize(ContextManager* cm, LLMClient* llm) {
       // Trigger if we're over 80% capacity
       float usage = (float)cm->current_tokens / cm->max_tokens;
       if (usage < 0.8f) return;

       int indices[10];
       int count = context_find_summarizable(cm, indices, 10);

       if (count >= 3) {
           context_summarize_entries(cm, llm, indices, count);
       }
   }
   // }}}
   ```

5. Implement `context_remove_at()` helper

6. Add summarization statistics tracking

7. Write summarization tests

## Related Documents
- 5-007a-context-manager-structure.md
- 5-007b-context-entry-management.md
- 5-007-context-window-management.md (parent issue)
- 5-001-llm-api-client.md

## Dependencies
- 5-007a: Context Manager Structure
- 5-007b: Context Entry Management
- 5-001: LLM API Client (for summarization calls)

## Summarization Strategy

| Condition | Action |
|-----------|--------|
| <80% capacity | No action |
| 80-95% capacity | Summarize 3-5 old entries |
| >95% capacity | Summarize 10+ entries |
| Near limit | Evict lowest priority |

## Acceptance Criteria
- [ ] Identifies summarizable entries
- [ ] LLM generates coherent summaries
- [ ] Old entries replaced with summary
- [ ] Token count decreases after summarization
- [ ] Essential information preserved
