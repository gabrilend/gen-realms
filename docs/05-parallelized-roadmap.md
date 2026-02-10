# Symbeline Realms - Parallelized Development Roadmap

This document reorganizes the sequential phase roadmap into 5 parallel development
tracks with synchronization checkpoints. This enables concurrent development by
multiple agents or developers working on independent subsystems.

## Track Overview

```
TIME ──────────────────────────────────────────────────────────────────────────►

     ╔═══════════════╦═══════════════╦═══════════════╦═══════════════╗
     ║   ALPHA       ║    BETA       ║    GAMMA      ║    DELTA      ║
     ║  Structures   ║  Serialization║   Protocol    ║   Playable    ║
     ╠═══════════════╬═══════════════╬═══════════════╬═══════════════╣
TRACK A ────●────────────●───────────────●───────────────●────────────►
Game Logic  │            │               │               │
            │            │               │               │
TRACK B ────┼────●───────┼───────────────●───────────────●────────────►
Server      │    │       │               │               │
Infra       │    │       │               │               │
            │    │       │               │               │
TRACK C ────┼────┼───────┼───────●───────●───────────────●────────────►
Client      │    │       │       │       │               │
Rendering   │    │       │       │       │               │
            │    │       │       │       │               │
TRACK D ────●────┼───────┼───────┼───────┼───────────────●────────────►
Game        │    │       │       │       │               │
Content     │    │       │       │       │               │
            │    │       │       │       │               │
TRACK E ────┼────●───────┼───────┼───────┼───────────────┼────●───────►
AI          │    │       │       │       │               │    │
Integration │    │       │       │       │               │    │
            ▼    ▼       ▼       ▼       ▼               ▼    ▼
```

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

## Track A: Game Logic (Core Engine)

Pure C implementation of game rules. No external dependencies beyond cJSON.
This track is the critical path - other tracks depend on its data structures.

### Pre-Alpha (Foundation)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 1-001   | Card data structure            | None         |
| 1-002   | Deck management system         | 1-001        |
| 1-003   | Player state management        | 1-001, 1-002 |

### Alpha → Beta (Core Mechanics)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 1-004   | Trade row implementation       | 1-001             |
| 1-005   | Turn loop structure            | 1-003             |
| 1-006   | Basic combat resolution        | 1-003             |
| 1-007*  | Card effect parser             | 1-001             |
| 1-008*  | Auto-draw resolution system    | 1-005, 1-007      |
| 1-009   | Deck flow tracker (d10/d4)     | 1-003             |

### Beta → Gamma (Advanced Mechanics)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 1-010   | Base card type                 | 1-001             |
| 1-011   | Spawning mechanics             | 1-010, 1-002      |
| 1-012   | Gamestate serialization        | All above         |

### Gamma → Delta (Demo)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 1-013   | Phase 1 Demo                   | 1-012             |

**Track A Deliverables:**
- `src/core/` - Complete game logic library
- `tests/test-game.c` - Unit tests for all mechanics
- `issues/completed/demos/phase-1-demo.sh` - CLI game loop

---

## Track B: Server Infrastructure

Network layer enabling multi-client connections. Can begin configuration and
HTTP setup immediately; WebSocket/SSH integration waits for gamestate protocol.

### Pre-Alpha (Configuration)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 2-001   | Configuration system           | None         |

### Alpha → Beta (HTTP Foundation)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 2-002   | HTTP server (libwebsockets)    | 2-001        |
| 2-004*  | SSH server integration         | 2-001        |

### Beta → Gamma (Protocol)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 2-005*  | Protocol implementation        | 1-012 (BETA)      |
| 2-003   | WebSocket handler              | 2-002, 2-005      |
| 2-006   | Connection manager             | 2-003, 2-004      |

### Gamma → Delta (Session Management)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 2-007   | Game session management        | 2-006             |
| 2-008   | Hidden information handling    | 2-007, 1-012      |
| 2-009   | Input validation               | 2-005             |
| 2-010   | Phase 2 Demo                   | All above         |

**Track B Deliverables:**
- `src/net/` - Complete networking layer
- `config/server.json.example` - Configuration template
- `issues/completed/demos/phase-2-demo.sh` - Two-player network game

---

## Track C: Client Rendering

Terminal and browser client implementations. Can develop rendering logic with
mock gamestate data, then integrate protocol after GAMMA checkpoint.

### Pre-Alpha (Parallel Start with Mock Data)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 3-003   | Wasm build configuration       | None         |
| 3-001a  | Terminal UI initialization     | None         |
| 3-004a  | Canvas infrastructure          | 3-003        |

### Alpha → Beta (Rendering Core)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 3-001b  | Terminal window rendering      | 3-001a            |
| 3-001c  | Terminal formatting            | 3-001b            |
| 3-004b  | Card rendering (Canvas)        | 3-004a, 1-001*    |
| 3-004c  | Game zones (Canvas)            | 3-004b            |

