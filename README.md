# Symbeline Realms

A fantasy-themed deck-building card game inspired by Star Realms, featuring an AI-powered Dungeon Master that narrates battles and dynamically generates card artwork.

## Overview

Symbeline Realms transforms the sci-fi deck-builder formula into a fantasy setting where an LLM-powered DM tells the story of your battles. As you play cards and attack opponents, the AI generates narrative text describing the clash of forces. Card artwork regenerates dynamically throughout the game, with upgrades reflected visually.

```
 .----------------.
 | >- 0  = .  @ * |
 |  *  \  / \  *  |
 | vv    o=.    jck
 | *-H    \\ < <  |
 |  /   *  ))     |
 '----------------'
```

## Features

- **AI Dungeon Master**: LLM-generated narrative for all game events
- **Dynamic Card Art**: Cards regenerate artwork when drawn and upgraded
- **Dual Client Support**: Play via terminal (SSH) or browser (WebSocket)
- **Modified Deck-Builder Mechanics**:
  - Draw order choice at turn start
  - Deck flow tracker (d10/d4) affecting hand size
  - Auto-draw effects that resolve before your turn
  - Base spawning mechanics

## Technology Stack

- **Language**: C11 (core engine)
- **Networking**: libwebsockets (HTTP/WebSocket), libssh (terminal)
- **JSON**: cJSON library for card data and protocol
- **LLM**: Configurable API client for narrative generation
- **Image Generation**: ComfyUI integration for dynamic card art

## Building

### Prerequisites

- GCC or Clang with C11 support
- ncurses (for terminal client)
- libwebsockets (optional, for WebSocket server)
- libssh (optional, for SSH server)

### Quick Start

```bash
# Build core engine and terminal client
make

# Run tests
make test

# Run Phase 1 demo (local play)
./run-phase1-demo.sh
```

### Build Targets

```bash
make              # Build all targets
make terminal     # Terminal client only
make demo         # Phase 1 demo
make test         # Run core tests
make test-core    # Core game logic tests
make test-config  # Configuration tests
make test-hidden-info  # Hidden information tests
make clean        # Remove build artifacts
```

### Dependencies

Local dependencies can be installed from source:

```bash
make deps         # Install local dependencies
make deps-info    # Show detected dependency sources
```

## Project Structure

```
symbeline-realms/
├── src/
│   ├── core/           # Game engine (cards, decks, combat)
│   ├── net/            # Networking (HTTP, WebSocket, SSH, protocol)
│   ├── client/         # Terminal and browser clients
│   ├── llm/            # LLM integration for narrative
│   ├── visual/         # ComfyUI client for image generation
│   └── demo/           # Phase demos
├── tests/              # Unit tests
├── libs/               # Third-party libraries (cJSON)
├── docs/               # Architecture and design documents
├── issues/             # Development issue tracking
├── assets/             # Card art and visual resources
└── config/             # Server configuration files
```

## Game Mechanics

### Factions

| Faction | Theme | Focus |
|---------|-------|-------|
| Merchant Guilds | Trade & Commerce | Kingdom Coin, Forecast |
| The Wilds | Beasts & Nature | Frontier, Pack Charge |
| High Kingdom | Knights & Order | Authority, Recruit |
| Artificer Order | Crafting & Upgrades | Scrap-Draw, Upgrades |

### Deck Flow Tracker

The d10/d4 system modifies your hand size over the game:
- **d10**: Starts at 5, increments on buy, decrements on scrap
- Overflow (10→0): Gain +1 permanent draw (tracked on d4)
- Underflow (0→9): Lose -1 permanent draw

### Draw Order Choice

At the start of each turn, choose the order to draw cards from your deck. This creates tactical decisions around sequencing auto-draw effects and responding to the trade row.

### Auto-Draw Effects

Cards with "draw a card" effects trigger automatically when drawn at turn start, eliminating the tedious play-draw-play loop of traditional deck builders.

## Development Status

### Phase 1: Core Engine - COMPLETE
Game logic, cards, decks, combat, effects, serialization.

### Phase 2: Network Layer - 16/18 COMPLETE
HTTP/WebSocket server, SSH server, protocol, sessions, hidden info.

### Phase 3: Client Renderers - In Progress
Terminal and browser client implementations.

### Future Phases
- Phase 4: Card Content and Balance
- Phase 5: LLM Dungeon Master
- Phase 6: Visual Generation System
- Phase 7-9: Campaign, Modding, Tournament

## Documentation

- [Architecture Overview](docs/01-architecture-overview.md)
- [Game Mechanics](docs/02-game-mechanics.md)
- [Development Roadmap](docs/03-roadmap.md)
- [C Server Architecture](docs/04-architecture-c-server.md)
- [Faction Guide](docs/06-faction-guide.md)

## Testing

```bash
# Run all core tests
make test-core

# Run specific test suites
make test-config       # Configuration parsing
make test-serialize    # Game state serialization
make test-protocol     # Network protocol
make test-connections  # Connection manager
make test-sessions     # Game session management
make test-hidden-info  # Hidden information handling
```

## License

This project is in active development.

## Acknowledgments

Inspired by Star Realms by White Wizard Games.
