# Symbeline Realms - Parallelized Development Roadmap

This document reorganizes the development into 3 parallel tracks with synchronization
checkpoints. This enables concurrent development by multiple agents or developers
working on independent subsystems.

## Track Overview (3-Track Structure)

```
TIME ──────────────────────────────────────────────────────────────────────────►

     ╔═══════════════╦═══════════════╦═══════════════╦═══════════════╗
     ║   ALPHA       ║    BETA       ║    GAMMA      ║    DELTA      ║
     ║  Structures   ║  Serialization║   Protocol    ║   Playable    ║
     ╠═══════════════╬═══════════════╬═══════════════╬═══════════════╣
TRACK ALPHA ─●───────────●───────────────●───────────────●────────────►
Core Engine  │           │               │               │
(was A)      │           │               │               │
             │           │               │               │
TRACK BETA ──┼───●───────┼───────────────●───────────────●────────────►
Infra + UI   │   │       │               │               │
(was B+C)    │   │       │               │               │
             │           │               │               │
TRACK GAMMA ─●───┼───────┼───────●───────┼───────────────┼────●───────►
AI + Content │   │       │       │       │               │    │
(was D+E)    │   │       │       │       │               │    │
             ▼   ▼       ▼       ▼       ▼               ▼    ▼
```

## Current Status (2026-02-12)

| Track | Status | In Progress | Blocking |
|-------|--------|-------------|----------|
| **Alpha-2** | Ready | 1-013 (demo) | - |
| **Beta-2** | Protocol phase | 2-010 | - |
| **Gamma-2** | Demos blocked | 4-009 can start | Needs 3-010 |

### Phase 1 Summary (Complete)

The first parallel development phase achieved:
- **Track Alpha:** Core engine complete (1-001 through 1-012)
- **Track Beta:** All rendering/input complete (25 issues)
- **Track Gamma:** All AI/content implementation complete (33 issues)

Total: 70 issues completed, 305 core tests + 235 AI tests = 540 tests passing

## Synchronization Checkpoints

| Checkpoint | Name           | Required Deliverables                              |
|------------|----------------|---------------------------------------------------|
| ALPHA      | Structures     | Card/Player/Game structs defined (1-001, 1-002)   |
| BETA       | Serialization  | Gamestate JSON export working (1-012)             |
| GAMMA      | Protocol       | Message types finalized (2-005)                   |
| DELTA      | Playable       | Full game loop over network (Phase 1-3 complete)  |
| EPSILON    | Content        | All cards/factions balanced (Phase 4 complete)    |
| OMEGA      | Complete       | AI narrative + visuals integrated (Phase 5-6)     |

---

## Track Alpha: Core Engine

**Consolidates:** Original Track A (Game Logic)

Pure C implementation of game rules. No external dependencies beyond cJSON.
This track is the critical path - other tracks depend on its data structures.

### Pre-Alpha (Foundation)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 1-001   | Card data structure            | COMPLETE  |
| 1-002   | Deck management system         | COMPLETE  |
| 1-003   | Player state management        | COMPLETE  |

### Alpha → Beta (Core Mechanics)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 1-004   | Trade row implementation       | COMPLETE  |
| 1-005   | Turn loop structure            | COMPLETE  |
| 1-006   | Basic combat resolution        | COMPLETE  |
| 1-007*  | Card effect parser             | COMPLETE  |
| 1-008*  | Auto-draw resolution system    | **IN PROGRESS** |
| 1-009   | Deck flow tracker (d10/d4)     | COMPLETE  |

### Beta → Gamma (Advanced Mechanics)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 1-010   | Base card type                 | COMPLETE  |
| 1-011   | Spawning mechanics             | COMPLETE  |
| 1-012   | Gamestate serialization        | **IN PROGRESS** |

### Gamma → Delta (Demo)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 1-013   | Phase 1 Demo                   | pending   |

**Track Alpha Deliverables:**
- `src/core/` - Complete game logic library
- `tests/test-core.c` - Unit tests for all mechanics
- `issues/completed/demos/phase-1-demo.sh` - CLI game loop

---

## Track Beta: Infrastructure

**Consolidates:** Original Track B (Server) + Track C (Client Rendering)

Network layer and client implementations. Server can begin immediately;
protocol integration waits for BETA checkpoint from Track Alpha.

### Pre-Alpha (Configuration + Rendering Foundation)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 2-001   | Configuration system           | COMPLETE  |
| 3-003   | Wasm build configuration       | COMPLETE  |
| 3-001a  | Terminal UI initialization     | COMPLETE  |
| 3-004a  | Canvas infrastructure          | COMPLETE  |

