# Symbeline Realms - Documentation Table of Contents

## Overview Documents

```
docs/
├── 01-architecture-overview.md    Core system design and components
├── 02-game-mechanics.md           Rules and modified mechanics
├── 03-roadmap.md                  Development phases and milestones
├── 04-architecture-c-server.md    C server architecture details
├── 05-parallelized-roadmap.md     5-track parallel development guide
├── 06-faction-guide.md            Faction design philosophy and strategy
├── wasm-transition-plan.md        JS/CSS to WebAssembly migration guide
└── table-of-contents.md           This document
```

## Project Notes

```
notes/
└── vision                         Original project concept
```

## Issue Tracking

### Phase Progress Files
```
issues/
├── phase-1-progress.md            Core C Engine status
├── phase-2-progress.md            Networking Layer status
├── phase-3-progress.md            Client Renderers status
├── phase-4-progress.md            Card Content status
├── phase-5-progress.md            LLM DM Integration status
└── phase-6-progress.md            Visual Generation status
```

### Phase 1: Core C Engine (22 issues)
```
issues/
├── 1-001-card-data-structure.md
├── 1-002-deck-management-system.md
├── 1-003-player-state-management.md
├── 1-004-trade-row-implementation.md
├── 1-005-turn-loop-structure.md
├── 1-006-basic-combat-resolution.md
├── 1-007-card-effect-parser.md
│   ├── 1-007a-effect-dispatch-infrastructure.md
│   ├── 1-007b-resource-effects.md
│   ├── 1-007c-card-manipulation-effects.md
│   ├── 1-007d-special-effects.md
│   ├── 1-007e-upgrade-spawn-effects.md
│   └── 1-007f-conditional-ally-abilities.md
├── 1-008-auto-draw-resolution-system.md
│   ├── 1-008a-eligibility-detection.md
│   ├── 1-008b-chain-resolution.md
│   ├── 1-008c-spent-flag-management.md
│   └── 1-008d-event-emission.md
├── 1-009-deck-flow-tracker.md
├── 1-010-base-card-type.md
├── 1-011-spawning-mechanics.md
├── 1-012-gamestate-serialization.md
└── 1-013-phase-1-demo.md
```

### Phase 2: Networking Layer (18 issues)
```
issues/
├── 2-001-configuration-system.md
├── 2-002-http-server.md
├── 2-003-websocket-handler.md
├── 2-004-ssh-server-integration.md
│   ├── 2-004a-libssh-build-integration.md
│   ├── 2-004b-ssh-authentication.md
│   ├── 2-004c-pty-terminal-handling.md
│   └── 2-004d-session-lifecycle.md
├── 2-005-protocol-implementation.md
│   ├── 2-005a-message-type-definitions.md
│   ├── 2-005b-client-to-server-handlers.md
│   ├── 2-005c-server-to-client-generation.md
│   └── 2-005d-validation-error-handling.md
├── 2-006-connection-manager.md
├── 2-007-game-session-management.md
├── 2-008-hidden-information-handling.md
├── 2-009-input-validation.md
└── 2-010-phase-2-demo.md
```

### Phase 3: Client Renderers (14 issues)
```
issues/
├── 3-001-terminal-renderer.md
├── 3-002-terminal-input-system.md
├── 3-003-wasm-build-configuration.md
├── 3-004-browser-canvas-renderer.md
│   ├── 3-004a-canvas-infrastructure.md
│   ├── 3-004b-card-rendering.md
│   ├── 3-004c-game-zones.md
│   └── 3-004d-status-narrative-panels.md
├── 3-005-browser-input-handler.md
├── 3-006-client-style-preferences.md
├── 3-007-draw-order-interface.md
├── 3-008-animation-system.md
├── 3-009-narrative-display.md
└── 3-010-phase-3-demo.md
```

### Phase 4: Card Content (10 issues)
```
issues/
├── 4-001-card-json-schema.md
├── 4-002-faction-design-merchant-guilds.md
├── 4-003-faction-design-the-wilds.md
├── 4-004-faction-design-high-kingdom.md
├── 4-005-faction-design-artificer-order.md
├── 4-006-starting-deck-definition.md
├── 4-007-neutral-trade-deck-cards.md
├── 4-008-upgrade-effect-cards.md
├── 4-009-card-balance-validator.md
└── 4-010-phase-4-demo.md
```

