# Track Gamma: AI & Content - Progress

This document tracks the progress of Track Gamma (AI & Content) which consolidates
the original Track D (Game Content) and Track E (AI Integration) issues.

## Track Overview

Track Gamma handles all game content and AI integration:
- Card design and faction definitions (JSON)
- Balance validation tools
- LLM narrative generation
- ComfyUI visual generation
- Context management and caching

## Current Status: Visual Pipeline In Progress

### In Progress

| Issue | Description | Status | Developer |
|-------|-------------|--------|-----------|
| - | - | - | - |

### Completed Issues (Content - was Track D)

| Issue | Description | Date |
|-------|-------------|------|
| 4-001 | Card JSON schema | 2026-02-10 |
| 4-002 | Merchant Guilds faction | 2026-02-10 |
| 4-003 | The Wilds faction | 2026-02-10 |
| 4-004 | High Kingdom faction | 2026-02-10 |
| 4-005 | Artificer Order faction | 2026-02-10 |
| 4-006 | Starting deck definition | 2026-02-10 |
| 4-007 | Neutral trade deck cards | 2026-02-10 |
| 4-008 | Upgrade effect cards | 2026-02-10 |

### Completed Issues (AI - was Track E)

| Issue | Description | Date |
|-------|-------------|------|
| 5-001 | LLM API client | 2026-02-10 |
| 6-001 | ComfyUI API client | 2026-02-10 |
| 5-002 | Prompt network structure | 2026-02-11 |
| 5-003 | World state prompt | 2026-02-11 |
| 5-004 | Force description prompts | 2026-02-11 |
| 5-005 | Event narration prompts | 2026-02-11 |
| 5-007a | Context manager structure | 2026-02-11 |
| 5-007b | Context entry management | 2026-02-11 |
| 5-007c | Context summarization | 2026-02-11 |
| 5-008 | Narrative caching | 2026-02-11 |
| 6-002 | Card image prompt builder | 2026-02-11 |
| 6-003a | Regeneration tracking | 2026-02-11 |
| 6-003b | Style guide integration | 2026-02-11 |
| 6-003c | Generation queue | 2026-02-11 |
| 6-003d | Cache invalidation | 2026-02-11 |
| 6-004 | Upgrade visualization | 2026-02-11 |
| 6-005 | Battle canvas manager | 2026-02-11 |
| 6-006a | Event-to-region mapping | 2026-02-11 |
| 6-006b | Priority queue | 2026-02-11 |
| 6-006c | Mask generation | 2026-02-11 |
| 6-007 | Scene composition rules | 2026-02-11 |
| 6-008 | Style transfer prompts | 2026-02-11 |

### Pending Issues (Content)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 4-009 | Card balance validator | Track Alpha engine |
| 4-010 | Phase 4 Demo | 4-009, Track Beta 3-010 |

### Pending Issues (AI - LLM)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 5-006 | Trade row selection logic | 5-005, Track Alpha 1-004 |
| 5-009 | Coherence recovery | 5-006, 5-008 |
| 5-010 | Phase 5 Demo | 5-009, Track Beta 3-010 |

### Pending Issues (AI - Visual)

| Issue | Description | Dependencies |
|-------|-------------|--------------|
| 6-009 | Image caching/persistence | 6-004 (done) |
| 6-010 | Phase 6 Demo | 6-009, 5-010 |

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | API clients ready |
| BETA | COMPLETE | Content and prompts done |
| GAMMA | IN PROGRESS | Context management done, caching WIP |
| DELTA | BLOCKED | Waiting on visual pipeline |
| EPSILON | BLOCKED | Waiting on balance testing |
| OMEGA | BLOCKED | Waiting on full integration |

## Statistics

**Content Issues:** 8 complete, 2 pending
**AI Issues:** 22 complete, 4 pending
**Total:** 30 complete, 6 pending

**Cards Created:** 65 total
- Starting: 2 cards
- Neutral: 6 cards
- Merchant Guilds: 14 cards
- The Wilds: 14 cards
- High Kingdom: 14 cards
- Artificer Order: 15 cards

**Unit Tests Passing:** 170 total
- LLM client: 9
- Prompts: 15
- World state: 12
- Force description: 18
- Event narration: 23
- Context manager: 37
- Narrative cache: 20
- ComfyUI client: 12
- Card prompts: 24

## Deliverables