### Alpha → Beta (Server + Rendering Core)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 2-002   | HTTP server (libwebsockets)    | COMPLETE  |
| 2-004*  | SSH server integration         | COMPLETE  |
| 3-001b  | Terminal window rendering      | COMPLETE  |
| 3-001c  | Terminal formatting            | COMPLETE  |
| 3-004b  | Card rendering (Canvas)        | COMPLETE  |
| 3-004c  | Game zones (Canvas)            | COMPLETE  |

### Beta → Gamma (Input Systems)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 3-001d  | Terminal input/resize          | COMPLETE  |
| 3-002   | Terminal input system          | COMPLETE  |
| 3-004d  | Status/narrative panels        | COMPLETE  |
| 3-005   | Browser input handler          | COMPLETE  |

### Gamma → Delta (Protocol + Integration)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 3-006*  | Client style preferences       | COMPLETE  |
| 3-007   | Draw order interface           | COMPLETE  |
| 3-008*  | Animation system               | COMPLETE  |
| 3-009   | Narrative display              | COMPLETE  |
| 2-005*  | Protocol implementation        | **BLOCKED** (needs 1-012) |
| 2-003   | WebSocket handler              | pending   |
| 2-006   | Connection manager             | pending   |
| 2-007   | Game session management        | pending   |
| 2-008   | Hidden information handling    | pending   |
| 2-009   | Input validation               | pending   |
| 2-010   | Phase 2 Demo                   | pending   |
| 3-010   | Phase 3 Demo                   | pending   |

**Track Beta Deliverables:**
- `src/net/` - Complete networking layer
- `src/client/terminal.c` - ncurses TUI client
- `src/client/wasm/` - Browser client (Canvas + WebSocket)
- `assets/web/` - Static HTML/CSS/JS
- `config/server.json.example` - Configuration template
- `issues/completed/demos/phase-2-demo.sh` - Two-player network game
- `issues/completed/demos/phase-3-demo.sh` - Dual-client game

---

## Track Gamma: AI & Content

**Consolidates:** Original Track D (Game Content) + Track E (AI Integration)

Card design, balance tools, LLM narrative, and visual generation.
Content can proceed independently; AI integration uses Track Alpha mechanics.

### Pre-Alpha (Schema + API Clients)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 4-001   | Card JSON schema               | COMPLETE  |
| 5-001   | LLM API client                 | COMPLETE  |
| 6-001   | ComfyUI API client             | COMPLETE  |

### Alpha → Beta (Content + Prompts)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 4-002   | Faction: Merchant Guilds       | COMPLETE  |
| 4-003   | Faction: The Wilds             | COMPLETE  |
| 4-004   | Faction: High Kingdom          | COMPLETE  |
| 4-005   | Faction: Artificer Order       | COMPLETE  |
| 4-006   | Starting deck definition       | COMPLETE  |
| 4-007   | Neutral trade deck cards       | COMPLETE  |
| 4-008   | Upgrade effect cards           | COMPLETE  |
| 5-002   | Prompt network structure       | COMPLETE  |
| 5-003   | World state prompt             | COMPLETE  |
| 6-002   | Card image prompt builder      | COMPLETE  |
| 6-008   | Style transfer prompts         | pending   |

### Beta → Gamma (Context Management)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 5-004   | Force description prompts      | COMPLETE  |
| 5-005   | Event narration prompts        | COMPLETE  |
| 5-007*  | Context window management      | COMPLETE  |
| 5-008   | Narrative caching              | **IN PROGRESS** |

### Gamma → Delta (Visual Pipeline)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 6-003*  | Dynamic art regeneration       | pending   |
| 6-004   | Upgrade visualization          | pending   |
| 6-005   | Battle canvas manager          | pending   |
| 6-006*  | Inpainting region selection    | pending   |
| 6-007   | Scene composition rules        | pending   |

### Delta → Omega (Full Integration)
| Issue   | Description                    | Status    |
|---------|--------------------------------|-----------|
| 4-009   | Card balance validator         | pending   |
| 4-010   | Phase 4 Demo                   | pending   |
| 5-006   | Trade row selection logic      | pending   |
| 5-009   | Coherence recovery             | pending   |
| 5-010   | Phase 5 Demo                   | pending   |
| 6-009   | Image caching/persistence      | pending   |
| 6-010   | Phase 6 Demo                   | pending   |

**Track Gamma Deliverables:**
- `assets/cards/*.json` - All card definitions (65 cards complete)
- `src/tools/balance-validator.c` - Balance analysis utility
- `src/llm/` - Complete LLM integration
- `src/visual/` - Image generation client
- `issues/completed/demos/phase-4-demo.sh` - Full game with all factions
- `issues/completed/demos/phase-5-demo.sh` - AI-narrated game
- `issues/completed/demos/phase-6-demo.sh` - Visual generation demo

