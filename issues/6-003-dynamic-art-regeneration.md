# 6-003: Dynamic Art Regeneration

## Current Behavior
No dynamic card art regeneration exists.

## Intended Behavior
A system that regenerates card art when cards are drawn:
- Track which cards need art regeneration (needs_regen flag)
- Trigger regeneration on card draw
- Apply user style preferences from localStorage
- Queue requests to avoid overwhelming image API
- Cache generated images for reuse

## Sub-Issues

This issue has been split into the following sub-issues:

| ID | Description | Status |
|----|-------------|--------|
| 6-003a | Regeneration Tracking | pending |
| 6-003b | Style Guide Integration | pending |
| 6-003c | Generation Queue | pending |
| 6-003d | Cache Invalidation | pending |

## Implementation Order

1. **6-003a** first - track which cards need regeneration
2. **6-003b** second - apply style preferences to prompts
3. **6-003c** third - queue and batch requests
4. **6-003d** last - manage cache lifecycle

## Related Documents
- docs/02-game-mechanics.md (dynamic art)
- docs/04-architecture-c-server.md
- 3-006-client-style-preferences.md
- 6-001-comfyui-api-client.md

## Dependencies
- 6-001: ComfyUI API Client
- 6-002: Card Image Prompt Builder
- 3-006: Client Style Preferences

## Dynamic Art Flow

```
Card Drawn
    │
    ▼
Check needs_regen flag ──── false ──► Use cached image
    │
    true
    │
    ▼
Get user style guide from localStorage
    │
    ▼
Build prompt with card data + style
    │
    ▼
Add to generation queue
    │
    ▼
ComfyUI generates image
    │
    ▼
Cache image, clear needs_regen
    │
    ▼
Display new art
```

## Acceptance Criteria
- [ ] needs_regen flag tracked per CardInstance
- [ ] Style guide applied to generation requests
- [ ] Queue prevents API overload
- [ ] Cache stores generated images
- [ ] Images display in browser client
