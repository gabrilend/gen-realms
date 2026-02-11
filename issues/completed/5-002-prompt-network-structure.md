# 5-002: Prompt Network Structure

## Status: COMPLETED

## Current Behavior
Prompt network with templates, variable interpolation, and chain support.

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
- [x] All prompt types defined
- [x] Variable interpolation works
- [x] Templates produce valid prompts
- [x] Chaining builds complex prompts
- [x] System prompts managed

## Implementation Notes

### Files Created
- `src/llm/02-prompts.h` - Type definitions and function prototypes
- `src/llm/02-prompts.c` - Template implementations
- `tests/test-prompts.c` - Unit tests (15 tests, all passing)

### Prompt Types Implemented
- PROMPT_WORLD_STATE - Game state description
- PROMPT_FORCE_DESCRIPTION - Player force details
- PROMPT_EVENT_NARRATION - Game event narration
- PROMPT_TRADE_ROW_SELECTION - Card selection for trade row
- PROMPT_TURN_SUMMARY - Turn summary generation
- PROMPT_CARD_PLAYED - Card play narration
- PROMPT_ATTACK - Attack event narration

### Key Features
- PromptVars container for flexible variable management
- {var_name} placeholder interpolation
- Required variable validation before build
- Pre-defined chains for common scenarios (turn, card_play, combat)
- Fantasy-themed system prompt for consistent narrative tone

### Test Coverage
- Variable creation, addition, update, retrieval
- Template access and validation
- Variable interpolation
- Missing variable detection
- Prompt chain creation and standard chains
- Memory safety (NULL handling)

Completed: 2026-02-10