---

## Checkpoint Details

### ALPHA: Data Structures Defined [COMPLETE]

**Completed deliverables:**
- Card struct with all fields (id, cost, effects, faction, upgrades)
- Player struct with authority, d10/d4, resources
- Deck/Hand/Discard containers working
- JSON schema for cards documented

### BETA: Gamestate Serialization [IN PROGRESS]

**Required before passing:**
- Full gamestate can be exported to JSON
- All game mechanics working (effects, combat, bases)
- Round-trip serialization tested

**Blocking:**
- Track Beta (2-005) needs gamestate format for protocol
- Track Beta can begin protocol integration after this completes

**In Progress:** 1-012

### GAMMA: Protocol Finalized

**Required before passing:**
- All message types defined (server→client, client→server)
- Validation rules implemented
- Hidden information handling specified

### DELTA: Playable Game

**Required before passing:**
- Complete game loop working over network
- Both terminal and browser clients functional
- Two players can complete a full game

### EPSILON: Content Complete

**Required before passing:**
- All 4 factions implemented and balanced
- Starting decks and neutral cards complete
- Balance validator passes all checks

### OMEGA: Complete Experience

**Required before passing:**
- AI narrative working for all game events
- Dynamic card art regenerating properly
- Battle canvas with inpainting functional
- All phase demos passing

---

## Recommended Team Assignments

### Solo Developer
Work tracks sequentially: Alpha → Beta → Gamma
Use checkpoints to validate each phase before continuing.

### Two Developers
- **Dev 1:** Track Alpha (core engine)
- **Dev 2:** Track Beta (server + clients) + Track Gamma (content + AI)
- **Sync at:** BETA, GAMMA, DELTA

### Three Developers (Current)
- **Dev 1:** Track Alpha (1-008, 1-012)
- **Dev 2:** Track Beta (waiting on Alpha)
- **Dev 3:** Track Gamma (5-008)
- **Sync at:** All checkpoints

---

## Cross-Track Dependencies Summary

```
Track Alpha (Core Engine)
    │
    ├──► 1-001 (Card struct) ──────► Track Gamma (4-001 schema)
    │                               Track Gamma (6-002 prompt builder)
    │
    ├──► 1-004 (Trade row) ────────► Track Gamma (5-006 trade row logic)
    │
    └──► 1-012 (Serialization) ───► Track Beta (2-005 protocol)
                                    Track Beta (2-008 hidden info)

Track Beta (Infrastructure)
    │
    ├──► 2-001 (Config) ──────────► Track Gamma (5-001, 6-001 API clients)
    │
    ├──► 2-005 (Protocol) ────────► Track Beta (3-007 draw order)
    │
    └──► 3-006 (Style prefs) ─────► Track Gamma (6-003 art regen)
```

---

## Issue Notation

Issues marked with `*` have been split into sub-issues:
- 1-007* → 1-007a through 1-007f (Card effect system)
- 1-008* → 1-008a through 1-008d (Auto-draw resolution)
- 2-004* → 2-004a through 2-004d (SSH server)
- 2-005* → 2-005a through 2-005d (Protocol)
- 3-001* → 3-001a through 3-001d (Terminal renderer)
- 3-004* → 3-004a through 3-004d (Canvas renderer)
- 3-006* → 3-006a through 3-006c (Style preferences)
- 3-008* → 3-008a through 3-008c (Animation system)
- 5-007* → 5-007a through 5-007c (Context window)
- 6-003* → 6-003a through 6-003d (Dynamic art)
- 6-006* → 6-006a through 6-006c (Inpainting regions)

---

## Progress Tracking

Each track maintains its own progress file:
- `issues/track-alpha-progress.md` - Core Engine
- `issues/track-beta-progress.md` - Infrastructure (Server + Client)
- `issues/track-gamma-progress.md` - AI & Content

Legacy track files (kept for history):
- `issues/track-b-progress.md` - Server Infrastructure (old)
- `issues/track-c-progress.md` - Client Rendering (old)
- `issues/track-d-progress.md` - Game Content (old)
- `issues/track-e-progress.md` - AI Integration (old)

---

---

## Phase 2: Track Assignments (2026-02-12)

With Phase 1 complete, development enters Phase 2 with revised track assignments.
Each track continues with the "-2" suffix to denote the second development phase.

### Track Alpha-2: Core Engine Completion

**Focus:** Phase 1 Demo
**Progress File:** `issues/track-alpha-2-progress.md`

