# 5-002: Prompt Network Structure

## Current Behavior
No prompt organization exists.

## Intended Behavior
A structured prompt system that:
- Defines prompt templates for each narrative type
- Chains prompts for complex generation
- Manages prompt dependencies
- Provides consistent formatting
- Supports variable interpolation

## Suggested Implementation Steps

1. Create `src/llm/prompts.h` with prompt types:
   ```c
   // {{{ prompt types
   typedef enum {
       PROMPT_WORLD_STATE,
       PROMPT_FORCE_DESCRIPTION,
       PROMPT_EVENT_NARRATION,
       PROMPT_TRADE_ROW_SELECTION,
       PROMPT_TURN_SUMMARY,
       PROMPT_COUNT
   } PromptType;

   typedef struct {
       PromptType type;
       char* template;
       char** required_vars;
       int var_count;
   } PromptTemplate;
   // }}}
   ```

2. Create `src/llm/prompts.c` with templates

3. Define prompt templates:
   ```c
   // {{{ templates
   static const char* WORLD_STATE_TEMPLATE =
       "The battle for Symbeline rages. "
       "Turn %d. %s commands authority of %d. "
       "%s commands authority of %d. "
       "The current phase is %s.";
   // }}}
   ```

4. Implement `prompt_build()`:
   ```c
   // {{{ prompt_build
   char* prompt_build(PromptType type, PromptVars* vars) {
       const PromptTemplate* tmpl = &templates[type];
       // Interpolate variables into template
       // Return allocated string
   }
   // }}}
   ```

5. Implement prompt chaining for complex narratives

6. Add system prompt management

7. Write tests for all prompt types

## Related Documents
- 5-001-llm-api-client.md
- 5-003-world-state-prompt.md

## Dependencies
- 5-001: LLM API Client

## Prompt Network

```
┌─────────────────┐
│  World State    │ (persistent context)
└────────┬────────┘
         │
    ┌────┴────┐
    ▼         ▼
┌───────┐ ┌───────────┐
│ Force │ │ Trade Row │
│ Desc  │ │ Selection │
└───┬───┘ └─────┬─────┘
    │           │
    └─────┬─────┘
          ▼
    ┌───────────┐
    │  Event    │
    │ Narration │
    └───────────┘
```

## Acceptance Criteria
- [ ] All prompt types defined
- [ ] Variable interpolation works
- [ ] Templates produce valid prompts
- [ ] Chaining builds complex prompts
- [ ] System prompts managed
