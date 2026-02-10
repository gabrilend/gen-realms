# Symbeline Realms - C Server Architecture

This document supersedes portions of `01-architecture-overview.md` regarding implementation technology. The game logic, networking, and rendering are implemented in C, with WebAssembly compilation for browser clients.

## Design Principles

1. **One binary, multiple modes** - server, client, and local play from same executable
2. **Protocol as API** - gamestate exchanged over well-defined protocol, enabling custom clients
3. **External AI services** - LLM and image generation are separate services, configured per-server
4. **Hidden information preserved** - server doesn't reveal hands until cards are played
5. **Client-side rendering** - each client renders gamestate according to its capabilities

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           Host Machine                                  │
│  ┌─────────────────────────────────────────────────────────────────┐    │
│  │                    Symbeline Server Binary                      │    │
│  │  ┌─────────────┐  ┌──────────────┐  ┌────────────────────────┐  │    │
│  │  │ Game Engine │  │  SSH Server  │  │  HTTP + WebSocket      │  │    │
│  │  │ (C core)    │  │  (libssh)    │  │  Server                │  │    │
│  │  │             │  │  port: cfg   │  │  port: cfg             │  │    │
│  │  └──────┬──────┘  └──────┬───────┘  └───────────┬────────────┘  │    │
│  │         │                │                      │               │    │
│  │         └────────────────┴──────────┬───────────┘               │    │
│  │                                     │                           │    │
│  │                          Gamestate Protocol                     │    │
│  └─────────────────────────────────────┼───────────────────────────┘    │
│                                        │                                │
│  ┌─────────────────┐  ┌────────────────┼────────────────┐               │
│  │ LLM Service     │  │                │                │               │
│  │ (llama.cpp,     │◄─┤   Configured   │                │               │
│  │  ollama, etc)   │  │   Endpoints    │                │               │
│  └─────────────────┘  │                │                │               │
│                       │                ▼                │               │
│  ┌─────────────────┐  │     ┌──────────────────┐        │               │
│  │ ComfyUI         │◄─┘     │ Config:          │        │               │
│  │ (SD frontend)   │        │  llm: ip:port    │        │               │
│  │                 │        │  sd:  ip:port    │        │               │
│  └─────────────────┘        └──────────────────┘        │               │
└─────────────────────────────────────────────────────────┼───────────────┘
                                                          │
                    ┌─────────────────────────────────────┼─────────────┐
                    │                                     │             │
            ┌───────▼───────┐                     ┌───────▼───────┐     │
            │ Terminal      │                     │ Browser       │     │
            │ Client        │                     │ Client        │     │
            │ (SSH)         │                     │ (Wasm)        │     │
            │               │                     │               │     │
            │ ┌───────────┐ │                     │ ┌───────────┐ │     │
            │ │ ncurses   │ │                     │ │ Canvas    │ │     │
            │ │ renderer  │ │                     │ │ renderer  │ │     │
            │ └───────────┘ │                     │ └─────┬─────┘ │     │
            │               │                     │       │       │     │
            │ No images     │                     │       ▼       │     │
            └───────────────┘                     │ ┌───────────┐ │     │
                                                  │ │ SD calls  │─┼─────┘
                                                  │ │ (direct)  │ │
                                                  │ └───────────┘ │
                                                  │               │
                                                  │ Style prefs   │
                                                  │ (localStorage)│
                                                  └───────────────┘
```

## Binary Modes

### Server Mode
```bash
./symbeline --server --config server.json
```
- Hosts game session
- Accepts SSH connections (terminal clients)
- Serves HTTP (Wasm client files) and upgrades to WebSocket
- Makes LLM calls for shared narrative
- Broadcasts gamestate updates to all clients

### Terminal Client Mode
```bash
./symbeline --connect 192.168.1.5:8080
```
- Wraps SSH connection to server
- Receives gamestate, renders with ncurses
- Sends input commands
- No image generation (text descriptions only)

### Local Mode
```bash
./symbeline --local
```
- Runs server and client in single process
- For solo play and development testing
- Can optionally connect to external LLM/SD services

### Browser Client
```
http://192.168.1.5:8080
```
- Server serves compiled Wasm + HTML/JS
- WebSocket connection for gamestate
- Canvas rendering with animations
- Direct ComfyUI calls for image generation
- Style preferences in browser localStorage

## Configuration

### Server Configuration (server.json)
```json
{
  "game_port": 8080,
  "ssh_port": 8022,
  "llm_endpoint": "192.168.1.10:5000",
  "llm_model": "mistral-7b",
  "comfyui_endpoint": "192.168.1.10:8188",
  "max_players": 4,
  "game_rules": {
    "starting_authority": 50,
    "starting_hand_size": 5,
    "d10_start": 5
  }
}
```

### Client Style Preferences (browser localStorage)
```json
{
  "style_guide": "dark fantasy, oil painting, dramatic lighting",
  "negative_prompts": "cartoon, anime, bright colors",
  "card_frame_style": "ornate gold",
  "animation_speed": 1.0
}
```

## Gamestate Protocol

The protocol is the core abstraction enabling multiple client types.

### Server → Client Messages
```json
{
  "type": "gamestate",
  "turn": 12,
  "phase": "main",
  "active_player": 1,
  "you": {
    "id": 1,
    "authority": 35,
    "d10": 7,
    "d4": 1,
    "hand": [
      {"id": "dire_bear", "instance_id": "abc123", "image_seed": 42}
    ],
    "trade": 3,
    "combat": 5,
    "bases": [...],
    "deck_count": 12,
    "discard_count": 5
  },
  "opponent": {
    "id": 2,
    "authority": 28,
    "hand_count": 4,
    "bases": [...],
    "deck_count": 8,
    "discard_count": 7
  },
  "trade_row": [...],
  "narrative": "The dire bear emerges from the Thornwood..."
}
```

### Client → Server Messages
```json
{
  "type": "action",
  "action": "play_card",
  "card_instance_id": "abc123"
}