### Phase 5: LLM Dungeon Master (10 issues)
```
issues/
├── 5-001-llm-api-client.md
├── 5-002-prompt-network-structure.md
├── 5-003-world-state-prompt.md
├── 5-004-force-description-prompts.md
├── 5-005-event-narration-prompts.md
├── 5-006-trade-row-selection-logic.md
├── 5-007-context-window-management.md
├── 5-008-narrative-caching.md
├── 5-009-coherence-recovery.md
└── 5-010-phase-5-demo.md
```

### Phase 6: Visual Generation (14 issues)
```
issues/
├── 6-001-comfyui-api-client.md
├── 6-002-card-image-prompt-builder.md
├── 6-003-dynamic-art-regeneration.md
│   ├── 6-003a-regeneration-tracking.md
│   ├── 6-003b-style-guide-integration.md
│   ├── 6-003c-generation-queue.md
│   └── 6-003d-cache-invalidation.md
├── 6-004-upgrade-visualization.md
├── 6-005-battle-canvas-manager.md
├── 6-006-inpainting-region-selection.md
├── 6-007-scene-composition-rules.md
├── 6-008-style-transfer-prompts.md
├── 6-009-image-caching-persistence.md
└── 6-010-phase-6-demo.md
```

### Completed Issues
```
issues/completed/
└── demos/
    ├── run-phase1-demo.sh         (created in 1-013)
    ├── run-phase2-demo.sh         (created in 2-010)
    ├── run-phase3-demo.sh         (created in 3-010)
    ├── run-phase4-demo.sh         (created in 4-010)
    ├── run-phase5-demo.sh         (created in 5-010)
    └── run-phase6-demo.sh         (created in 6-010)
```

## Source Code (planned structure)

```
src/
├── core/
│   ├── 01-card.c                  Card data structure
│   ├── 02-deck.c                  Deck management
│   ├── 03-player.c                Player state
│   ├── 04-trade-row.c             Trade row mechanics
│   ├── 05-turn.c                  Turn loop
│   ├── 06-combat.c                Combat resolution
│   ├── 07-effects.c               Effect parser
│   ├── 08-auto-draw.c             Auto-draw system
│   ├── 09-flow-tracker.c          d10/d4 deck flow tracker
│   ├── 10-base.c                  Base card type
│   ├── 11-spawning.c              Spawning mechanics
│   └── 12-gamestate.c             Gamestate serialization
├── server/
│   ├── 01-config.c                Configuration system
│   ├── 02-http.c                  HTTP server (libwebsockets)
│   ├── 03-websocket.c             WebSocket handler
│   ├── 04-ssh.c                   SSH server (libssh)
│   ├── 05-protocol.c              Protocol implementation
│   ├── 06-connection.c            Connection manager
│   ├── 07-session.c               Game session management
│   ├── 08-hidden-info.c           Hidden information handling
│   └── 09-validation.c            Input validation
├── client/
│   ├── 01-terminal.c              ncurses terminal renderer
│   ├── 02-input.c                 Terminal input system
│   └── 03-preferences.c           Client preferences (native)
├── llm/
│   ├── api-client.c               LLM API integration
│   ├── prompts.c                  Prompt network structure
│   ├── context-manager.c          Context window management
│   ├── cache.c                    Narrative caching
│   └── coherence.c                Coherence recovery
└── main.c                         Entry point with mode dispatch
```

## Assets (planned structure)

```
assets/
├── cards/
│   ├── schema.json                Card schema definition
│   ├── starting/                  Starting deck cards
│   ├── merchant/                  Merchant Guilds faction
│   ├── wilds/                     The Wilds faction
│   ├── kingdom/                   High Kingdom faction
│   ├── artificer/                 Artificer Order faction
│   └── neutral/                   Neutral trade cards
├── web/
│   ├── index.html                 Browser client HTML
│   ├── style.css                  Browser styles
│   ├── canvas.js                  Canvas renderer
│   ├── input.js                   Browser input handler
│   ├── preferences.js             Client preferences UI
│   ├── draw-order.js              Draw order interface
│   ├── animation.js               Animation system
│   └── narrative.js               Narrative display
└── art/
    ├── placeholders/              Development placeholder art
    └── references/                Style reference images
```

## Libraries

```
libs/
├── cJSON/                         JSON parsing
├── libwebsockets/                 HTTP/WebSocket server
└── libssh/                        SSH server (compiled from source)
```

## Total Issues: 88
- Phase 1: 22 issues (Core C Engine, including 10 sub-issues)
- Phase 2: 18 issues (Networking, including 8 sub-issues)
- Phase 3: 14 issues (Clients, including 4 sub-issues)
- Phase 4: 10 issues (Content)
- Phase 5: 10 issues (LLM DM)
- Phase 6: 14 issues (Visuals, including 4 sub-issues)
