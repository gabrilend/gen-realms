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
| 6-003a | Regeneration Tracking | completed |
| 6-003b | Style Guide Integration | completed |
| 6-003c | Generation Queue | completed |
| 6-003d | Cache Invalidation | completed |

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
- [x] needs_regen flag tracked per CardInstance
- [x] Style guide applied to generation requests
- [x] Queue prevents API overload
- [x] Cache stores generated images
- [ ] Images display in browser client (requires integration)

## Implementation Notes (2026-02-11)

Created 4 JavaScript modules for browser client:

**6-003a: art-tracker.js**
- ArtTracker class with pending card tracking
- markForRegeneration(), markComplete(), markFailed()
- Event handlers for card upgrades, draws, and regen requests

**6-003b: style-merger.js**
- StyleMerger class for prompt building
- Faction styles, art style keywords, upgrade modifiers
- buildCompletePrompt() combines all elements
- Integration with preferences.js

**6-003c: generation-queue.js**
- GenerationQueue class with priority ordering
- Concurrency limits and retry with exponential backoff
- Progress callbacks for UI updates
- Mock ComfyUI client for development

**6-003d: image-cache.js**
- ImageCache class with two-tier caching
- Memory cache with LRU eviction
- IndexedDB for persistence across page reloads
- invalidateAll() for style preference changes

## Status: COMPLETE