### Beta → Gamma (Input Systems)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 3-001d  | Terminal input/resize          | 3-001c            |
| 3-002   | Terminal input system          | 3-001d            |
| 3-004d  | Status/narrative panels        | 3-004c            |
| 3-005   | Browser input handler          | 3-004d            |

### Gamma → Delta (Integration)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 3-006*  | Client style preferences       | 3-005             |
| 3-007   | Draw order interface           | 3-005, 2-005*     |
| 3-008*  | Animation system               | 3-004c            |
| 3-009   | Narrative display              | 3-004d            |
| 3-010   | Phase 3 Demo                   | All above, 2-010  |

**Track C Deliverables:**
- `src/client/terminal.c` - ncurses TUI client
- `src/client/wasm/` - Browser client (Canvas + WebSocket)
- `assets/web/` - Static HTML/CSS/JS
- `issues/completed/demos/phase-3-demo.sh` - Dual-client game

---

## Track D: Game Content

Card design and balance work. Requires only the card JSON schema from Track A.
Can proceed largely independently, with periodic balance testing against the
evolving engine.

### Pre-Alpha (Schema)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 4-001   | Card JSON schema               | 1-001 (struct)    |

### Alpha → Delta (Parallel Content Creation)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 4-002   | Faction: Merchant Guilds       | 4-001        |
| 4-003   | Faction: The Wilds             | 4-001        |
| 4-004   | Faction: High Kingdom          | 4-001        |
| 4-005   | Faction: Artificer Order       | 4-001        |
| 4-006   | Starting deck definition       | 4-001        |
| 4-007   | Neutral trade deck cards       | 4-001        |
| 4-008   | Upgrade effect cards           | 4-001        |

### Delta → Epsilon (Balance)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 4-009   | Card balance validator         | 4-002-4-008       |
| 4-010   | Phase 4 Demo                   | 4-009, 3-010      |

**Track D Deliverables:**
- `assets/cards/*.json` - All card definitions
- `src/tools/balance-validator.c` - Balance analysis utility
- `issues/completed/demos/phase-4-demo.sh` - Full game with all factions

**Note:** Track D can be assigned to a game designer or separate agent with
minimal C knowledge - only JSON authoring is required until the balance
validator implementation.

---

## Track E: AI Integration

LLM narrative and visual generation systems. API clients can be developed
early; full integration requires playable game (DELTA checkpoint).

### Pre-Alpha (API Clients)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 5-001   | LLM API client                 | 2-001 (config)    |
| 6-001   | ComfyUI API client             | 2-001 (config)    |

### Alpha → Beta (Prompt Infrastructure)
| Issue   | Description                    | Dependencies |
|---------|--------------------------------|--------------|
| 5-002   | Prompt network structure       | 5-001        |
| 5-003   | World state prompt             | 5-002        |
| 6-002   | Card image prompt builder      | 6-001, 4-001 |
| 6-008   | Style transfer prompts         | 6-002        |

### Beta → Gamma (Context Management)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 5-004   | Force description prompts      | 5-003             |
| 5-005   | Event narration prompts        | 5-004             |
| 5-007*  | Context window management      | 5-005             |
| 5-008   | Narrative caching              | 5-005             |

### Gamma → Delta (Visual Pipeline)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 6-003*  | Dynamic art regeneration       | 6-002, 3-006      |
| 6-004   | Upgrade visualization          | 6-003             |
| 6-005   | Battle canvas manager          | 6-001             |
| 6-006*  | Inpainting region selection    | 6-005             |
| 6-007   | Scene composition rules        | 6-006             |

### Delta → Omega (Full Integration)
| Issue   | Description                    | Dependencies      |
|---------|--------------------------------|-------------------|
| 5-006   | Trade row selection logic      | 5-005, 1-004      |
| 5-009   | Coherence recovery             | 5-006, 5-008      |
| 5-010   | Phase 5 Demo                   | 5-009, 3-010      |
| 6-009   | Image caching/persistence      | 6-004             |
| 6-010   | Phase 6 Demo                   | 6-009, 5-010      |

**Track E Deliverables:**
- `src/llm/` - Complete LLM integration
- `src/client/wasm/comfyui.c` - Image generation client
- `issues/completed/demos/phase-5-demo.sh` - AI-narrated game
- `issues/completed/demos/phase-6-demo.sh` - Visual generation demo

---

## Checkpoint Details

### ALPHA: Data Structures Defined

**Required before passing:**
- Card struct with all fields (id, cost, effects, faction, upgrades)
- Player struct with authority, d10/d4, resources
- Deck/Hand/Discard containers working
- JSON schema for cards documented

**Tracks waiting on ALPHA:**
- Track D (4-001) needs card struct for JSON schema
- Track E (6-002) needs card data for prompt building

