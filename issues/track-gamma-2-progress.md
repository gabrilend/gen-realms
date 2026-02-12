# Track Gamma-2: Demos & Balance - Progress

This document tracks the progress of Track Gamma-2, the second parallel development
phase for AI & Content. This track focuses on completing the phase demos and
the balance validation tools.

## Track Overview

Track Gamma-2 completes the content and AI integration with:
- Card balance validator tool
- Phase 4 Demo (complete game with all factions)
- Phase 5 Demo (AI-narrated game)
- Phase 6 Demo (visual generation)

## Current Status: Waiting on Track Beta-2

All core implementation is complete. Track Gamma-2 is blocked on Track Beta-2
completing the network integration (2-010, 3-010) before demos can proceed.

### In Progress

| Issue | Description | Status | Developer |
|-------|-------------|--------|-----------|
| - | - | - | - |

### Completed Issues (Previous Phase - Content)

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

### Completed Issues (Previous Phase - LLM)

| Issue | Description | Date |
|-------|-------------|------|
| 5-001 | LLM API client | 2026-02-10 |
| 5-002 | Prompt network structure | 2026-02-11 |
| 5-003 | World state prompt | 2026-02-11 |
| 5-004 | Force description prompts | 2026-02-11 |
| 5-005 | Event narration prompts | 2026-02-11 |
| 5-006 | Trade row selection logic | 2026-02-11 |
| 5-007a | Context manager structure | 2026-02-11 |
| 5-007b | Context entry management | 2026-02-11 |
| 5-007c | Context summarization | 2026-02-11 |
| 5-008 | Narrative caching | 2026-02-11 |
| 5-009 | Coherence recovery | 2026-02-11 |

### Completed Issues (Previous Phase - Visual)

| Issue | Description | Date |
|-------|-------------|------|
| 6-001 | ComfyUI API client | 2026-02-10 |
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
| 6-009 | Image caching/persistence | 2026-02-11 |

### Pending Issues

| Issue | Description | Dependencies | Priority |
|-------|-------------|--------------|----------|
| 4-009 | Card balance validator | Core engine (done) | MEDIUM |
| 4-010 | Phase 4 Demo | 4-009, 3-010 (Beta-2) | BLOCKED |
| 5-010 | Phase 5 Demo | 5-009 (done), 3-010 (Beta-2) | BLOCKED |
| 6-010 | Phase 6 Demo | 6-009 (done), 5-010 | BLOCKED |

## Issue Details

### 4-009: Card Balance Validator (CAN START)

**Status:** Ready to implement
**Dependencies:** Core game engine (complete)

**Deliverables:**
- `src/tools/balance-validator.c` - Balance analysis utility
- Calculate value-per-cost ratios
- Compare faction power curves
- Identify outliers (over/underpowered cards)
- Output balance report

**Acceptance Criteria:**
- Utility runs on full card database
- Value calculations are reasonable
- Outliers correctly identified
- Report is human-readable
- Can inform card adjustments

### 4-010: Phase 4 Demo

**Status:** Blocked on 4-009 and 3-010 (Track Beta-2)
**Dependencies:** 4-009 (balance validator), 3-010 (Phase 3 Demo)

**Deliverables:**
- Complete game with all 4 factions playable
- Balance report showing card distribution
- All card types working

### 5-010: Phase 5 Demo

**Status:** Blocked on 3-010 (Track Beta-2)
**Dependencies:** 5-009 (done), 3-010 (Phase 3 Demo)

**Deliverables:**
- Full game with AI-generated narrative
- LLM connected and generating for all events
- World state tracking throughout game
- Context window management visible
- Coherence maintained through full game

**Expected Output:**
```
=== SYMBELINE REALMS: PHASE 5 DEMO ===

Connecting to LLM endpoint: http://localhost:11434/v1
Model: llama3

--- GAME START ---
[NARRATIVE] The contested realm of Symbeline stretches...

Total LLM calls: 47
Cache hits: 12 (25.5%)
Coherence recoveries: 0
```

### 6-010: Phase 6 Demo

**Status:** Blocked on 5-010
**Dependencies:** 6-009 (done), 5-010