{
  "type": "action",
  "action": "buy_card",
  "slot": 2
}

{
  "type": "action",
  "action": "attack",
  "target": "player"
}

{
  "type": "action",
  "action": "set_draw_order",
  "order": ["def456", "ghi789", "abc123"]
}
```

## Dynamic Card Art System

Cards have dynamic artwork that regenerates throughout the game.

### Image Lifecycle
1. **Card drawn** → Client requests new image from ComfyUI
2. **Card in hand** → Current image displayed
3. **Card played** → Image persists for this instance
4. **Card discarded** → Old image cached, marked for regeneration
5. **Deck shuffled** → Regeneration queue processed
6. **Card upgraded** → New image generated reflecting upgrade

### Image Request (Client → ComfyUI)
```json
{
  "prompt": "A dire bear with iron-clad armor, dark fantasy style",
  "negative_prompt": "sci-fi, modern, cartoon",
  "seed": 42,
  "card_id": "dire_bear",
  "instance_id": "abc123",
  "upgrades": ["+1_attack"],
  "style_guide": "user preference string..."
}
```

### Draw Order Choice
Players choose the order to draw cards from their deck:
1. Server sends list of card backs (count only, no identity)
2. Player selects order: "draw 3rd, then 1st, then 5th..."
3. Server reveals cards in chosen order
4. Each card triggers image generation in sequence
5. Strategic value: see trade row state before committing to draws

## Card Upgrade System

Cards can be permanently modified by effects.

### Upgrade Data Structure
```c
typedef struct {
    char* card_id;           // base card type
    char* instance_id;       // unique instance
    int attack_bonus;        // permanent +attack
    int trade_bonus;         // permanent +trade
    int authority_bonus;     // permanent +authority
    char** applied_upgrades; // list of upgrade names
    int image_seed;          // for regeneration
    bool needs_regen;        // image outdated
} CardInstance;
```

### Upgrade Flow (Blacksmith Example)
1. Turn N: Play "Blacksmith" (temporary card, scraps on play, draws a card)
2. Blacksmith effect: Choose a card in discard, apply "+1 attack"
3. Turn N: Upgraded card still in discard (effect pending)
4. Turn N+X: Deck shuffles, upgraded card enters draw pile
5. Turn N+Y: Upgraded card drawn, new image generated showing better weapons
6. Total delay: ~2 shuffle cycles for visual effect

## LLM Narrative System

### Prompt Network
The DM uses interconnected prompts that build on each other:

```
┌─────────────────┐
│ World State     │ (persistent context: factions, terrain, time of day)
└────────┬────────┘
         │
         ▼
┌─────────────────┐     ┌─────────────────┐
│ Player 1 Forces │     │ Player 2 Forces │
│ (warband desc)  │     │ (warband desc)  │
└────────┬────────┘     └────────┬────────┘
         │                       │
         └───────────┬───────────┘
                     │
                     ▼
         ┌───────────────────┐
         │ Event Description │ (card played, attack, etc)
         └─────────┬─────────┘
                   │
                   ▼
         ┌───────────────────┐
         │ Scene Narration   │ (final output to players)
         └───────────────────┘
```

### Prompt Chaining
Each game event triggers a chain:
1. Update world state prompt with new info
2. Regenerate affected force descriptions
3. Generate event-specific narration
4. Compose final scene description
5. Cache intermediate results for continuity

### Coherence Management
- Key story beats are preserved in context
- Conflicting events are reconciled narratively
- "Sometimes things don't line up" is acceptable flavor

## Build System

### Dependencies
- **libssh** - SSH server (compiled from source)
- **cJSON** - JSON parsing
- **libwebsockets** - WebSocket server
- **Emscripten** - Wasm compilation
- **ncurses** - Terminal rendering (native only)

### Build Targets
```makefile
symbeline-server    # Native server binary (Linux)
symbeline-client    # Native terminal client
symbeline.wasm      # Browser client
symbeline.js        # Wasm glue code
```

### Installation
```bash
./configure          # Check dependencies, download libssh source
make deps            # Compile libssh from source
make                 # Build all targets
make install         # Install to /usr/local/bin
```

## File Structure

```
symbeline-realms/
├── src/
│   ├── core/           # Game logic (C)
│   │   ├── 01-card.c
│   │   ├── 02-deck.c
│   │   ├── 03-player.c
│   │   ├── 04-trade-row.c
│   │   ├── 05-turn.c
│   │   ├── 06-combat.c
│   │   ├── 07-effects.c
│   │   └── 08-upgrades.c
│   ├── net/            # Networking
│   │   ├── ssh-server.c
│   │   ├── ws-server.c
│   │   ├── http-server.c
│   │   └── protocol.c
│   ├── client/         # Client-side
│   │   ├── terminal.c
│   │   └── wasm/
│   │       ├── canvas.c
│   │       ├── comfyui.c
│   │       └── storage.c
│   ├── llm/            # LLM integration
│   │   ├── api-client.c
│   │   ├── prompt-network.c
│   │   └── context.c
│   └── main.c          # Entry point, mode switching
├── assets/
│   ├── cards/          # JSON card definitions
│   └── web/            # Static files for browser
├── config/
│   └── server.json.example
└── docs/
```

## Security Considerations

- SSH provides encrypted terminal connections
- WebSocket should use WSS (TLS) in production
- API keys for LLM/SD services stay server-side
- Client preferences are local only (no server storage)
- Game protocol validates all actions server-side
