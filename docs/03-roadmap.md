# Symbeline Realms - Development Roadmap

This roadmap describes a C-based implementation with network-first architecture.
See `04-architecture-c-server.md` for detailed technical design.

<!-- ================================ phase 1 issue files ================================ -->

## Phase 1: Core C Engine

Foundation layer implementing game logic in C with JSON card parsing.

### 1-001: Project Structure and Build System
Set up C project with Makefile, directory structure, and dependency management.

### 1-002: Card Data Structure
Define C structs for cards including cost, effects, faction, upgrades, and instance tracking.

### 1-003: JSON Card Parser
Parse card definitions from JSON files using cJSON library.

### 1-004: Deck Management System
Implement deck, hand, discard pile, and shuffle logic with card instances.

### 1-005: Player State Management
Track authority, deck flow (d10/d4), per-turn resources, and draw order preferences.

### 1-006: Trade Row Implementation
Create the 5-card trade row with buy/replace mechanics and DM hook points.

### 1-007: Turn Loop Structure
Implement draw phase (with order choice) -> main -> discard phase flow.

### 1-008: Basic Combat Resolution
Handle attack values, authority damage, and base targeting.

### 1-009: Card Effect System
Dispatch table for effect execution with upgrade support.

### 1-010: Card Upgrade System
Permanent modifications to card instances (blacksmith pattern).

### 1-011: Base and Spawning Mechanics
Bases with delayed activation and unit spawning.

### 1-012: Gamestate Serialization
Convert game state to JSON for protocol transmission.

### 1-013: Phase 1 Demo
Command-line game loop for testing (stdin/stdout, no network).

---

<!-- ================================ phase 2 issue files ================================ -->

## Phase 2: Network Layer

Server infrastructure for multi-client connections.

### 2-001: Configuration System
JSON config file parsing for server settings and external service endpoints.

### 2-002: HTTP Server
Static file serving for Wasm client distribution (libwebsockets).

### 2-003: WebSocket Handler
Bidirectional gamestate sync over WebSocket connections.

### 2-004: SSH Server Integration
Embed libssh for terminal client connections (compile from source).

### 2-005: Protocol Implementation
Message types, validation, and dispatch for client-server communication.

### 2-006: Connection Manager
Track connected clients, handle join/leave, broadcast updates.

### 2-007: Game Session Management
Create/join games, player assignment, spectator support.

### 2-008: Hidden Information Handling
Ensure server only reveals appropriate data to each client.

### 2-009: Input Validation
Server-side validation of all client actions.

### 2-010: Phase 2 Demo
Two players connecting (one SSH, one placeholder WebSocket) playing a game.

---

<!-- ================================ phase 3 issue files ================================ -->

## Phase 3: Client Renderers

Terminal and browser client implementations.

### 3-001: Terminal Renderer (ncurses)
Full TUI for SSH clients with game display and input handling.

### 3-002: Terminal Input System
Command parsing and action dispatch for terminal clients.

### 3-003: Wasm Build Configuration
Emscripten setup for compiling client to WebAssembly.

### 3-004: Browser Canvas Renderer
HTML5 Canvas rendering of gamestate with card layouts.

### 3-005: Browser Input Handler
Click/touch handling and action submission via WebSocket.

### 3-006: Client Style Preferences
localStorage system for user rendering preferences and style guides.

### 3-007: Draw Order Interface
UI for players to choose card draw sequence.

### 3-008: Animation System
Smooth transitions for card plays, attacks, and state changes.

### 3-009: Narrative Display
Scrollable story text panel alongside game interface.

### 3-010: Phase 3 Demo
Full game playable in both terminal and browser simultaneously.

---

<!-- ================================ phase 4 issue files ================================ -->

## Phase 4: Card Content and Balance

Creating the actual game content with fantasy-themed cards.

### 4-001: Card JSON Schema
Define and document the JSON format for card definitions.

### 4-002: Faction Design - Merchant Guilds
Trade-focused faction with commerce and wealth mechanics.

### 4-003: Faction Design - The Wilds
Aggressive faction with swarming and beast spawning.

### 4-004: Faction Design - High Kingdom
Defensive faction with authority gain and disruption.

### 4-005: Faction Design - Artificer Order
Scrap-focused faction with upgrades and deck manipulation.

### 4-006: Starting Deck Definition
Basic scout and viper equivalents for each player.

### 4-007: Neutral Trade Deck Cards
Faction-neutral cards for the trade row.

### 4-008: Upgrade Effect Cards
Cards that apply permanent upgrades (blacksmith, enchanter, etc).

### 4-009: Card Balance Validator
C utility to analyze card power distribution and flag outliers.

### 4-010: Phase 4 Demo
Complete game with all factions playable and balanced.

---

<!-- ================================ phase 5 issue files ================================ -->

## Phase 5: LLM Dungeon Master

AI narrative system with prompt networking.

### 5-001: LLM API Client
HTTP client for LLM endpoint communication (configurable backend).

### 5-002: Prompt Network Structure
Define the graph of interconnected prompts (world, forces, events, narration).