### Content (Phase 4)
- [x] `assets/cards/schema.json` - Card definition schema
- [x] `assets/cards/starting/*.json` - Starting deck cards
- [x] `assets/cards/neutral/*.json` - Neutral trade deck
- [x] `assets/cards/merchant/*.json` - Merchant Guilds faction
- [x] `assets/cards/wilds/*.json` - The Wilds faction
- [x] `assets/cards/kingdom/*.json` - High Kingdom faction
- [x] `assets/cards/artificer/*.json` - Artificer Order faction
- [ ] `src/tools/balance-validator.c` - Balance analysis (4-009)
- [ ] `issues/completed/demos/phase-4-demo.sh` - Demo script (4-010)

### AI - LLM (Phase 5)
- [x] `src/llm/01-api-client.h/c` - LLM API client
- [x] `src/llm/02-prompts.h/c` - Prompt templates
- [x] `src/llm/03-world-state.h/c` - World state tracking
- [x] `src/llm/04-force-description.h/c` - Faction descriptions
- [x] `src/llm/05-event-narration.h/c` - Event narration
- [x] `src/llm/06-context-manager.h/c` - Context window management
- [x] `src/llm/07-narrative-cache.h/c` - Caching (5-008)
- [ ] `issues/completed/demos/phase-5-demo.sh` - AI-narrated game

### AI - Visual (Phase 6)
- [x] `src/visual/01-comfyui-client.h/c` - ComfyUI client
- [x] `src/visual/02-card-prompts.h/c` - Card image prompts (6-002)
- [x] `assets/web/art-tracker.js` - Regeneration tracking (6-003a)
- [x] `assets/web/style-merger.js` - Style guide integration (6-003b)
- [x] `assets/web/generation-queue.js` - Generation queue (6-003c)
- [x] `assets/web/image-cache.js` - Cache invalidation (6-003d)
- [x] `assets/web/upgrade-viz.js` - Upgrade visualization (6-004)
- [x] `assets/web/battle-canvas.js` - Battle canvas manager (6-005)
- [x] `assets/web/region-selector.js` - Inpainting region selection (6-006)
- [x] `assets/web/scene-composition.js` - Scene composition rules (6-007)
- [x] `assets/web/style-transfer.js` - Style transfer prompts (6-008)
- [ ] `issues/completed/demos/phase-6-demo.sh` - Visual generation demo

## Notes

### 2026-02-11: Visual Pipeline Complete
Completed entire visual generation pipeline (6-003 through 6-008):

**6-003 (dynamic art regeneration)** - 4 JavaScript modules:
- art-tracker.js: Track cards needing regeneration
- style-merger.js: Merge user preferences with card prompts
- generation-queue.js: Priority queue with retry logic
- image-cache.js: Two-tier cache (memory + IndexedDB)

**6-004 (upgrade visualization)** - upgrade-viz.js:
- Particle effects for ally_active, empowered, scrapped, targeted
- Glow rendering with pulsing animation
- Canvas-based overlay system

**6-005 (battle canvas manager)** - battle-canvas.js:
- Region-based canvas with 6 defined zones
- Undo/redo history stack (max 50 states)
- Mask generation for inpainting
- Export to PNG/JPEG

**6-006 (inpainting region selection)** - region-selector.js:
- Event-to-region mapping (game_start, card_played, attack, etc.)
- Priority queue with dependency-aware ordering
- Region generation order: sky → bases → forces → center

**6-007 (scene composition rules)** - scene-composition.js:
- Z-layer management (BACKGROUND=0 through OVERLAY=4)
- Element placement with overlap detection
- Zone density tracking
- Composition-aware prompt building

**6-008 (style transfer prompts)** - style-transfer.js:
- Complete faction style definitions with color palettes
- 5 art style presets (painterly, detailed, stylized, icon, cinematic)
- Multi-faction style blending
- Style consistency validation

### Faction Themes

**Merchant Guilds** - Trade generation, authority gain, prosperity
**The Wilds** - Combat power, swarming, pack hunting
**High Kingdom** - Authority/defense, opponent disruption
**Artificer Order** - Scrapping, deck thinning, upgrades

### Dependencies

**On Track Alpha:**
- 5-006 needs 1-004 (trade row) for selection logic
- 4-009 needs game engine for balance validation

**On Track Beta:**
- 6-003 needs 3-006 (style preferences) for art regeneration
- Phase demos (5-010, 6-010) need 3-010 (Phase 3 Demo)

### Next Steps
1. 6-009 (image caching/persistence) - can start immediately
2. 5-006 (trade row logic) - blocked on Track Alpha 1-004
3. Phase demos (5-010, 6-010) - blocked on respective dependencies
