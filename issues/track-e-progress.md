# Track E Progress: AI Integration

## Goal
Implement LLM narrative generation and ComfyUI visual generation systems.
This track enables AI-driven game narration and dynamic card art generation.

## Status: In Progress (Pre-Alpha Complete)

## Current Checkpoint: Post-ALPHA

## Issues by Phase

### Pre-Alpha (API Clients) - COMPLETE
| ID | Description | Status |
|----|-------------|--------|
| 5-001 | LLM API Client | completed |
| 6-001 | ComfyUI API Client | completed |

### Alpha → Beta (Prompt Infrastructure)
| ID | Description | Status | Dependencies |
|----|-------------|--------|--------------|
| 5-002 | Prompt Network Structure | completed | 5-001 |
| 5-003 | World State Prompt | pending | 5-002 |
| 6-002 | Card Image Prompt Builder | pending | 6-001, 4-001 |
| 6-008 | Style Transfer Prompts | pending | 6-002 |

### Beta → Gamma (Context Management)
| ID | Description | Status | Dependencies |
|----|-------------|--------|--------------|
| 5-004 | Force Description Prompts | pending | 5-003 |
| 5-005 | Event Narration Prompts | pending | 5-004 |
| 5-007 | Context Window Management | pending | 5-005 |
| 5-008 | Narrative Caching | pending | 5-005 |

### Gamma → Delta (Visual Pipeline)
| ID | Description | Status | Dependencies |
|----|-------------|--------|--------------|
| 6-003 | Dynamic Art Regeneration | pending | 6-002, 3-006 |
| 6-004 | Upgrade Visualization | pending | 6-003 |
| 6-005 | Battle Canvas Manager | pending | 6-001 |
| 6-006 | Inpainting Region Selection | pending | 6-005 |
| 6-007 | Scene Composition Rules | pending | 6-006 |

### Delta → Omega (Full Integration)
| ID | Description | Status | Dependencies |
|----|-------------|--------|--------------|
| 5-006 | Trade Row Selection Logic | pending | 5-005, 1-004 |
| 5-009 | Coherence Recovery | pending | 5-006, 5-008 |
| 5-010 | Phase 5 Demo | pending | 5-009, 3-010 |
| 6-009 | Image Caching/Persistence | pending | 6-004 |
| 6-010 | Phase 6 Demo | pending | 6-009, 5-010 |

## Completed Issues: 3/20

## Deliverables Progress

### Phase 5 (LLM Integration)
- [x] `src/llm/01-api-client.h` - API client header
- [x] `src/llm/01-api-client.c` - API client implementation
- [x] `tests/test-llm.c` - Unit tests (9 passing)
- [x] `src/llm/02-prompts.h` - Prompt network header
- [x] `src/llm/02-prompts.c` - Prompt templates and interpolation
- [x] `tests/test-prompts.c` - Unit tests (15 passing)
- [ ] Context window management
- [ ] Narrative caching

### Phase 6 (Visual Generation)
- [x] `src/visual/01-comfyui-client.h` - ComfyUI client header
- [x] `src/visual/01-comfyui-client.c` - ComfyUI client implementation
- [x] `tests/test-comfyui.c` - Unit tests (12 passing)
- [ ] Card image prompt builder
- [ ] Dynamic art regeneration
- [ ] Battle canvas manager

## Dependencies on Other Tracks

### Waiting for Track A
- 1-004 (Trade Row) needed for 5-006 (Trade Row Selection Logic)

### Waiting for Track C
- 3-006 (Client Style Preferences) needed for 6-003 (Dynamic Art Regeneration)
- 3-010 (Phase 3 Demo) needed for 5-010, 6-010 (Phase Demos)

### Waiting for Track D
- 4-001 (Card JSON Schema) needed for 6-002 (Card Image Prompt Builder)

## Notes

### Pre-Alpha Completion (2026-02-10)
Both API clients (5-001, 6-001) are implemented and tested.
- LLM client: OpenAI-compatible, retry with exponential backoff
- ComfyUI client: Async workflow submission with polling

### Next Steps
The next issues (5-002, 6-002) can proceed once the configuration system
dependency (2-001) is confirmed working, which it is. However, 6-002
also needs 4-001 (Card JSON Schema) from Track D.

5-002 (Prompt Network Structure) can begin immediately.