**Estimated issues complete:** 3 (1-001, 1-002, 1-003)

---

### BETA: Gamestate Serialization

**Required before passing:**
- Full gamestate can be exported to JSON
- All game mechanics working (effects, combat, bases)
- Round-trip serialization tested

**Tracks waiting on BETA:**
- Track B (2-005) needs gamestate format for protocol
- Track C can begin protocol integration

**Estimated issues complete:** 12 (Phase 1 core: 1-001 through 1-012)

---

### GAMMA: Protocol Finalized

**Required before passing:**
- All message types defined (server→client, client→server)
- Validation rules implemented
- Hidden information handling specified

**Tracks waiting on GAMMA:**
- Track C (3-007) needs protocol for draw order
- Track B (2-007+) needs protocol for session management

**Estimated issues complete:** 20 (Phase 1 + Phase 2 protocol)

---

### DELTA: Playable Game

**Required before passing:**
- Complete game loop working over network
- Both terminal and browser clients functional
- Two players can complete a full game

**Tracks waiting on DELTA:**
- Track D (4-009, 4-010) can do final balance with real gameplay
- Track E (5-006, 5-009+) can integrate with live game events

**Estimated issues complete:** 45 (Phase 1 + 2 + 3)

---

### EPSILON: Content Complete

**Required before passing:**
- All 4 factions implemented and balanced
- Starting decks and neutral cards complete
- Balance validator passes all checks

**Tracks waiting on EPSILON:**
- Track E can generate faction-specific narratives
- Final integration testing begins

**Estimated issues complete:** 55 (Phase 1-4)

---

### OMEGA: Complete Experience

**Required before passing:**
- AI narrative working for all game events
- Dynamic card art regenerating properly
- Battle canvas with inpainting functional
- All phase demos passing

**Final deliverable:** Complete Symbeline Realms game

**Estimated issues complete:** 88 (All phases)

---

## Recommended Team Assignments

### Solo Developer
Work tracks sequentially: A → B → C → D → E
Use checkpoints to validate each phase before continuing.

### Two Developers
- **Dev 1:** Track A + Track B (engine + networking)
- **Dev 2:** Track C + Track D (clients + content)
- **Sync at:** BETA, GAMMA, DELTA

### Three Developers
- **Dev 1:** Track A (core engine authority)
- **Dev 2:** Track B + Track C (server + clients)
- **Dev 3:** Track D + Track E (content + AI)
- **Sync at:** ALPHA, BETA, GAMMA, DELTA

### Four Developers
- **Dev 1:** Track A (game logic)
- **Dev 2:** Track B (server infrastructure)
- **Dev 3:** Track C (client rendering)
- **Dev 4:** Track D + Track E (content + AI)
- **Sync at:** All checkpoints

### Five Developers (Maximum Parallelism)
- **Dev 1:** Track A
- **Dev 2:** Track B
- **Dev 3:** Track C
- **Dev 4:** Track D
- **Dev 5:** Track E
- **Sync at:** All checkpoints

---

## Cross-Track Dependencies Summary

```
Track A (Game Logic)
    │
    ├──► 1-001 (Card struct) ──────► Track D (4-001 schema)
    │                               Track E (6-002 prompt builder)
    │
    ├──► 1-012 (Serialization) ───► Track B (2-005 protocol)
    │
    └──► All mechanics ───────────► Track E (5-006 trade row logic)

Track B (Server)
    │
    ├──► 2-001 (Config) ──────────► Track E (5-001, 6-001 API clients)
    │
    └──► 2-005 (Protocol) ────────► Track C (3-007 draw order)

Track C (Clients)
    │
    └──► 3-006 (Style prefs) ─────► Track E (6-003 art regen)

Track D (Content)
    │
    └──► 4-001 (Schema) ──────────► Track E (6-002 prompt builder)
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

When working on these issues, complete all sub-issues before marking the
parent issue complete.

---

## Progress Tracking

Each track maintains its own progress file:
- `issues/track-a-progress.md` - Game Logic
- `issues/track-b-progress.md` - Server Infrastructure
- `issues/track-c-progress.md` - Client Rendering
- `issues/track-d-progress.md` - Game Content
- `issues/track-e-progress.md` - AI Integration

Checkpoints are tracked in:
- `issues/checkpoint-status.md`

---

## Starting Points

Each track can begin immediately with these issues:

| Track   | First Issue | Description                          |
|---------|-------------|--------------------------------------|
| A       | 1-001       | Card data structure                  |
| B       | 2-001       | Configuration system                 |
| C       | 3-003       | Wasm build configuration             |
| D       | 4-001       | Card JSON schema (after ALPHA)       |
| E       | 5-001       | LLM API client (after B:2-001)       |

**Critical Path:** Track A → ALPHA → Track D + rest of Track A → BETA → Track B protocol