| Issue | Description | Status | Priority |
|-------|-------------|--------|----------|
| 1-013 | Phase 1 Demo | READY | HIGH |

**Notes:**
- Only one issue remaining
- All dependencies satisfied
- Can complete quickly to unblock other work

### Track Beta-2: Network Protocol & Integration

**Focus:** Protocol implementation and demos
**Progress File:** `issues/track-beta-2-progress.md`

| Issue | Description | Status | Priority |
|-------|-------------|--------|----------|
| 2-005* | Protocol implementation | UNBLOCKED | CRITICAL |
| 2-003 | WebSocket handler | blocked on 2-005 | HIGH |
| 2-006 | Connection manager | blocked on 2-003 | HIGH |
| 2-007 | Game session management | blocked on 2-006 | HIGH |
| 2-008 | Hidden information handling | blocked on 2-007 | HIGH |
| 2-009 | Input validation | blocked on 2-005 | HIGH |
| 2-010 | Phase 2 Demo | **IN PROGRESS** | HIGH |
| 3-010 | Phase 3 Demo | blocked on 2-010 | MEDIUM |
| 3-011* | WASM JS elimination | independent | MEDIUM |

**Critical Path:**
```
2-005 ──► 2-003 ──► 2-006 ──► 2-007 ──► 2-008 ──► 2-010 ──► 3-010
   └──────► 2-009 ─────────────────────────────────┘
```

**Notes:**
- 2-005 (Protocol) is now unblocked - critical path item
- 2-010 is in progress
- 3-011 (WASM elimination) is large but can be parallel work

### Track Gamma-2: Demos & Balance

**Focus:** Balance tools and phase demos
**Progress File:** `issues/track-gamma-2-progress.md`

| Issue | Description | Status | Priority |
|-------|-------------|--------|----------|
| 4-009 | Card balance validator | READY | MEDIUM |
| 4-010 | Phase 4 Demo | blocked on 4-009, 3-010 | LOW |
| 5-010 | Phase 5 Demo | blocked on 3-010 | MEDIUM |
| 6-010 | Phase 6 Demo | blocked on 5-010 | MEDIUM |

**Notes:**
- 4-009 can start immediately (only needs core engine)
- All other issues blocked on Track Beta-2 completing 3-010
- This track has the most blocked issues

---

## Phase 2 Critical Path

```
                    CRITICAL PATH
                         │
    Track Alpha-2        │         Track Gamma-2
         │               │              │
      1-013 ─────────────┼──────────► 4-009
    (Phase 1)            │           (Balance)
         │               │              │
         ▼               │              ▼
    Track Beta-2         │           4-010
         │               │         (Phase 4)
      2-005 ◄────────────┘              │
    (Protocol)                          │
         │                              │
         ▼                              │
   2-003 → 2-006                        │
   2-009   2-007                        │
         │  │                           │
         ▼  ▼                           │
      2-008                             │
         │                              │
         ▼                              │
      2-010 ─────────────────────────► 3-010
    (Phase 2)                        (Phase 3)
         │                              │
         │                              ├──────► 5-010 ──► 6-010
         │                              │      (Phase 5)  (Phase 6)
         └──────────────────────────────┘
```

---

## Recommended Team Assignments (Phase 2)

### Solo Developer
Work in order: Alpha-2 → Beta-2 → Gamma-2
1. Complete 1-013 first (quick win)
2. Work through Beta-2 critical path (2-005 → 2-010)
3. Complete 3-010, then Gamma-2 demos

### Two Developers
- **Dev 1:** Track Beta-2 (critical path: 2-005 through 3-010)
- **Dev 2:** Track Alpha-2 (1-013) → Track Gamma-2 (4-009 → demos)
- **Sync at:** After 3-010 completion

### Three Developers
- **Dev 1:** Track Alpha-2 (1-013) then assist Beta-2
- **Dev 2:** Track Beta-2 (2-005 through 2-010, then 3-010)
- **Dev 3:** Track Gamma-2 (4-009) then assist Beta-2 or work on 3-011
- **Sync at:** After each demo completion

---

## Starting Points (Phase 2)

| Track     | Current Work | Next After Current | Blocked By |
|-----------|--------------|-------------------|------------|
| Alpha-2   | 1-013 (demo) | (complete) | - |
| Beta-2    | 2-010 (in progress) | 3-010 | - |
| Gamma-2   | 4-009 (balance) | 5-010, 4-010, 6-010 | 3-010 |

**Critical Path:** 2-005 → 2-003 → 2-006 → 2-007 → 2-008 → 2-010 → 3-010 → All demos