### 5-003: World State Prompt
Persistent context describing the realm, factions, and current state.

### 5-004: Force Description Prompts
Per-player prompts describing their warband composition and style.

### 5-005: Event Narration Prompts
Turn-by-turn narration of card plays, attacks, and purchases.

### 5-006: Trade Row Selection Logic
DM-influenced card selection with singleton encouragement.

### 5-007: Context Window Management
Rolling history with key moment preservation within token limits.

### 5-008: Narrative Caching
Cache LLM responses for replay and reduced API calls.

### 5-009: Coherence Recovery
Handle narrative conflicts when game state diverges from story.

### 5-010: Phase 5 Demo
Full game with AI-generated narrative for all events.

---

<!-- ================================ phase 6 issue files ================================ -->

## Phase 6: Visual Generation System

Dynamic card art and battle scene generation.

### 6-001: ComfyUI API Client
HTTP client for ComfyUI workflows and image retrieval.

### 6-002: Card Image Prompt Builder
Generate prompts from card data, upgrades, and style preferences.

### 6-003: Dynamic Art Regeneration
Trigger image regeneration on draw, respecting shuffle boundaries.

### 6-004: Upgrade Visualization
Modify prompts to reflect card upgrades (better weapons, armor, etc).

### 6-005: Battle Canvas Manager
Track scene composition for progressive inpainting.

### 6-006: Inpainting Region Selection
Determine which canvas regions to fill based on game events.

### 6-007: Scene Composition Rules
Placement logic for forces, bases, and action elements.

### 6-008: Style Transfer Prompts
Fantasy aesthetic enforcement via prompt engineering.

### 6-009: Image Caching and Persistence
Cache generated images, export frames for replay/sharing.

### 6-010: Phase 6 Demo
Complete visual experience with dynamic card art and battle scenes.

---

<!-- ================================ phase 7 issue files ================================ -->

## Phase 7: Campaign Mode

Persistent progression and story-driven gameplay.

### 7-001: Campaign State Persistence
Save/load campaign progress with player choices and unlocks.

### 7-002: Scenario Definition Format
JSON schema for campaign scenarios with custom rules and conditions.

### 7-003: Branching Narrative System
Story branches based on player decisions and game outcomes.

### 7-004: Unlock System
Unlock cards, factions, and cosmetics through campaign progression.

### 7-005: Campaign AI Opponents
Scripted opponents with thematic decks for campaign encounters.

### 7-006: Phase 7 Demo
Playable campaign prologue with branching paths.

---

<!-- ================================ phase 8 issue files ================================ -->

## Phase 8: AI Gameplay System

Strategic AI for playing the game, with a meta-level orchestrator that dynamically
generates and selects between gameplay strategies using a fractal tree structure.

### 8-001: AI Runner Infrastructure
Core framework for AI game execution - action enumeration, evaluation, and execution.

### 8-002: Strategy Tree Data Structure
Fractal tree-like data structure for representing branching strategic options.
Nodes contain action choices, edges represent transitions, leaves are terminal states.

### 8-003: Option Generation System
Runtime generation of strategic options based on current game state.
Dynamically builds tree branches as game evolves.

### 8-004: Character Motivation Model
Define AI "personalities" through motivation vectors - aggression, economy,
defense, disruption weights that influence option evaluation.

### 8-005: Movement Interaction Analyzer
Track card interactions and movement patterns to inform strategic choices.
Identify synergies, threats, and opportunity costs.

### 8-006: Strategy Orchestrator
Meta-level selector that chooses between AI strategy designs based on:
- Current game state evaluation
- Opponent modeling
- Risk assessment
- Long-term planning horizon

### 8-007: Tree Pruning and Evaluation
Efficient traversal and pruning of strategy tree using heuristics.
Alpha-beta style cutoffs for large decision spaces.

### 8-008: Opponent Modeling System
Build probabilistic model of opponent tendencies from observed play.
Adapt strategy selection based on opponent patterns.

### 8-009: AI Difficulty Profiles
Configurable AI personalities from "chaotic beginner" to "optimal calculator".
Personality affects tree depth, evaluation weights, and deliberate suboptimality.

### 8-010: Strategy Learning System
Track which strategies succeed/fail against different situations.
Update strategy selection weights based on outcomes (optional ML integration).

### 8-011: Performance Optimization
Caching, parallelization, and incremental tree updates for real-time play.
Budget-based computation limiting for consistent response times.

### 8-012: Phase 8 Demo
AI vs AI exhibition match with strategy visualization and commentary.
Human vs AI with adjustable difficulty.

### 8-013: Faction Synergy Heuristics
Bias card acquisition toward faction coherence - cards sharing a faction with
existing ally-effect cards are valued higher. Creates emergent deck-building
strategies where the AI naturally gravitates toward faction concentration.

---

## Future Phases (Outline)

### Phase 9: Card Editor and Modding
- In-game card creation tools
- Custom style guide authoring
- Balance testing sandbox
- Custom AI personality creation

### Phase 10: Tournament Features
- Matchmaking
- Ranked play with AI rating calibration
- Replay system with AI analysis
- Tournament brackets