**Deliverables:**
- Full visual generation demo
- ComfyUI generating card art
- Battle canvas with progressive inpainting
- Style-consistent faction visuals
- Image caching statistics

**Expected Output:**
```
=== SYMBELINE REALMS: PHASE 6 DEMO ===

Connecting to ComfyUI: http://localhost:8188
Model: sd_xl_base_1.0

Generating card art...
[1/5] Dire Bear (Wilds) - 3.2s
...
ComfyUI Calls: 12
Cache Hits: 3 (25%)
```

## Dependency Chain

```
                          Track Beta-2
                              │
                              ▼
4-009 (Balance) ────────► 3-010 (Phase 3) ◄──── 5-009 (DONE)
      │                       │                      │
      │                       ├──────────────────────┘
      │                       │
      ▼                       ▼
    4-010              5-010 (Phase 5)
  (Phase 4)                   │
                              ▼
                        6-009 (DONE)
                              │
                              ▼
                        6-010 (Phase 6)
```

## Checkpoint Status

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA | COMPLETE | API clients ready |
| BETA | COMPLETE | Content and prompts done |
| GAMMA | COMPLETE | Context management done |
| DELTA | BLOCKED | Needs Track Beta-2 |
| EPSILON | BLOCKED | Needs balance testing |
| OMEGA | BLOCKED | Needs full integration |

## Statistics

**Content Issues:** 8 complete, 2 pending (4-009, 4-010)
**AI-LLM Issues:** 11 complete, 1 pending (5-010)
**AI-Visual Issues:** 14 complete, 1 pending (6-010)
**Total:** 33 complete, 4 pending

**Cards Created:** 65 total
- Starting: 2 cards
- Neutral: 6 cards
- Merchant Guilds: 14 cards
- The Wilds: 14 cards
- High Kingdom: 14 cards
- Artificer Order: 15 cards

**Unit Tests Passing:** 235 total
- LLM client: 9
- Prompts: 15
- World state: 12
- Force description: 18
- Event narration: 23
- Context manager: 37
- Narrative cache: 20
- ComfyUI client: 12
- Card prompts: 24
- Image cache: 16
- Trade select: 26
- Coherence: 23

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
- [x] `src/llm/07-narrative-cache.h/c` - Caching
- [x] `src/llm/08-trade-select.h/c` - Trade row selection
- [x] `src/llm/09-coherence.h/c` - Coherence recovery
- [ ] `issues/completed/demos/phase-5-demo.sh` - AI-narrated game

### AI - Visual (Phase 6)
- [x] `src/visual/01-comfyui-client.h/c` - ComfyUI client
- [x] `src/visual/02-card-prompts.h/c` - Card image prompts
- [x] `assets/web/art-tracker.js` - Regeneration tracking
- [x] `assets/web/style-merger.js` - Style guide integration
- [x] `assets/web/generation-queue.js` - Generation queue
- [x] `assets/web/image-cache.js` - Cache invalidation
- [x] `assets/web/upgrade-viz.js` - Upgrade visualization
- [x] `assets/web/battle-canvas.js` - Battle canvas manager
- [x] `assets/web/region-selector.js` - Inpainting region selection
- [x] `assets/web/scene-composition.js` - Scene composition rules
- [x] `assets/web/style-transfer.js` - Style transfer prompts
- [x] `src/visual/03-image-cache.h/c` - Image caching/persistence
- [ ] `issues/completed/demos/phase-6-demo.sh` - Visual generation demo

## Dependencies on Other Tracks

**From Track Alpha-2:**
- Core game engine for balance validator (available)

**From Track Beta-2:**
- 3-010 (Phase 3 Demo) required for all demos in this track

## Notes

### 2026-02-12: Track Gamma-2 Planning

Track Gamma-2 is mostly blocked on Track Beta-2 completing the network
integration. However, 4-009 (balance validator) can proceed immediately
as it only needs the core game engine which is complete.

**Recommended approach:**
1. Start 4-009 (balance validator) immediately
2. Wait for Track Beta-2 to complete 3-010
3. Then proceed with 5-010, 4-010, and finally 6-010

### Work That Can Start Now

Only **4-009** (Card Balance Validator) can start immediately. It analyzes
the card database using the core game engine to calculate power curves
and identify balance outliers.
